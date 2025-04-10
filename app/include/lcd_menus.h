/*
 * This module is used to interface with the LCD menus.
 * It allows the user to visualize different menus and
 * interact with the system using the LCD display.
 */

#ifndef _LCD_MENUS_H_
#define _LCD_MENUS_H_


/* 
 * This function initializes the LCD menus module and starts the menu thread.
 */
void lcd_menu_init();


/**
 * This function sets the keypoint buffer to the new keypoints.
 * @param new_keypoints The new keypoints to set.
 * @param size The size of the new keypoints array.
 */
void set_keypoint_buff(int new_keypoints[], int size);


/* 
 * This function cleans up the LCD menus module and stops the menu thread.
 */
void lcd_menu_cleanup();

#endif