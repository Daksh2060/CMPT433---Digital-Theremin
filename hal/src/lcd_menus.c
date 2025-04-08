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


// Screen buffer
static UWORD *s_fb;



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
  is_initialized = true;


}

void lcd_menu_cleanup(){
  assert(is_initialized);
  //clean lcd memory
  free(s_fb);
  s_fb = NULL;
  DEV_ModuleExit();
  is_initialized = false;
}

void draw_statistics_screen(){
  //does nothing
}

void draw_hand_screen(int points[], int size){
  assert(is_initialized);
  assert(size %2 == 0);
  const int joint_radius = 3;

  Paint_NewImage(s_fb, LCD_1IN54_WIDTH, LCD_1IN54_HEIGHT, 0, WHITE, 16);
  Paint_Clear(WHITE);

  //draw joint points on scree
  for(int i = 0; i < size - 1; i+=2){
    int x = points[i];
    int y = points[i+1];
    if(x > 0 && x < LCD_1IN54_WIDTH && y > 0 && y < LCD_1IN54_HEIGHT){
      Paint_DrawCircle(x, y, joint_radius, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
    } 
  }

  //draw joint connections REFER TO JOINT MAP
  //wrist to thumb
  Paint_DrawLine(points[0], points[1], points[2], points[3], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED); //0-1
  Paint_DrawLine(points[2], points[3], points[4], points[5], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED); //1-2
  Paint_DrawLine(points[4], points[5], points[6], points[7], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED); //2-3
  Paint_DrawLine(points[6], points[7], points[8], points[9], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED); //3-4

  //wrist to index tip
  Paint_DrawLine(points[0], points[1], points[10], points[11], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED); //0-5
  Paint_DrawLine(points[10], points[11], points[12], points[13], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//5-6
  Paint_DrawLine(points[12], points[13], points[14], points[15], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//6-7
  Paint_DrawLine(points[14], points[15], points[16], points[17], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//7-8

  //upper palm
  Paint_DrawLine(points[10], points[11], points[18], points[19], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//5-9
  Paint_DrawLine(points[18], points[19], points[26], points[27], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//9-13
  Paint_DrawLine(points[26], points[27], points[34], points[35], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//13-17

  //base middle to middle tip
  Paint_DrawLine(points[18], points[19], points[20], points[21], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//9-10
  Paint_DrawLine(points[20], points[21], points[22], points[23], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//10-11
  Paint_DrawLine(points[22], points[23], points[24], points[25], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//11-12

  //base ring to ring tip
  Paint_DrawLine(points[26], points[27], points[28], points[29], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//13-14
  Paint_DrawLine(points[28], points[29], points[30], points[31], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//14-15
  Paint_DrawLine(points[30], points[31], points[32], points[33], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//15-16

  //wrist to pinky tip
  Paint_DrawLine(points[0], points[1], points[34], points[35], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//0-17
  Paint_DrawLine(points[34], points[35], points[36], points[37], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//17-18
  Paint_DrawLine(points[36], points[37], points[38], points[39], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//18-19
  Paint_DrawLine(points[38], points[39], points[40], points[41], BLACK, DOT_PIXEL_1X1, LINE_STYLE_DOTTED);//19-20
  

  LCD_1IN54_Display(s_fb);


}



