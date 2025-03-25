/*
 * This module is used to interface with the Joystick button
 * through GPIO events. It allows the creation of a pthread which actively
 * reads the button state and increments a counter based on button presses.
 */

#ifndef _JOYSTICK_BUTTON_H_
#define _JOYSTICK_BUTTON_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>
#include <gpiod.h>

// Struct representing the Joystick button device
typedef struct {
    bool is_initialized; // Flag indicating if the button is initialized
} JoystickButton;


/**
 * Initializes the Joystick button and starts thread.
 * 
 * @param button A pointer to the JoystickButton struct to initialize.
 */
void joystick_button_init(JoystickButton *button);


/**
 * Gets the current value of the Joystick button.
 * 
 * @param button A pointer to the JoystickButton struct.
 * @return The current value of the Joystick button.
 */
int get_joystick_button_value(JoystickButton *button);


/**
 * Cleans up the Joystick button.
 * 
 * @param button A pointer to the JoystickButton struct to clean up.
 */
void clean_joystick_button(JoystickButton *button);

#endif