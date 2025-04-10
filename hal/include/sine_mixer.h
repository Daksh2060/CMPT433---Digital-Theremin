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
enum SineMixerWaveform
{
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
void sine_mixer_init(void);

/**
 * Queues the continous playback of the given frequency.
 * Replaces the currently playing frequency if there is one.
 * @param frequency the frequency of the sine wave to be played
 */
void sine_mixer_queue_frequency(double frequency);

/**
 * Gets the currently playing frequency.
 * @return the currently playing frequency.
 */
double sine_mixer_get_frequency(void);

/*
 * Stops the playback of the current frequency, if there is one.
 */
void sine_mixer_stop_playback(void);

/**
 * Changes the currently playing waveform to the desired one.
 * @param waveform the waveform to change to.
 */
void sine_mixer_set_waveform(enum SineMixerWaveform waveform);

/**
 * Gets the currently playing waveform.
 * @return the currently playing waveform.
 */
enum SineMixerWaveform sine_mixer_get_waveform(void);

/**
 * Adds a random amount of offset to the playing frequency.
 * the amount of offset is relative to the given frequency
 * @param distortion the % amount of distortion to add to a given note.
 * for example, 0.01 adds 1% distortion
 */
void sine_mixer_set_distortion(double distortion);

/**
 * Gets the current distortion value.
 * @return the current distortion value.
 */
double sine_mixer_get_distortion(void);

/**
 * Get the volume of the audio playback.
 * @return The current volume level (0-100).
 */
int sine_mixer_get_volume(void);

/**
 * Sets the volume of the audio playback.
 * @param newVolume the new volume level (0-100).
 */
void sine_mixer_set_volume(int newVolume);

/*
 * Cleans up the SineMixer resources and stops playback.
 */
void sine_mixer_cleanup(void);

#endif