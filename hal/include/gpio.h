/*
 * This module for low-level GPIO access using libgpiod. Includes functions 
 * for initializing GPIO chips, opening GPIO lines for event reading, and
 * waiting for line changes.
 */

#ifndef _GPIO_H_
#define _GPIO_H_

#include <stdbool.h>
#include <gpiod.h>

// Structure representing a GPIO line
struct GPIOLine;

// Enumeration of available GPIO chips.
enum eGPIOChips {
    GPIO_CHIP_0,       // GPIO chip 0
    GPIO_CHIP_1,       // GPIO chip 1
    GPIO_CHIP_2,       // GPIO chip 2
    GPIO_NUM_CHIPS     // Total number of GPIO chips
};


/**
 * Initializes the GPIO subsystem. Must be called before any GPIO functions.
 */
void gpio_initialize(void);


/**
 * Opens a GPIO line for event handling.
 * 
 * @param chip The GPIO chip to use (e.g., GPIO_CHIP_0).
 * @param pinNumber The GPIO pin number to open (e.g., 15).
 * @return A pointer to the GPIOLine structure representing the opened line.
 */
struct GPIOLine* gpio_open_for_events(enum eGPIOChips chip, int pin_number);


/**
 * Waits for changes on one specified GPIO line.
 * 
 * @param line1 Pointer to the first GPIO line to monitor.
 * @param bulk_events Pointer to a gpiod_line_bulk structure for event storage.
 * @param should_exit Pointer to a flag used for graceful thread exit.
 * @return 0 on success, or a negative value on failure.
 */
int gpio_wait_for_line_change_one(
    struct GPIOLine* line1, 
    struct gpiod_line_bulk *bulkEvents, 
    volatile bool *should_exit
);


/**
 * Waits for changes on two specified GPIO lines.
 * 
 * @param line1 Pointer to the first GPIO line to monitor.
 * @param line2 Pointer to the second GPIO line to monitor.
 * @param bulk_events Pointer to a gpiod_line_bulk structure for event storage.
 * @param should_exit Pointer to a flag used for graceful thread exit.
 * @return 0 on success, or a negative value on failure.
 */
int gpio_wait_for_line_change_two(
    struct GPIOLine* line1,
    struct GPIOLine* line2,
    struct gpiod_line_bulk *bulk_events,
    volatile bool *should_exit
);


/**
 * Closes the specified GPIO line and releases associated resources.
 * 
 * @param line Pointer to the GpioLine structure representing the line to close.
 */
void gpio_close(struct GPIOLine* line);


/**
 * Cleans up the GPIO subsystem. Should be called after all GPIO operations are complete. Handles all GPIO lines at once, only once per program run.
 */
void gpio_cleanup(void);

#endif
