#ifndef _KNOB_CONTROLS_H_
#define _KNOB_CONTROLS_H_

void knob_controls_init();

void get_knob_volume(int *vol);

void set_knob_volume(int vol);

void toggle_mute();

void knob_controls_cleanup();

#endif