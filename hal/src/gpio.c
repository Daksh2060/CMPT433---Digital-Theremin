/*
 * This file implements GPIO functionality using the gpiod library for event-based
 * GPIO handling, initialization, and cleanup.
 * It interacts with multiple GPIO chips and waits for events on specific GPIO lines.
 *
 * Dependencies:
 * - gpiod library: https://www.ics.com/blog/gpio-programming-exploring-libgpiod-library
 * - Cross-compilation setup for ARM64
 */

#include "gpio.h"
#include <stdlib.h>
#include <assert.h>
#include <stdio.h>
#include <gpiod.h>
#include <time.h>

// Timeout for event waiting in nanoseconds (100ms)
#define TIMEOUT_LIMIT_NS 100000000

// List of GPIO chip names
static char *chip_names[] = {
    "gpiochip0",
    "gpiochip1",
    "gpiochip2",
};

// Hold open GPIO chips
static struct gpiod_chip *open_gpiod_chips[GPIO_NUM_CHIPS];

// Flag indicating whether GPIOs have been initialized
static bool is_initialized = false;

void gpio_initialize(void)
{
    // Open each GPIO chip by name
    if (is_initialized){
        return;
    }

    for (int i = 0; i < GPIO_NUM_CHIPS; i++){
        open_gpiod_chips[i] = gpiod_chip_open_by_name(chip_names[i]);
        if (!open_gpiod_chips[i]){
            perror("GPIO Initializing: Unable to open GPIO chip");
            exit(EXIT_FAILURE);
        }
    }
    is_initialized = true;
}

struct GPIOLine *gpio_open_for_events(enum eGPIOChips chip, int pin_number)
{
    assert(is_initialized);

    // Get the GPIO line from the specified chip and pin number
    struct gpiod_chip *gpiod_chip = open_gpiod_chips[chip];
    struct gpiod_line *line = gpiod_chip_get_line(gpiod_chip, pin_number);
    if (!line){
        perror("Unable to get GPIO line");
        exit(EXIT_FAILURE);
    }

    // Cast to internal GPIOLine struct to hide gpiod dependency
    return (struct GPIOLine *)line;
}

int gpio_wait_for_line_change_one(
    struct GPIOLine *line1,
    struct gpiod_line_bulk *bulkEvents, 
    volatile bool *should_exit)
{
    struct gpiod_line_bulk bulkWait;
    gpiod_line_bulk_init(&bulkWait);
    gpiod_line_bulk_add(&bulkWait, (struct gpiod_line *)line1);

    gpiod_line_request_bulk_both_edges_events(&bulkWait, "Event Waiting");

    // Set timeout for event waiting
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = TIMEOUT_LIMIT_NS;

    while (!(*should_exit)){
        int result = gpiod_line_event_wait_bulk(&bulkWait, &timeout, bulkEvents);

        if (result == -1){
            return -1;
        }
        else if (result > 0){
            return gpiod_line_bulk_num_lines(bulkEvents);  // Return number of events
        }
    }
    return 0;  // No event or exit requested
}

int gpio_wait_for_line_change_two(struct GPIOLine *line1, struct GPIOLine *line2,
                                  struct gpiod_line_bulk *bulk_events, 
                                  volatile bool *should_exit)
{
    struct gpiod_line_bulk bulk_wait;
    gpiod_line_bulk_init(&bulk_wait);
    gpiod_line_bulk_add(&bulk_wait, (struct gpiod_line *)line1);
    gpiod_line_bulk_add(&bulk_wait, (struct gpiod_line *)line2);

    // Request both edges events for both lines
    gpiod_line_request_bulk_both_edges_events(&bulk_wait, "Event Waiting");

    // Set timeout for event waiting
    struct timespec timeout;
    timeout.tv_sec = 0;
    timeout.tv_nsec = TIMEOUT_LIMIT_NS;

    // Wait for events until exit signal
    while (!(*should_exit)){
        int result = gpiod_line_event_wait_bulk(&bulk_wait, &timeout, bulk_events);

        if (result == -1){
            return -1;
        }
        else if (result > 0){
            return gpiod_line_bulk_num_lines(bulk_events);
        }
    }
    return 0;
}

void gpio_close(struct GPIOLine *line)
{
    gpiod_line_release((struct gpiod_line *)line);
}

void gpio_cleanup(void)
{
    if (!is_initialized){
        return;
    }
    for (int i = 0; i < GPIO_NUM_CHIPS; i++){
        gpiod_chip_close(open_gpiod_chips[i]);
        
        if (!open_gpiod_chips[i]){
            perror("GPIO Cleanup: Unable to close GPIO chip");
            exit(EXIT_FAILURE);
        }
    }
    is_initialized = false;
}
