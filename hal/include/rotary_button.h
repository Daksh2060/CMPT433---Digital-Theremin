/*
 * This module is used to interface with the Rotary Encoder button
 * through GPIO events. It allows the creation of a pthread which actively
 * reads the button state and increments a counter based on button presses.
 */

#ifndef _ROTARY_BUTTON_H_
#define _ROTARY_BUTTON_H_

#include <stdbool.h>
#include <stdlib.h>
#include <stdio.h>

// Struct representing the Rotary Encoder button device
typedef struct {
    bool is_initialized; // Flag indicating if the button is initialized
} RotaryButton;

/**
 * Initializes the Rotary Encoder button and starts thread.
 * 
 * @param button A pointer to the RotaryButton struct to initialize.
 */
void rotary_button_init(RotaryButton *button);


/**
 * Sets the value of the button counter manually.
 * 
 * @param value The value to set the button counter to.
 */
void set_rotary_button_value(int value);


/**
 * Gets the current value of the Rotary Encoder button.
 * 
 * @param button A pointer to the RotaryButton struct.
 * @return The current value of the Rotary Encoder button.
 */
int get_rotary_button_value(RotaryButton *button);


/**
 * Cleans up the Rotary Encoder button.
 * 
 * @param button A pointer to the RotaryButton struct to clean up.
 */
void clean_rotary_button(RotaryButton *button);

#endif