/*
 * This module contains utility functions used in both the HAL and applications.
 */

#ifndef _UTILS_H_
#define _UTILS_H_

#include <stdbool.h>
#include <time.h>
#include <stdint.h>

/**
 * Pauses the program for a number of milliseconds.
 * This function was obtained from the course LED Guide.
 *
 * @param delay_in_ms The delay duration in milliseconds.
 */
void sleep_for_ms(long long delay_in_ms);

/**
 * Checks if the joystick has been idle for a certain amount of time.
 * Used to exit input reading.
 *
 * @param start_time The start time from where the timeout is measured.
 * @param timeout_seconds The timeout duration limit in seconds.
 * @return True if the timeout limit has passed, otherwise false.
 */
bool has_timeout_passed(time_t start_time, int timeout_seconds);

/**
 * Gets the current time in milliseconds.
 * This function was derived from the ADC Guide on the course site.
 *
 * @return The time in milliseconds elapsed from January 1st 1970.
 */
long long get_time_in_ms(void);

/**
 * Gets the current time in microseconds.
 *
 * @return The time in microseconds elapsed from January 1st 1970.
 */
long long get_time_in_us(void);

/**
 * Gets the current time in nanoseconds.
 *
 * @return The time in nanoseconds elapsed from January 1st 1970.
 */
long long get_time_in_ns(void);

/**
 * Trims the newline character from a string (if present).
 *
 * @param str The string to trim.
 */
void trim_newline(char *str);

void write_string_to_file(char *fileName, char *input);

int get_data_from_file(char *fileName);

void write_int_to_file(char *fileName, int input);

#endif
