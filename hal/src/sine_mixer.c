/*
 * This file implements the sine mixer module, which generates and plays
 * which is a modified version of the audio mixer from assignment 3.
 */
#include "sine_mixer.h"
#include <alsa/asoundlib.h>
#include <stdbool.h>
#include <pthread.h>
#include <limits.h>
#include <alloca.h>
#include <math.h>

#define PI 3.1415926535897932		// Pi constant used in sine wave calculations
#define DEFAULT_VOLUME 80			// Default volume level
#define SAMPLE_RATE 44100			// Sample rate in Hz
#define NUM_CHANNELS 1				// Number of audio channels (mono)
#define SAMPLE_SIZE (sizeof(short)) // Bytes per sample

// Audio buffer size and playback buffer
static unsigned long playbackBufferSize = 0;
static short *playbackBuffer = NULL;
static snd_pcm_t *handle;

// vars to control current frequency playback
static enum SineMixer_waveform currentWaveform = SINEMIXER_WAVE_SINE;
static double decayingSine_timeVar = 0;

static double frequencyDistortion = 0;
static double currentFrequency = 0;
static double desiredFrequency = 0;
static double frequencyChangeRate = 0;
static bool isPlaying = false;
static double phase = 0;
static int volume = 0;

// Playback threading
static _Bool stopping = false;
static pthread_t playbackThreadId;
static pthread_mutex_t audioMutex = PTHREAD_MUTEX_INITIALIZER;
static void* playbackThread(void* arg);

// Waveform functions
static double squareWave(double phase);
static double triangleWave(double phase);
static double sawtoothWave(double phase);
static double stairWave(double phase);
static double rectifiedSineWave(double phase);
static double decayingSineWave(double phase);

void SineMixer_init(void)
{
	SineMixer_setVolume(DEFAULT_VOLUME);

	// Open the PCM output
	int err = snd_pcm_open(&handle, "default", SND_PCM_STREAM_PLAYBACK, 0);
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Configure parameters of PCM output
	err = snd_pcm_set_params(handle,
			SND_PCM_FORMAT_S16_LE,
			SND_PCM_ACCESS_RW_INTERLEAVED,
			NUM_CHANNELS,
			SAMPLE_RATE,
			1,			// Allow software resampling
			50000);		// 0.05 seconds per buffer
	if (err < 0) {
		printf("Playback open error: %s\n", snd_strerror(err));
		exit(EXIT_FAILURE);
	}

	// Allocate this software's playback buffer to be the same size as the
	// the hardware's playback buffers for efficient data transfers.
	// ..get info on the hardware buffers:
 	unsigned long unusedBufferSize = 0;
	snd_pcm_get_params(handle, &unusedBufferSize, &playbackBufferSize);
	// ..allocate playback buffer:
	playbackBuffer = malloc(playbackBufferSize * sizeof(*playbackBuffer));

	// Launch playback thread:
	pthread_create(&playbackThreadId, NULL, playbackThread, NULL);
}

void SineMixer_queueFrequency(double frequency)
{
    if(frequency == desiredFrequency) return;
    pthread_mutex_lock(&audioMutex);
	{
		desiredFrequency = frequency;
		if(currentFrequency == 0) {
			currentFrequency = desiredFrequency;
		}
		isPlaying = true;
		decayingSine_timeVar = 0;
	}
    pthread_mutex_unlock(&audioMutex);
}

double SineMixer_getFrequency(void)
{
	return desiredFrequency;
}

void SineMixer_stopPlayback(void)
{
    pthread_mutex_lock(&audioMutex);
    {
		isPlaying = false;
		currentFrequency = 0;
		desiredFrequency = 0;
	}
    pthread_mutex_unlock(&audioMutex);
}

void SineMixer_setWaveform(enum SineMixer_waveform waveform)
{
	pthread_mutex_lock(&audioMutex);
	{
		currentWaveform = waveform;
		decayingSine_timeVar = 0;
	}
	pthread_mutex_unlock(&audioMutex);
}

enum SineMixer_waveform Sinemixer_getWaveform(void)
{
	return currentWaveform;
}

void SineMixer_setDistortion(double distortion)
{
	pthread_mutex_lock(&audioMutex);
	{
		frequencyDistortion = distortion;
	}
    pthread_mutex_unlock(&audioMutex);
}

double SineMixer_getDistortion(void)
{
	return frequencyDistortion;
}

// Function copied from:
// http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
// Written by user "trenki".
void SineMixer_setVolume(int newVolume)
{
	// Ensure volume is reasonable; If so, cache it for later getVolume() calls.
	if (newVolume < SINEMIXER_VOLUME_MIN || newVolume > SINEMIXER_VOLUME_MAX) {
		printf("ERROR: Volume must be between 0 and 100.\n");
		return;
	}
	volume = newVolume;

    long min, max;
    snd_mixer_t *mixerHandle;
    snd_mixer_selem_id_t *sid;
    const char *card = "default";
    const char *selem_name = "PCM";	// For ZEN cape
    //const char *selem_name = "Speaker";	// For USB Audio

    snd_mixer_open(&mixerHandle, 0);
    snd_mixer_attach(mixerHandle, card);
    snd_mixer_selem_register(mixerHandle, NULL, NULL);
    snd_mixer_load(mixerHandle);

    snd_mixer_selem_id_alloca(&sid);
    snd_mixer_selem_id_set_index(sid, 0);
    snd_mixer_selem_id_set_name(sid, selem_name);
    snd_mixer_elem_t* elem = snd_mixer_find_selem(mixerHandle, sid);

    snd_mixer_selem_get_playback_volume_range(elem, &min, &max);
    snd_mixer_selem_set_playback_volume_all(elem, volume * max / 100);

    snd_mixer_close(mixerHandle);
}

