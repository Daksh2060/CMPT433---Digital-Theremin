#ifndef _DIAL_CONTROLS_H_
#define _DIAL_CONTROLS_H_

void dial_controls_init();

void get_dial_volume(int *vol);

void set_dial_volume(int vol);

void toggle_mute();

void dial_controls_cleanup();

#endif