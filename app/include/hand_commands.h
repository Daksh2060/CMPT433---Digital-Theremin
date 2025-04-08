#ifndef _HAND_COMMANDS_H_
#define _HAND_COMMANDS_H_

#include <stdio.h>
#include <unistd.h>           

void command_handler_init();

void command_handler_update_current_command(int cmd);

void command_handler_cleanup();

int command_handler_getOctave();
void command_handler_setOctave(int octave);

#endif