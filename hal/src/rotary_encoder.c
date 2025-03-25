/*
 * This file implements the rotary encoder module, handling state machine logic,
 * GPIO event processing, and thread management for a rotary encoder.
 */

#include "rotary_encoder.h"
#include "utils.h"
#include "gpio.h"
#include <stdatomic.h>
#include <pthread.h>
#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>


// Pin configuration for the rotary encoder channels
//   $ gpiofind GPIO24
//   >> gpiochip0 10
#define GPIO_CHIP GPIO_CHIP_2    // GPIO chip identifier for the rotary encoder
#define GPIO_LINE_NUMBER_A 7     // GPIO line number for encoder channel A
#define GPIO_LINE_NUMBER_B 8     // GPIO line number for encoder channel B

// Handles for GPIO lines A and B
struct GPIOLine *line_a = NULL;
struct GPIOLine *line_b = NULL;

// Counter to track rotary encoder value, thread-safe due to atomic type
static atomic_int counter = 120;

// Thread control variables
static bool thread_running = false;
static pthread_t state_machine_thread;
static pthread_mutex_t counter_mutex = PTHREAD_MUTEX_INITIALIZER;

// Flag to signal thread termination
static volatile bool exit_thread = false;

// Module initialization status
static bool is_initialized = false;

// Internal function prototypes
static void *rotary_encoder_thread(void *arg);
static void increment_counter();
static void decrement_counter();
static void rotary_encoder_do_state();

// Array of each state transition based on A/B rising/falling edges
static struct state states[] = {
    {{&states[1], NULL}, {&states[4], NULL}, {&states[0], NULL}, {&states[0], NULL}},
    {{&states[1], NULL}, {&states[2], NULL}, {&states[0], NULL}, {&states[1], NULL}},
    {{&states[2], NULL}, {&states[2], NULL}, {&states[3], NULL}, {&states[1], NULL}},
    {{&states[2], NULL}, {&states[3], NULL}, {&states[3], NULL}, {&states[0], increment_counter}},
    {{&states[5], NULL}, {&states[4], NULL}, {&states[4], NULL}, {&states[0], NULL}},
    {{&states[5], NULL}, {&states[5], NULL}, {&states[4], NULL}, {&states[6], NULL}},
    {{&states[5], NULL}, {&states[6], NULL}, {&states[0], decrement_counter}, {&states[6], NULL}},
};

// Pointer to the current state in the state machine
static struct state *current_state = &states[0];

void rotary_encoder_init(RotaryEncoder *rotary_encoder)
{
    assert(!is_initialized);
    assert(!rotary_encoder->is_initialized);
    gpio_initialize();
    
    // Sets both Line A and B to listen at the same time
    line_a = gpio_open_for_events(GPIO_CHIP, GPIO_LINE_NUMBER_A);
    line_b = gpio_open_for_events(GPIO_CHIP, GPIO_LINE_NUMBER_B);
    is_initialized = true;
    rotary_encoder->is_initialized = true;
}

void rotary_encoder_start_thread(RotaryEncoder *rotary_encoder)
{
    assert(is_initialized);
    assert(rotary_encoder->is_initialized);
    thread_running = true;

    if (pthread_create(&state_machine_thread, NULL, rotary_encoder_thread, NULL) != 0){
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }
}

int rotary_encoder_get_value(RotaryEncoder *rotary_encoder)
{
    assert(is_initialized);
    assert(rotary_encoder->is_initialized);

    int value;
    pthread_mutex_lock(&counter_mutex);
    {
        value = counter;
    }
    pthread_mutex_unlock(&counter_mutex);
    return value;
}

void rotary_encoder_set_value(int value)
{
    pthread_mutex_lock(&counter_mutex);
    {
        counter = value;
    }
    pthread_mutex_unlock(&counter_mutex);
}

void rotary_encoder_stop_thread(RotaryEncoder *rotary_encoder)
{
    assert(is_initialized);
    assert(rotary_encoder->is_initialized);
    thread_running = false;
    exit_thread = true;
    pthread_join(state_machine_thread, NULL);
}

void rotary_encoder_cleanup(RotaryEncoder *rotary_encoder)
{
    assert(is_initialized);
    assert(rotary_encoder->is_initialized);

    gpio_close(line_a);
    gpio_close(line_b);
    pthread_mutex_destroy(&counter_mutex);

    is_initialized = false;
    rotary_encoder->is_initialized = false;
}

// Thread function that continuously handles state transitions
static void *rotary_encoder_thread(void *arg)
{
    (void)arg;
    while (thread_running){
        rotary_encoder_do_state();
    }
    return NULL;
}

// Increments the counter, with an upper limit of 500
static void increment_counter()
{
    pthread_mutex_lock(&counter_mutex);
    {
        if (counter < 300){
            counter += 5;
        }
    }
    pthread_mutex_unlock(&counter_mutex);
}

// Decrements the counter, with a lower limit of 0
static void decrement_counter()
{
    pthread_mutex_lock(&counter_mutex);
    {
        if (counter > 40){
            counter -= 5;
        }
    }
    pthread_mutex_unlock(&counter_mutex);
}

// Core state machine logic: processes GPIO events and update states
static void rotary_encoder_do_state()
{
    assert(is_initialized);

    struct gpiod_line_bulk bulk;

    // Wait for an event on line A/B, and can exit thread if program ends
    int num = gpio_wait_for_line_change_two(line_a, line_b, &bulk, &exit_thread);
    if (num == -1) {
        return;
    }
    for (int i = 0; i < num; i++){
        struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulk, i);
        unsigned int this_line_number = gpiod_line_offset(line_handle);

        struct gpiod_line_event event;
        if (gpiod_line_event_read(line_handle, &event) == -1){
            perror("Line Event");
            exit(EXIT_FAILURE);
        }

        // Determine which state transition to take based on the line and event type
        struct stateEvent *state_event = NULL;

        if (this_line_number == GPIO_LINE_NUMBER_A) {
            if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
                state_event = &current_state->A_rising;
            } 
            else {
                state_event = &current_state->A_falling;
            }
        } 
        else {
            if (event.event_type == GPIOD_LINE_EVENT_RISING_EDGE) {
                state_event = &current_state->B_rising;
            } 
            else {
                state_event = &current_state->B_falling;
            }
        }

        // Execute action if defined (increment/decrement counter)
        if (state_event->action != NULL){
            state_event->action();
        }
        current_state = state_event->next_state;
    }
} 
