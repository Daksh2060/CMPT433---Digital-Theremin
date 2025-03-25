/*
 * This module is used to interface with a Rotary Encoder.
 * It allows the creation of a pthread which actively reads
 * and adjusts the value of the Rotary Encoder based on CW
 * and CCW turns to increment and decrement.
 */

#ifndef _ROTARY_ENCODER_H_
#define _ROTARY_ENCODER_H_

#include <pthread.h>
#include <stdbool.h>

// Struct representing the Rotary Encoder device
typedef struct {
    bool is_initialized; // Flag indicating if the Rotary Encoder is initialized
} RotaryEncoder;

// Struct representing a state event for the state machine
struct stateEvent {
    struct state *next_state;  // Pointer to the next state
    void (*action)();          // Action to be performed during the event
};

// Struct representing a state in the state machine
struct state {
    struct stateEvent A_rising;  // Event for A pin rising edge
    struct stateEvent B_rising;  // Event for B pin rising edge
    struct stateEvent A_falling; // Event for A pin falling edge
    struct stateEvent B_falling; // Event for B pin falling edge
};


/**
 * Initializes the Rotary Encoder.
 * 
 * @param rotary_encoder A pointer to the RotaryEncoder struct to initialize.
 */
void rotary_encoder_init(RotaryEncoder *rotary_encoder);


/**
 * Starts the thread that handles the Rotary Encoder state changes.
 * 
 * @param rotary_encoder A pointer to the RotaryEncoder struct.
 */
void rotary_encoder_start_thread(RotaryEncoder *rotary_encoder);


/**
 * Sets the value of the Rotary Encoder.
 * 
 * @param value The value to set the Rotary Encoder counter to.
 */
void rotary_encoder_set_value(int value);


/**
 * Gets the current value of the Rotary Encoder.
 * 
 * @param rotary_encoder A pointer to the RotaryEncoder struct.
 * @return The current value of the Rotary Encoder counter.
 */
int rotary_encoder_get_value(RotaryEncoder *rotary_encoder);


/**
 * Stops the thread that handles the Rotary Encoder state changes.
 * 
 * @param rotary_encoder A pointer to the RotaryEncoder struct.
 */
void rotary_encoder_stop_thread(RotaryEncoder *rotary_encoder);


/**
 * Cleans up resources used by the Rotary Encoder.
 * 
 * @param rotary_encoder A pointer to the RotaryEncoder struct to clean up.
 */
void rotary_encoder_cleanup(RotaryEncoder *rotary_encoder);

#endif
 


