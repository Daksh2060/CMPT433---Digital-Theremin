// allows for the playback of a single continuous frequency.
#ifndef _SINE_MIXER_H
#define _SINE_MIXER_H

#define SINEMIXER_VOLUME_MAX 100
#define SINEMIXER_VOLUME_MIN 0

enum SineMixer_waveform{
    SINEMIXER_WAVE_SINE,
    SINEMIXER_WAVE_SQUARE,
    SINEMIXER_WAVE_TRIANGLE,
    SINEMIXER_WAVE_SAWTOOTH,
    SINEMIXER_WAVE_STAIRS,
    SINEMIXER_WAVE_RECTIFIED_SINE,
    SINEMIXER_WAVE_DECAYING_SINE,
    SINEMIXER_WAVE_COUNT
};

void SineMixer_init(void);
void SineMixer_cleanup(void);

/**
 * @brief queues the continous playback of the given frequency. 
 * Replaces the currently playing frequency if there is one.
 * @param frequency the frequency of the sine wave to be played
 */
void SineMixer_queueFrequency(double frequency);
double SineMixer_getFrequency(void);

/**
 * @brief stops the playback of the current frequency, if there is one.
 */
void SineMixer_stopPlayback(void);

/**
 * @brief changes the currently playing waveform to the desired one.
 * @param waveform the waveform to change to.
 */
void SineMixer_setWaveform(enum SineMixer_waveform waveform);
enum SineMixer_waveform Sinemixer_getWaveform(void);

/**
 * @brief adds a random amount of offset to the playing frequency.
 * the amount of offset is relative to the given frequency
 * @param distortion the % amount of distortion to add to a given note.
 * for example, 0.01 adds 1% distortion
 */
void SineMixer_setDistortion(double distortion);
double SineMixer_getDistortion(void);

int SineMixer_getVolume(void);
void SineMixer_setVolume(int newVolume);

#endif