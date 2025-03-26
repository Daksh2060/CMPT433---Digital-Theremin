
#include <stdio.h>
#include "knob_controls.h"
#include "utils.h"
#include "rotary_encoder.h"
#include "lcd.h"
#include "hand_commands.h"
#include "sine_mixer.h"

// some very bad code to avoid having to manually enable binary
// literals in the compiler
#define B0000 0
#define B1000 8
#define B0100 4
#define B0010 2
#define B0001 1
#define B1100 12
#define B0110 6
#define B0011 3
#define B1110 14
#define B0111 7 
#define B1111 15

int main()
{   
    SineMixer_init();
    command_handler_init();
    SineMixer_setVolume(100);
    int commands[] = {
        B0000,
        B1000,
        B0100,
        B0010,
        B0001,
        B1100,
        B0110,
        B0011,
        B1110,
        B0111,
        B1111
    };

    for(int i = 0; i < 11; i++) {
        command_handler_update_current_command(commands[i]);
        sleep_for_ms(5000);
    }

    command_handler_cleanup();
    SineMixer_cleanup();

    while (1)
    {
       sleep_for_ms(100);
    }
    knob_controls_cleanup();
    
    return 0;
}