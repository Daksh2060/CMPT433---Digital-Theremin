/* 
 * This module is used to interface with a Sine Mixer. It constantly plays
 * a sine wave at a given frequency and allows the user to change the
 * frequency, waveform, and volume of the sound. It also allows for
 * distortion of the frequency to simulate different sound effects.
 */


#ifndef _SINE_MIXER_H
#define _SINE_MIXER_H

#define SINEMIXER_VOLUME_MAX 100
#define SINEMIXER_VOLUME_MIN 0

// Different waveforms that can be played
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


/*
 * Initializes the SineMixer module. Must be called before any other functions.
 * This function sets up the audio playback system and prepares it for use.
 */
void SineMixer_init(void);


/**
 * Queues the continous playback of the given frequency. 
 * Replaces the currently playing frequency if there is one.
 * @param frequency the frequency of the sine wave to be played
 */
void SineMixer_queueFrequency(double frequency);


/**
 * Gets the currently playing frequency.
 * @return the currently playing frequency.
 */
double SineMixer_getFrequency(void);


/*
 * Stops the playback of the current frequency, if there is one.
 */
void SineMixer_stopPlayback(void);


/**
 * Changes the currently playing waveform to the desired one.
 * @param waveform the waveform to change to.
 */
void SineMixer_setWaveform(enum SineMixer_waveform waveform);


/**
 * Gets the currently playing waveform.
 * @return the currently playing waveform.
 */
enum SineMixer_waveform Sinemixer_getWaveform(void);


/**
 * Adds a random amount of offset to the playing frequency.
 * the amount of offset is relative to the given frequency
 * @param distortion the % amount of distortion to add to a given note.
 * for example, 0.01 adds 1% distortion
 */
void SineMixer_setDistortion(double distortion);


/**
 * Gets the current distortion value.
 * @return the current distortion value.
 */
double SineMixer_getDistortion(void);


/**
 * Get the volume of the audio playback.
 * @return The current volume level (0-100).
 */
int SineMixer_getVolume(void);


/**
 * Sets the volume of the audio playback.
 * @param newVolume the new volume level (0-100).
 */
void SineMixer_setVolume(int newVolume);


/*
 * Cleans up the SineMixer resources and stops playback.
 */
void SineMixer_cleanup(void);

#endif