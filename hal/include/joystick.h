/*
 * This module is used to read the joystick input on the BeagleBone. The joystick
 * can be read along the X or Y axis, and the values are scaled to [-100, 100] in
 * both directions to make it easier to work with.
 */

#ifndef _JOYSTICK_H
#define _JOYSTICK_H

#include <stdbool.h>
#include <stdint.h>


// Enum representing the joystick directions.
typedef enum {
    JOYSTICK_X, // Joystick input along the X-axis.
    JOYSTICK_Y  // Joystick input along the Y-axis.
} JoystickDirection;

// Struct of a joystick.
typedef struct {
    int i2c_file_desc;   // I2C file descriptor for the joystick.
    bool is_initialized; // Flag to track if the joystick has been initialized
} Joystick;


/**
 * Initialize joystick by setting up communication with I2C.
 * 
 * @param joystick Pointer to the Joystick struct, created in main.
 */
void joystick_init(Joystick *joystick);


/**
 * Reads joystick input along the specified direction.
 * 
 * @param joystick Pointer to the Joystick struct to read.
 * @param dir The direction to read from (JOYSTICK_X or JOYSTICK_Y).
 * @return The value read from the joystick scaled to [-100,100].
 */
int joystick_read_input(Joystick *joystick, JoystickDirection dir);


/**
 * Clean up joystick by closing the I2C connection and uninitializing.
 * 
 * @param joystick Pointer to the Joystick struct to be cleaned up.
 */
void joystick_cleanup(Joystick *joystick);

#endif
