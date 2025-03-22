#include "period_timer.h"
#include "audio_mixer.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h>

#define SAMPLE_RATE 44100			// Samples per second
#define NUM_CHANNELS 1				// Mono sound only has 1 channel
#define SAMPLE_SIZE (sizeof(short)) // Bytes per sample, works for mono files because each sample is 1 value
#define MAX_SOUND_BITES 30			// Maximum number of sound bites that can be played simultaneously
#define DEFAULT_VOLUME 80			// Default volume level

#define CLIP_MAX 32767 // Clip the maximum value of the sound bite
#define CLIP_MIN 32768 // Clip the minimum value of the sound bite

// The currently active sound bites being played
static snd_pcm_t *handle;

// Size of the hardware playback buffer
static unsigned long playbackBufferSize = 0;

// Playback buffer
static short *playbackBuffer = NULL;

// Structure to hold a sound bite
typedef struct
{
	wavedata_t *pSound; // A pointer to the previously allocated sound bite
	int location;		// The offset into the pData of pSound
} playbackSound_t;

// Array of sound bites that are currently being played
static playbackSound_t soundBites[MAX_SOUND_BITES];

// Thread control and mutex variables
static pthread_t playbackThreadId;
static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;
static _Bool stopping = false;

// Volume level
static int volume = 0;

// Helper functions to play the sound bites
static void *playbackThread(void *arg);
static void fillPlaybackBuffer(short *buff, int size);

