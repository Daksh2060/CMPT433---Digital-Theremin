/*
 * This file implements the joystick button module, handling state
 * machine logic, GPIO event processing, and thread management for the button clicks.
 */

#include "joystick_button.h"
#include "joystick.h"
#include "utils.h"
#include "gpio.h"
#include "i2c.h"
#include <stdatomic.h>
#include <pthread.h>
#include <unistd.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <fcntl.h>

// Pin configuration for the joystick button
//   $ gpiofind GPIO24
//   >> gpiochip0 10
#define GPIO_CHIP_JOYSTICK GPIO_CHIP_2
#define GPIO_LINE_JOYSTICK 15

// Debounce interval in milliseconds
#define DEBOUNCE_INTERVAL 50

#define JOYSTICK_BUTTON_MAX 3  // Maximum value for the rotary button
#define JOYSTICK_BUTTON_MIN 1  // Minimum value for the rotary button

// Struct representing the Joystick button device
struct GPIOLine *joystick_button = NULL;

// Counter to track rotary button value, set to loop between 1-3.
static atomic_int joystick_button_counter = 1;

// Thread control variables
static bool joystick_thread_running = false;
static pthread_t joystick_button_thread;
static pthread_mutex_t joystick_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Flag to signal thread termination
static volatile bool exit_joystick_thread = false;

// Module initialization status
static bool joystick_initialized = false;

// Internal function prototypes for the joystick button
static void joystick_button_start_thread(JoystickButton *button);
static void joystick_button_do_state();
static void *joystick_thread(void *arg);
static void increment_joystick_counter();

// Struct representing a state event for the state machine
struct stateEvent{
    struct state *pNextState; // Pointer to the next state
    void (*action)();         // Action to be performed during the event
};

// Struct representing a state in the state machine
struct state{
    struct stateEvent rising;  // Event for rising edge
    struct stateEvent falling; // Event for falling edge
};

// Array of each state transition based on rising/falling edges
struct state joystick_states[] = {
    {
        .rising = {&joystick_states[0], NULL},
        .falling = {&joystick_states[1], NULL},
    },
    {
        .rising = {&joystick_states[0], increment_joystick_counter},
        .falling = {&joystick_states[1], NULL},
    }
};

// Pointer to the current state in the state machine
struct state *joystick_current_state = &joystick_states[0];

void joystick_button_init(JoystickButton *button)
{
    gpio_initialize();
    button->is_initialized = true;
    joystick_button = gpio_open_for_events(GPIO_CHIP_JOYSTICK, GPIO_LINE_JOYSTICK);
    joystick_initialized = true;
    joystick_button_start_thread(button);
}

int get_joystick_button_value(JoystickButton *button)
{
    assert(button->is_initialized);
    int value;
    pthread_mutex_lock(&joystick_counter_mutex);
    {
        value = joystick_button_counter;
    }
    pthread_mutex_unlock(&joystick_counter_mutex);
    return value;
}

void clean_joystick_button(JoystickButton *button)
{
    gpio_close(joystick_button);
    pthread_mutex_destroy(&joystick_counter_mutex);

    joystick_initialized = false;
    joystick_thread_running = false;
    exit_joystick_thread = true;
    pthread_join(joystick_button_thread, NULL);
    button->is_initialized = false;
}

static void *joystick_thread(void *arg)
{
    (void)arg;
    while (joystick_thread_running) { 
        joystick_button_do_state();
    }
    return NULL;
}

static void joystick_button_start_thread(JoystickButton *button)
{
    assert(button->is_initialized);
    joystick_thread_running = true;
    if(pthread_create(&joystick_button_thread, NULL, joystick_thread, NULL) != 0){
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    } 
}

static void increment_joystick_counter()
{
    static long last_press_time = 0;
    long current_time = get_time_in_ms();

    // Debounce: Ignore if the press is within 50ms of the last one
    if (current_time - last_press_time < DEBOUNCE_INTERVAL) {
        return;
    }

    pthread_mutex_lock(&joystick_counter_mutex);
    {   
        if (joystick_button_counter == JOYSTICK_BUTTON_MAX){
            joystick_button_counter = JOYSTICK_BUTTON_MIN;
        }
        else {
            joystick_button_counter++;
        }
    }
    pthread_mutex_unlock(&joystick_counter_mutex);

    last_press_time = current_time;
}

static void joystick_button_do_state()
{
    assert(joystick_initialized);

    struct gpiod_line_bulk bulk_events;
    int num_events = gpio_wait_for_line_change_one(joystick_button, &bulk_events, &exit_joystick_thread);

    // Exit thread if program ends from GPIO timeout
    if (num_events == -1) {
        return;
    }

    for (int i = 0; i < num_events; i++){
        struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulk_events, i);

        struct gpiod_line_event event;
        if (gpiod_line_event_read(line_handle, &event) == -1){
            return;
        }

        bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

        struct stateEvent *pStateEvent = NULL;
        if (isRising){
            pStateEvent = &joystick_current_state->rising;
        }
        else{
            pStateEvent = &joystick_current_state->falling;
        }

        if (pStateEvent->action != NULL){
            pStateEvent->action();
        }
        joystick_current_state = pStateEvent->pNextState;
    }
}