int SineMixer_getVolume()
{
	// Return the cached volume; good enough unless someone is changing
	// the volume through other means and the cached value is out of date.
	return volume;
}

void SineMixer_cleanup(void)
{
	// Stop the PCM generation thread
	stopping = true;
	pthread_join(playbackThreadId, NULL);

	// Shutdown the PCM output, allowing any pending sound to play out (drain)
	snd_pcm_drain(handle);
	snd_pcm_close(handle);

	// Free playback buffer
	// (note that any wave files read into wavedata_t records must be freed
	// in addition to this by calling AudioMixer_freeWaveFileData() on that struct.)
	free(playbackBuffer);
	playbackBuffer = NULL;

	fflush(stdout);
}

// following functions return a value between -1 and 1 when provided a value between 0 and 2pi

static double squareWave(double phase)
{
	return (phase < PI) ? 1.0 : -1.0;
}

static double triangleWave(double phase)
{
	if(phase < PI) {
		return (2.0*phase/PI) - 1.0;
	} 
	else {
		return 1.0 - (2.0*(phase-PI)/PI);
	}
}

static double sawtoothWave(double phase)
{
	return (phase/PI) - 1.0;
}

#define STAIRWAVE_NUMSTEPS 6
static double stairWave(double phase)
{
	return floor((phase / (2*PI)) * STAIRWAVE_NUMSTEPS) / STAIRWAVE_NUMSTEPS*2.0-1.0;  
}

static double rectifiedSineWave(double phase)
{
	return fabs(sin(phase));
}

#define DECAYINGSINE_DECAYRATE 4.0
static double decayingSineWave(double phase)
{
	decayingSine_timeVar += 0.00002;
	return sin(phase) * exp(-DECAYINGSINE_DECAYRATE * decayingSine_timeVar);
}

// Fill the buff array with new PCM values to output.
//    buff: buffer to fill with new PCM data from sound bites.
//    size: the number of *values* to store into buff
static void fillPlaybackBuffer(short *buff, int size)
{
	memset(buff,0,size * sizeof(short));
	double freq;
	double distortion;
	enum SineMixer_waveform waveform;
	bool playing;
	pthread_mutex_lock(&audioMutex);
    {
		freq = currentFrequency;
		distortion = frequencyDistortion;
		waveform = currentWaveform;
		playing = isPlaying;
	}
	pthread_mutex_unlock(&audioMutex);
	distortion = freq * distortion;
	freq += (2*distortion)*(rand()/(double)RAND_MAX)-distortion;
	
    if(!playing || freq <= 0) {
        return; // with an empty buffer
    }

    double phase_increment = 2.0 * PI * freq / SAMPLE_RATE;

    for(int i = 0; i < size; i++) {
        double sample;

		switch(waveform) {

			case SINEMIXER_WAVE_SINE:
				sample = sin(phase);
				break;

			case SINEMIXER_WAVE_SQUARE:
				sample = squareWave(phase);
				break;

			case SINEMIXER_WAVE_TRIANGLE:
				sample = triangleWave(phase);
				break;

			case SINEMIXER_WAVE_SAWTOOTH:
				sample = sawtoothWave(phase);
				break;

			case SINEMIXER_WAVE_STAIRS:
				sample = stairWave(phase);
				break;

			case SINEMIXER_WAVE_RECTIFIED_SINE:
				sample = rectifiedSineWave(phase);
				break;

			case SINEMIXER_WAVE_DECAYING_SINE:
				sample = decayingSineWave(phase);
				break;

			default:
				sample = 0;
				break;
		}

        buff[i] = (short)(sample * 32767);
        phase += phase_increment;
		
        if(phase >= 2.0 * PI) {
            phase -= 2.0 * PI;
        }
    }
}

static void* playbackThread(void* _arg)
{
	(void)_arg;
	while (!stopping) {
		// incrememt current freq to desired freq
		if(currentFrequency != desiredFrequency) {
			// dynamic change rate dependant on the distance between the old and new note.
			frequencyChangeRate = fabs(currentFrequency - desiredFrequency)/5;
			// set playing frequency to desired if below threshold
			// to avoid weird oscillation
			if(frequencyChangeRate <= 5) {
				currentFrequency = desiredFrequency;
			} 
			else if(currentFrequency < desiredFrequency) {
				currentFrequency += frequencyChangeRate;
			} 
			else {
				currentFrequency -= frequencyChangeRate;
			}
		}
		
		// Generate next block of audio
		fillPlaybackBuffer(playbackBuffer, playbackBufferSize);

		// Output the audio
		snd_pcm_sframes_t frames = snd_pcm_writei(handle,
				playbackBuffer, playbackBufferSize);

		// Check for (and handle) possible error conditions on output
		if (frames < 0) {
			fprintf(stderr, "AudioMixer: writei() returned %li\n", frames);
			frames = snd_pcm_recover(handle, frames, 1);
		}
		if (frames < 0) {
			fprintf(stderr, "ERROR: Failed writing audio with snd_pcm_writei(): %li\n",
					frames);
			exit(EXIT_FAILURE);
		}
		if (frames > 0 && frames < (long)playbackBufferSize) {
			printf("Short write (expected %li, wrote %li)\n",
					playbackBufferSize, frames);
		}
	}

	return NULL;
}