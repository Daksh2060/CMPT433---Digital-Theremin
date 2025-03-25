/*
 * This module is used to control the LCD display. It supports
 * adjusting brightness, displaying multiple lines of text,
 * clearing the display, and performs cleanup operations. Note that
 * it causes a memory leak at cleanup due to the lgpio library.
 */

#ifndef _LCD_H_
#define _LCD_H_

#include <stdbool.h>

// Struct of a LCD device for extra security
typedef struct {
    bool is_initialized; // Flag indicating if LCD is initialized
} LCD;


/**
 * Initializes the LCD display.
 * 
 * @param lcd A pointer to the LCD struct to initialize.
 */
void lcd_init(LCD *lcd);


/**
 * Sets the brightness level of the LCD display.
 * 
 * @param lcd A pointer to the LCD struct.
 * @param level The brightness level to set between (0, 1023).
 */
void lcd_set_brightness(LCD *lcd, int level);


/**
 * Displays multiple lines of text on the LCD.
 * 
 * @param lcd A pointer to the LCD struct.
 * @param messages An array of strings, each displayed on a new line (no '\n').
 * @param numLines The number of lines to display.
 * @param font_sizes An array of font sizes for each line of text.
 */
void lcd_display_text(LCD *lcd, const char** messages, const int* font_sizes, int num_lines);


/**
 * Clears the LCD and sets to a blank white screen.
 * 
 * @param lcd A pointer to the LCD struct to clear.
 */
void lcd_clear(LCD *lcd);


/**
 * Clears LCD, turns off display, and handles closing file.
 * 
 * @param lcd A pointer to the LCD struct to clean up.
 */
void lcd_cleanup(LCD *lcd);

#endif
 