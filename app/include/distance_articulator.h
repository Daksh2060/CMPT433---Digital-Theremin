/*
 * This module is used to convert the data from the distance sensor
 * into a volume level for the sine mixer. It takes the distance
 * value from the distance sensor and linearly maps it to a volume level
 * based on the maximum set volume by the dials.
 */

#ifndef _DISTANCE_ARTICULATOR_
#define _DISTANCE_ARTICULATOR_

#include "sine_mixer.h"
#include <stdbool.h>

#define DISTANCE_ARTICULATOR_VOLUME_MAX SINEMIXER_VOLUME_MAX // Maximum volume level
#define DISTANCE_ARTICULATOR_VOLUME_MIN SINEMIXER_VOLUME_MIN // Minimum volume level


/*
 * Initializes the distance articulator module. This function must be called
 */
void distance_articulator_init(void);


/**
 * Sets the max volume level for the distance articulator.
 * @param volume The volume level to set (0-100) from the dial.
 */
void distance_articulator_set_volume(int volume);


/**
 * Gets the current max volume level for the distance articulator.
 * @return The current max volume level (0-100).
 */
int distance_acticulator_get_volume(void);


/**
 * Sets the mute state of the distance articulator.
 * @param isMuted True to mute, false to unmute.
 */
void distance_articulator_set_mute(bool isMuted);


/*
 * Cleans up the distance articulator module. Must call before extiting the program.
 */
void distance_articulator_cleanup(void);

#endif