// allows for the playback of a single continuous frequency.
#ifndef _SINE_MIXER_H
#define _SINE_MIXER_H

#define SINEMIXER_VOLUME_MAX 100
#define SINEMIXER_VOLUME_MIN 0

void SineMixer_init(void);
void SineMixer_cleanup(void);

/**
 * @brief queues the continous playback of the given frequency. 
 * Replaces the currently playing frequency if there is one.
 * @param frequency the frequency of the sine wave to be played
 */
void SineMixer_queueFrequency(double frequency);

/**
 * @brief stops the playback of the current frequency, if there is one.
 */
void SineMixer_stopPlayback(void);

int SineMixer_getVolume(void);
void SineMixer_setVolume(int newVolume);

#endif