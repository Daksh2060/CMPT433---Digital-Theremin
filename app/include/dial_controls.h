#ifndef _DIAL_CONTROLS_H_
#define _DIAL_CONTROLS_H_

#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <stdbool.h>

typedef enum
{
    REST,
    VOLUME,
    OCTAVE,
    WAVEFORM,
    DISTORTION,
} Control;






Control get_current_control();

bool is_mute();

int get_volume();

int get_octave();

int get_waveform();

int get_distortion();




void dial_controls_init();

void get_dial_volume(int *vol);

void set_dial_volume(int vol);

void toggle_mute();

void dial_controls_cleanup();



#endif