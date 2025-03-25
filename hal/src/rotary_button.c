/*
 * This file implements the rotary encoder button module, handling state
 * machine logic, GPIO event processing, and thread management for the button clicks.
 */

#include "rotary_button.h"
#include "joystick.h"
#include "utils.h"
#include "gpio.h"
#include "i2c.h"
#include <stdatomic.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>

// Pin configuration for the rotary encoder button
//   $ gpiofind GPIO24
//   >> gpiochip0 10
#define GPIO_CHIP_ROTARY GPIO_CHIP_0 // GPIO chip identifier for the rotary encoder button
#define GPIO_LINE_ROTARY 10          // GPIO line number for the rotary encoder button

// Debounce interval in milliseconds
#define DEBOUNCE_INTERVAL 50

#define ROTARY_BUTTON_MAX 3  // Maximum value for the rotary button
#define ROTARY_BUTTON_MIN 1  // Minimum value for the rotary button

// Struct representing the Rotary Encoder button device
struct GPIOLine *rotary_button = NULL;

// Counter to track rotary button value, set to loop between 1-3.
static atomic_int rotary_button_counter = 1;

// Thread control variables
static bool rotary_thread_running = false;
static pthread_t rotary_button_thread;
static pthread_mutex_t rotary_counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Flag to signal thread termination
static volatile bool exit_rotary_thread = false;

// Module initialization status
static bool rotary_initialized = false;

// Internal function prototypes for the rotary button
static void button_start_thread(RotaryButton *button);
static void *rotary_thread(void *arg);
static void rotary_button_do_state();
static void increment_rotary_counter();

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
static struct state rotary_states[] = {
    {
        .rising = {&rotary_states[0], NULL},
        .falling = {&rotary_states[1], NULL},
    },
    {
        .rising = {&rotary_states[0], increment_rotary_counter},
        .falling = {&rotary_states[1], NULL},
    }
};

// Pointer to the current state in the state machine
struct state *rotary_current_state = &rotary_states[0];

void rotary_button_init(RotaryButton *button)
{
    button->is_initialized = true;
    rotary_button = gpio_open_for_events(GPIO_CHIP_ROTARY, GPIO_LINE_ROTARY);
    rotary_initialized = true;
    button_start_thread(button);
    gpio_initialize();
}

void set_rotary_button_value(int value)
{
    pthread_mutex_lock(&rotary_counter_mutex);
    {
        rotary_button_counter = value;
    }
    pthread_mutex_unlock(&rotary_counter_mutex);
}

int get_rotary_button_value(RotaryButton *button)
{
    assert(button->is_initialized);
    int value;
    pthread_mutex_lock(&rotary_counter_mutex);
    {
        value = rotary_button_counter;
    }
    pthread_mutex_unlock(&rotary_counter_mutex);
    return value;
}

void clean_rotary_button(RotaryButton *button)
{
    assert(button->is_initialized);
    gpio_close(rotary_button);
    pthread_mutex_destroy(&rotary_counter_mutex);

    rotary_initialized = false;
    rotary_thread_running = false;
    exit_rotary_thread = true;
    pthread_join(rotary_button_thread, NULL);
}

static void *rotary_thread(void *arg)
{
    (void)arg;
    while (rotary_thread_running){
        rotary_button_do_state();
    }
    return NULL;
}

static void button_start_thread(RotaryButton *button)
{
    assert(button->is_initialized);
    rotary_thread_running = true;
    if (pthread_create(&rotary_button_thread, NULL, rotary_thread, NULL) != 0){
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }
}

static void increment_rotary_counter()
{
    static long last_press_time = 0;
    long current_time = get_time_in_ms();

    // Debounce: Ignore if the press is within 50ms of the last one
    if (current_time - last_press_time < DEBOUNCE_INTERVAL){
        return;
    }
    pthread_mutex_lock(&rotary_counter_mutex);
    {
        if (rotary_button_counter == ROTARY_BUTTON_MAX){
            rotary_button_counter = ROTARY_BUTTON_MIN;
        }
        else{
            rotary_button_counter++;
        }
    }
    pthread_mutex_unlock(&rotary_counter_mutex);
    last_press_time = current_time;
}

static void rotary_button_do_state()
{
    assert(rotary_initialized);

    struct gpiod_line_bulk bulk_events;
    int num_events = gpio_wait_for_line_change_one(rotary_button, &bulk_events, &exit_rotary_thread);

    // Exit thread if program ends from GPIO timeout
    if (num_events == -1){
        return;
    }

    for (int i = 0; i < num_events; i++){
        struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulk_events, i);

        struct gpiod_line_event event;
        if (gpiod_line_event_read(line_handle, &event) == -1){
            perror("Line Event");
            exit(EXIT_FAILURE);
        }

        bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;

        struct stateEvent *pStateEvent = NULL;
        if (isRising){
            pStateEvent = &rotary_current_state->rising;
        }
        else{
            pStateEvent = &rotary_current_state->falling;
        }

        if (pStateEvent->action != NULL){
            pStateEvent->action();
        }
        rotary_current_state = pStateEvent->pNextState;
    }
}