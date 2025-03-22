/*
 * Thismodule is used to playback sounds in real time, allowing multiple
 * simultaneous wave files to be mixed together and played without jitter.
 */

#ifndef AUDIO_MIXER_H
#define AUDIO_MIXER_H

// Maximum number of sound bites that can be played simultaneously.
typedef struct{
	int numSamples; // Number of samples in the wave file
	short *pData;	// Pointer to the wave file data
} wavedata_t;

// Maximum volume level
#define AUDIOMIXER_MAX_VOLUME 100

/*
 * Initialize the audio mixer. This function must be called before any other.
 */
void audio_mixer_init(void);


/*
 * Read the contents of a wave file into the pSound structure. Note that
 * the pData pointer in this structure will be dynamically allocated in
 * readWaveFileIntoMemory(), and is freed by calling freeWaveFileData().
 */
void audio_mixer_read_file(char *fileName, wavedata_t *pSound);


/*
 * Queue a sound to be played. The sound will be mixed with any other sounds
 * currently playing.
 */
void audio_mixer_queue_sound(wavedata_t *pSound);


/*
 * Get the current volume level. Function copied from:
 * http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
 */
int audio_mixer_get_volume();


/*
 * Set the volume level. Function copied from:
 * http://stackoverflow.com/questions/6787318/set-alsa-master-volume-from-c-code
 */
void audio_mixer_set_volume(int newVolume);


/*
 * Free the memory allocated for the wave file data.
 */
void audio_mixer_free_file_data(wavedata_t *pSound);


/*
 * Cleanup the audio mixer. This function must be called when the audio mixer
 * is no longer needed.
 */
void audio_mixer_cleanup(void);

#endif
