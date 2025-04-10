/* 
 * This module is used to interface with the dial controls. Allows the user to
 * change the volume, octave, waveform, and distortion of the sound using the joystick
 * and rotary encoder.
 */

#ifndef _DIAL_CONTROLS_H_
#define _DIAL_CONTROLS_H_

#include "sine_mixer.h"
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// Different dials that can be controlled
typedef enum
{
    REST,
    VOLUME,
    OCTAVE,
    WAVEFORM,
    DISTORTION,
} Control;


/*
 * This function initializes the dial controls module and starts the control thread.
 */
void dial_controls_init();


/**
 * This function retrieves the current control state.
 * @return The current control state of the joystick.
 */
Control get_current_control();


/*
 * This function allows users to toggle the mute state of the device.
 */
void toggle_mute();


/** 
 * This function tells users if the device is muted or not.
 * @return True if the device is muted, false otherwise.
 */
bool is_mute();


/**
 * This function retrieves the current volume level.
 * @return The current volume level.
 */
int get_volume();


/**
 * This function retrieves the current octave level.
 * @return The current octave level.
 */
int get_octave();


/**
 * This function retrieves the current waveform type.
 * @return The current waveform type.
 */
enum SineMixer_waveform get_waveform();


/**
 * This function retrieves the current distortion level.
 * @return The current distortion level.
 */
double get_distortion();


/**
 * This function gets the dial value of volume for syncing between mutes.
 * @param vol Pointer to hold the previous volume save.
 */
void get_dial_volume(int *vol);


/**
 * This function sets the dial value of volume for syncing between mutes.
 * @param vol The volume to retore to.
 */
void set_dial_volume(int vol);


/*
 * This function cleans up the dial controls module and stops the control thread.
 */
void dial_controls_cleanup();

#endif