#include "lcd_menus.h"
#include "DEV_Config.h"
#include "LCD_1in54.h"
#include "GUI_Paint.h"
#include "GUI_BMP.h"

#include "lcd_menus.h"
#include "utils.h"
#include "dial_controls.h"

#include <stdio.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>
#include <assert.h>

#define LCD_MARGIN_LEFT 5       // Left margin for all text on the display
#define LCD_FIRST_LINE_Y 70     // Y position where the first line of text starts
#define LCD_LINE_HEIGHT 20      // Vertical space between each line of text


//lcd menu initializer
bool is_initialized = false;

// LCD Thread Data
static pthread_t lcd_thread_id;
static pthread_mutex_t lcd_menu_mutex = PTHREAD_MUTEX_INITIALIZER;
static void *lcd_menu_thread();

// LCD Menu Data
static LCD_MENU current_menu = STATISTICS_MENU;
typedef void (*MenuFunction)(void);
static void statistics_menu(); // Can be extended for more menu functions
static void hand_menu();

// Screen buffer
static UWORD *s_fb;

// Get Current menu and Set Current menu
LCD_MENU get_menu(){
  pthread_mutex_lock(&lcd_menu_mutex);
  LCD_MENU menu_mode_ret = current_menu;
  pthread_mutex_unlock(&lcd_menu_mutex);
  return menu_mode_ret;
}
void set_menuMode(LCD_MENU new_menu_mode){
  pthread_mutex_lock(&lcd_menu_mutex);
  current_menu = new_menu_mode % NUM_MENUS;
  pthread_mutex_unlock(&lcd_menu_mutex);
}

void lcd_menu_init(){
  assert(!is_initialized);
  //initialize LCD
  if(DEV_ModuleInit() != 0){
      DEV_ModuleExit();
      exit(0);
  }
  // LCD Init
  DEV_Delay_ms(2000);
  LCD_1IN54_Init(HORIZONTAL);
  LCD_1IN54_Clear(WHITE);
  LCD_SetBacklight(1023);
  UDOUBLE Imagesize = LCD_1IN54_HEIGHT*LCD_1IN54_WIDTH*2;
  if((s_fb = (UWORD *)malloc(Imagesize)) == NULL) {
      perror("Failed to apply for black memory");
      exit(0);
  }

  //create LCD thread
  is_initialized = true;
  if(pthread_create(&lcd_thread_id, NULL, lcd_menu_thread, NULL) != 0){
    fprintf(stderr, "ERROR: Could not initialize LCD thread");
    exit(EXIT_FAILURE);
  }
}

void lcd_menu_cleanup(){
  assert(is_initialized);
  //clean lcd memory
  free(s_fb);
  s_fb = NULL;
  DEV_ModuleExit();
  //clean thread
  is_initialized = false;
  pthread_join(lcd_thread_id, NULL);
  printf("Cleaned LCD Thread\n");
}


static void statistics_menu(){
  assert(is_initialized);
  //write stats info to screen
  char *volume_msg = "Volume: ";
  char  volume_buff[30];
  char *period_msg = "Period: ";
  char period_buff[30];
  char *waveform_msg = "Waveform: ";
  char waveform_buff[30];
  char *brightness_msg = "Brightness: ";
  char brightness_buff[30]; 

  snprintf(volume_buff, sizeof(volume_buff), "%s%d", volume_msg, get_vol());
  snprintf(period_buff, sizeof(period_buff),"%s%d", period_msg, get_period());
  snprintf(waveform_buff, sizeof(waveform_buff), "%s%d", waveform_msg, get_waveform());
  snprintf(brightness_buff, sizeof(brightness_buff), "%s%d", brightness_msg, get_brightness());
  char *messages[] = {volume_buff, period_buff, waveform_buff, brightness_buff};
  int num_msg = 4;
  
  Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
  Paint_Clear(WHITE);

  int x_pos = LCD_MARGIN_LEFT;
  int y_pos = LCD_FIRST_LINE_Y;
  for(int i = 0; i < num_msg; i++){
    Paint_DrawString_EN(x_pos, y_pos, messages[i], &Font16, WHITE, BLACK);
    
    y_pos += LCD_LINE_HEIGHT;
    if(y_pos + LCD_LINE_HEIGHT > LCD_1IN54_HEIGHT){break;}
  }
  LCD_1IN54_Display(s_fb);


}

void hand_menu(){
  assert(is_initialized);
  //draw hand to screen

  
}



static void *lcd_menu_thread(){
  assert(is_initialized);
  MenuFunction func = NULL;

  while(is_initialized){
    pthread_mutex_lock(&lcd_menu_mutex);
    switch(current_menu){
        case STATISTICS_MENU:
          func = statistics_menu;
          break;
        case HAND_MENU:
          func = hand_menu;
          break;
        default:
          printf("Unknown Menu. Opening stats menu\n");
          func = statistics_menu;
          break;
    }
    pthread_mutex_unlock(&lcd_menu_mutex);
    //draw current menu to screen
    func();
    sleep_for_ms(100);
    
  }
  pthread_exit(NULL);

}