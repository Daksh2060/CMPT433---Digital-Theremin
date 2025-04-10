/*
 * This file implements the button controls module, handling the rotary encoder button. 
 * It allows users to mute and unmute the theremin by clicking the rotary button.
 */

#include "button_controls.h"
#include "joystick_button.h"
#include "rotary_button.h"
#include "dial_controls.h"
#include "utils.h"
#include <stdatomic.h>
#include <stdbool.h>
#include <pthread.h>

// Button control variables
JoystickButton joy_button;
RotaryButton rot_button;

// Mute state variables
static int prev_volume;  
bool button_pressed = false;

// Thread control variables
static bool exit_thread = false;
static pthread_t button_thread = 0;

// Helper function prototypes
static void read_rotary_button();
static void* button_thread_func(void* arg);

void button_controls_init()
{
    joystick_button_init(&joy_button);
    rotary_button_init(&rot_button);
    get_dial_volume(&prev_volume);

    if (pthread_create(&button_thread, NULL, button_thread_func, NULL) != 0){
        perror("Failed to create button thread");
    }
}

void button_controls_cleanup()
{
    exit_thread = true;

    int res = pthread_join(button_thread, NULL);
    if (res != 0){
        perror("Failed to join button thread");
    }

    clean_joystick_button(&joy_button);
    clean_rotary_button(&rot_button);
}

// Function to read the rotary button state and mute/unmute
static void read_rotary_button()
{
    int button = get_rotary_button_value(&rot_button);
    
    if (button == 1){
        if (!button_pressed){
            button_pressed = true;
            toggle_mute();
            get_dial_volume(&prev_volume);
            set_dial_volume(0);
        }
    }
    if (button == 0){
        if (button_pressed){
            button_pressed = false;
            set_dial_volume(prev_volume);
            toggle_mute();
        }
    }
}

// Function to read the state of the rotary button
static void* button_thread_func(void* arg)
{
    (void)arg;
    while (!exit_thread){
        read_rotary_button();
        sleep_for_ms(50);
    }
    return NULL;
}
