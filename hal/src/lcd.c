/*
 * This file implements the LCD control module, handling initialization,
 * brightness adjustment, text display, and cleanup for the LCD screen.
 */

#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"
#include "lcd.h"
#include <stdbool.h> 
#include <assert.h> 
#include <stdlib.h>
#include <signal.h> 
#include <stdio.h>

// LCD formatting constants
#define MAX_LINES 25             // Maximum number of text lines allowed on the display
#define MIN_LINES 1              // Minimum number of text lines allowed on the display
#define MAX_BRIGTHNESS 1023      // Maximum backlight brightness value
#define MIN_BRIGTHNESS 0         // Minimum backlight brightness value
#define FONT_SIZE 16             // Font size for text display

// Positioning and layout constants
#define LCD_MARGIN_LEFT 5       // Left margin for all text on the display
#define LCD_FIRST_LINE_Y 70     // Y position where the first line of text starts
#define LCD_LINE_HEIGHT 20      // Vertical space between each line of text

// Framebuffer to store image data for the display
static UWORD *s_fb;

// Flag to track whether the LCD is initialized
static bool is_initialized = false;

void lcd_init(LCD *lcd)
{   
    assert(lcd != NULL);
    assert(!is_initialized);
    assert(!lcd->is_initialized); 
    
    if(DEV_ModuleInit() != 0){
        DEV_ModuleExit();
        exit(EXIT_FAILURE);
    }
    
    // Wait for the hardware to settle
    DEV_Delay_ms(2000);
    LCD_1IN54_Init(HORIZONTAL);
    LCD_1IN54_Clear(WHITE);      
    LCD_SetBacklight(MAX_BRIGTHNESS);

    // Allocate memory for the framebuffer
    UDOUBLE Imagesize = LCD_1IN54_HEIGHT * LCD_1IN54_WIDTH * 2;
    if((s_fb = (UWORD *)malloc(Imagesize)) == NULL) {
        perror("Failed to apply for black memory");
        exit(EXIT_FAILURE);
    }
    is_initialized = true;
    lcd->is_initialized = true; 
}

void lcd_set_brightness(LCD *lcd, int level)
{
    assert(lcd != NULL);
    assert(is_initialized);
    assert(lcd->is_initialized);
    
    // Clamp the brightness level between MIN and MAX
    if(level < MIN_BRIGTHNESS) {
        level = MIN_BRIGTHNESS;
    } 
    else if(level > MAX_BRIGTHNESS) {
        level = MAX_BRIGTHNESS;
    }
    LCD_SetBacklight(level);
}

void lcd_display_text(LCD *lcd, const char** messages, const int* font_sizes, int num_lines)
{
    assert(lcd != NULL);
    assert(is_initialized);
    assert(lcd->is_initialized);
    assert(messages != NULL);    
    
    if(num_lines > MAX_LINES) {
        printf("Number of lines exceeds maximum allowed\n");
        exit(EXIT_FAILURE);
    }

    if(num_lines < MIN_LINES) {
        printf("Number of lines is less than minimum allowed\n");
        exit(EXIT_FAILURE);
    }

    // Initialize the paint context
    Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 24);
    Paint_Clear(WHITE);

    // Loop through each line and display the message with the font
    for (int i = 0; i < num_lines; i++) {
        int currentY = LCD_FIRST_LINE_Y + (i * LCD_LINE_HEIGHT);

        if (font_sizes[i] == 24) {
            Paint_DrawString_EN(LCD_MARGIN_LEFT, currentY, messages[i], &Font24, WHITE, BLACK);
        } 
        else if (font_sizes[i] == 16) {
            Paint_DrawString_EN(LCD_MARGIN_LEFT, currentY, messages[i], &Font16, WHITE, BLACK);
        }
    }

    // Update the LCD display with the drawn text
    LCD_1IN54_DisplayWindows(LCD_MARGIN_LEFT, LCD_FIRST_LINE_Y, 
                             LCD_1IN54_WIDTH, LCD_FIRST_LINE_Y + 
                             (num_lines * LCD_LINE_HEIGHT), s_fb);
}



void lcd_clear(LCD *lcd)
{
    assert(lcd != NULL);
    assert(is_initialized);     
    assert(lcd->is_initialized);

    // Clear the screen with white color
    LCD_1IN54_Clear(WHITE);
}

void lcd_cleanup(LCD *lcd)
{   
    assert(lcd != NULL);
    assert(is_initialized);      
    assert(lcd->is_initialized); 
    
    lcd_clear(lcd);             
    lcd_set_brightness(lcd, MIN_BRIGTHNESS); 
    
    DEV_ModuleExit();           
    free(s_fb);                
    s_fb = NULL;             
    is_initialized = false;
    lcd->is_initialized = false; 
}