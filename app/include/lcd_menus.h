#ifndef _LCD_MENUS_H_
#define _LCD_MENUS_H_

//Module Allows us to init and clean the LCD Display, and loop through Menu Modes
//Implementation of menu modes is hidden inside lcd_menus.c
//Access is via the getters and setters, the rest is handled independently

//enum for storing menu modes
typedef enum{
  STATISTICS_MENU,
  HAND_MENU,
} LCD_MENU;
#define NUM_MENUS 2
LCD_MENU get_menu();
void set_menu(LCD_MENU new_menu);

void lcd_menu_init();
void lcd_menu_cleanup();

#endif