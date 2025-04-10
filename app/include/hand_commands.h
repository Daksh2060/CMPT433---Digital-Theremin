/*
 * This module handles the translation of hand commands into 
 * corresponding actions for the sine mixer. It takes input from
 * the MediaPipe hand tracking system and translates it into alterations to
 * the currently playing wave.
 */

#ifndef _HAND_COMMANDS_H_
#define _HAND_COMMANDS_H_

#include <unistd.h>         
#include <stdio.h>
  

/*
 * This function initializes the command handler module and starts the command thread.
 */
void command_handler_init();


/**
 * This function updates the current hand command to be processed.
 * @param cmd The command to be processed.
 */
void command_handler_update_current_command(int cmd);


/**
 * This function retrieves the current octave level.
 * @return The current octave level.
 */
int command_handler_getOctave();


/**
 * This function sets the current octave level.
 * @param octave The octave level to set.
 */
void command_handler_setOctave(int octave);


/**
 * This function cleans up the command handler module and stops the command thread.
 */
void command_handler_cleanup();

#endif