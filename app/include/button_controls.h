/*
 * This module is used to interface with the button controls.
 * It allows users to mute and unmute the device using the rotary button.
 */

#ifndef _BUTTON_CONTROLS_H_
#define _BUTTON_CONTROLS_H_


/*
 * This function initializes the button controls module and starts the button thread.
 */
void button_controls_init();


/*
 * This function cleans up the button controls module and stops the button thread.
 */
void button_controls_cleanup();

#endif