void audio_mixer_init(void)
{
	audio_mixer_set_volume(DEFAULT_VOLUME);

	for (int i = 0; i < MAX_SOUND_BITES; i++){
		soundBites[i].pSound = NULL;
		soundBites[i].location = 0;
	}

	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0){
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	err = snd_pcm_set_params(handle,
							 SND_PCM_FORMAT_S16_LE,
							 SND_PCM_ACCESS_RW_INTERLEAVED,
							 NUM_CHANNELS,
							 SAMPLE_RATE,
							 1,
							 50000);
	if (err < 0){
		printf("Play-back configuration error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}
	unsigned long unusedBufferSize = 0;
	snd_pcm_get_params(handle, &unusedBufferSize, &playbackBufferSize);
	playbackBuffer = malloc(playbackBufferSize * sizeof(*playbackBuffer));
	pthread_create(&playbackThreadId, NULL, playbackThread, NULL);
}

void audio_mixer_read_file(char *fileName, wavedata_t *pSound)
{
	assert(pSound);
	const int PCM_DATA_OFFSET = 44;

	FILE *file = fopen(fileName, "r");
	if (file == NULL){
		fprintf(stderr, "ERROR: Unable to open file %s.\n", fileName);
		exit(EXIT_FAILURE);
	}

	fseek(file, 0, SEEK_END);
	int sizeInBytes = ftell(file) - PCM_DATA_OFFSET;
	pSound->numSamples = sizeInBytes / SAMPLE_SIZE;

	fseek(file, PCM_DATA_OFFSET, SEEK_SET);

	pSound->pData = malloc(sizeInBytes);
	if (pSound->pData == 0){
		fprintf(stderr, "ERROR: Unable to allocate %d bytes for file %s.\n",
				sizeInBytes, fileName);
		exit(EXIT_FAILURE);
	}

	int samplesRead = fread(pSound->pData, SAMPLE_SIZE, pSound->numSamples, file);
	if (samplesRead != pSound->numSamples){
		fprintf(stderr, "ERROR: Unable to read %d samples from file %s (read %d).\n",
				pSound->numSamples, fileName, samplesRead);
		exit(EXIT_FAILURE);
	}
}

void audio_mixer_queue_sound(wavedata_t *pSound)
{
	assert(pSound->numSamples > 0);
	assert(pSound->pData);
	bool found = false;

	pthread_mutex_lock(&audioMutex);
	{
		for (int i = 0; i < MAX_SOUND_BITES; i++){
			if (soundBites[i].pSound == NULL){
				soundBites[i].pSound = pSound;
				soundBites[i].location = 0;
				found = true;
				break;
			}
		}
	}
	pthread_mutex_unlock(&audioMutex);

	if (!found){
		printf("ERROR: No free sound bite slots available to play sound.\n");
	}
}

int audio_mixer_get_volume()
{
	return volume;
}

// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void audio_mixer_set_volume(int newVolume)
{
	if (newVolume < 0 || newVolume > AUDIOMIXER_MAX_VOLUME){
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}
	volume = newVolume;

	long min, max;
	snd_mixer_t *mixerHandle;
	snd_mixer_selem_id_t *sid;
	const char *card = "default";
	const char *selem_name = "Master"; // For ZEN cape
	// const char *selem_name = "Speaker";	// For USB Audio

	snd_mixer_open(&mixerHandle, 0);

	snd_mixer_attach(mixerHandle, card);
	snd_mixer_selem_register(mixerHandle, NULL, NULL);
	snd_mixer_load(mixerHandle);

	snd_mixer_selem_id_alloca(&sid);
	snd_mixer_selem_id_set_index(sid, 0);
	snd_mixer_selem_id_set_name(sid, selem_name);
	snd_mixer_elem_t *elem = snd_mixer_find_selem(mixerHandle, sid);

	snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
	snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

	snd_mixer_close(mixerHandle);
}

void audio_mixer_free_file_data(wavedata_t *pSound)
{
	pSound->numSamples = 0;
	free(pSound->pData);
	pSound->pData = NULL;
}

void audio_mixer_cleanup(void)
{
	printf("Stopping audio...\n");
	stopping = true;
	pthread_join(playbackThreadId, NULL);
	snd_pcm_drain(handle);
	snd_pcm_close(handle);
	free(playbackBuffer);
	playbackBuffer = NULL;
	printf("Done stopping audio...\n");
	fflush(stdout);
}

static void *playbackThread(void *_arg)
{
	(void)_arg;
	while (!stopping){
		fillPlaybackBuffer(playbackBuffer, playbackBufferSize);

		snd_pcm_sframes_t frames = snd_pcm_writei(handle, playbackBuffer, playbackBufferSize);

		if (frames < 0){
			fprintf(stderr, "AudioMixer: writei() returned %li\n", frames);
			frames = snd_pcm_recover(handle, frames, 1);
		}
		if (frames < 0){
			fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n", frames);
			exit(EXIT_FAILURE);
		}
		if (frames > 0 && (size_t)frames < playbackBufferSize){
			printf("Short write (expected %lu, wrote %li)\n",
				   (unsigned long)playbackBufferSize, frames);
		}
	}
	return NULL;
}

// Fill the buff array with new PCM values to output.
//    buff: buffer to fill with new PCM data from sound bites.
//    size: the number of *values* to store into buff
static void fillPlaybackBuffer(short *buff, int size)
{
	memset(buff, 0, size * SAMPLE_SIZE);

	pthread_mutex_lock(&audioMutex);
	{
		for (int i = 0; i < MAX_SOUND_BITES; i++){

			if (soundBites[i].pSound != NULL){

				int remainingSamples = soundBites[i].pSound->numSamples - soundBites[i].location;
				int samplesToMix;
				if (remainingSamples < size){
					samplesToMix = remainingSamples;
				}
				else{
					samplesToMix = size;
				}

				for (int j = 0; j < samplesToMix; j++){
					int mixedSample = buff[j] + soundBites[i].pSound->pData[soundBites[i].location + j];
					if (mixedSample > CLIP_MAX){
						mixedSample = CLIP_MAX;
					}
					if (mixedSample < -CLIP_MIN){
						mixedSample = -CLIP_MIN;
					}
					buff[j] = (short)mixedSample;
				}

				soundBites[i].location += samplesToMix;
				if (soundBites[i].location >= soundBites[i].pSound->numSamples){
					soundBites[i].pSound = NULL;
				}
			}
		}
	}
	pthread_mutex_unlock(&audioMutex);
	period_mark_event(PERIOD_EVENT_SAMPLE_AUDIO);
}
