#ifndef _DIAL_CONTROLS_H_
#define _DIAL_CONTROLS_H_

typedef enum
{
    REST,
    VOLUME,
    PERIOD,
    WAVEFORM,
    BRIGHTNESS
} Control;






Control get_current_control();

bool is_mute();

int get_volume();

int get_period();

int get_waveform();

int get_brightness();




void dial_controls_init();

void get_dial_volume(int *vol);

void set_dial_volume(int vol);

void toggle_mute();

void dial_controls_cleanup();



#endif