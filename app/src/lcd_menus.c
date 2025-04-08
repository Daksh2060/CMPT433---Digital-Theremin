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

#define POPUP_BORDER_WIDTH 3
#define POPUP_MARGIN_X 50
#define POPUP_MARGIN_Y 50

#define NUM_HAND_KEYPOINTS 42
//lcd menu initializer
bool is_initialized = false;

// Screen buffer
static UWORD *s_fb;

// Hand Datapoint Buffer
static int hand_keypoints[NUM_HAND_KEYPOINTS];

// Thread data
static pthread_t lcdMenuThreadID;
static pthread_mutex_t lcd_menu_mutex = PTHREAD_MUTEX_INITIALIZER;
static void *lcd_menu_thread();

//popup screens
static void draw_hand_screen(int arr[], int size);
static void draw_volume_popup();
static void draw_period_popup();
static void draw_waveform_popup();
static void draw_brightness_popup();

void set_keypoint_buff(int new_keyponts[], int size){
  assert(size == NUM_HAND_KEYPOINTS);
  pthread_mutex_lock(&lcd_menu_mutex);
  memcpy(hand_keypoints, new_keyponts, sizeof(int) * size);
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
  is_initialized = true;
  // Create LCD MENU Thread
  if(pthread_create(&lcdMenuThreadID, NULL, lcd_menu_thread, NULL) != 0){
    fprintf(stderr, "ERROR: Could not initialize Beat thread");
    exit(EXIT_FAILURE);
  }

}

static void *lcd_menu_thread(){
  assert(is_initialized);
  while(is_initialized){
    //copy the buffer to local
    pthread_mutex_lock(&lcd_menu_mutex);
    int keypoints[NUM_HAND_KEYPOINTS];
    memcpy(keypoints, hand_keypoints, sizeof(int) * NUM_HAND_KEYPOINTS);
    pthread_mutex_unlock(&lcd_menu_mutex);
    //pass local copy to draw function
    draw_hand_screen(keypoints, NUM_HAND_KEYPOINTS);
    sleep_for_ms(100);
  }
  pthread_exit(NULL);
}

void lcd_menu_cleanup(){
  assert(is_initialized);
  //clean lcd memory
  free(s_fb);
  s_fb = NULL;
  DEV_ModuleExit();
  is_initialized = false;
  pthread_join(lcdMenuThreadID, NULL);
  printf("Cleaned LCD Menu Thread\n");
  
}


static void draw_hand_screen(int points[], int size){
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
  

  //get current joystick state. If necessary we draw the corresponding popup ONTOP
  Control curr_control = get_current_control();
  switch(curr_control){
    case REST:
        break;
    case VOLUME:
        draw_volume_popup();
        break;
    case PERIOD:
        draw_period_popup();
        break;
    case WAVEFORM:
        draw_waveform_popup();
        break;
    case BRIGHTNESS:
        draw_brightness_popup();
        break;
    default:
        break;
  }

  LCD_1IN54_Display(s_fb);
}

static void draw_volume_popup(){
  Paint_DrawRectangle(POPUP_MARGIN_X - POPUP_BORDER_WIDTH, POPUP_MARGIN_Y - POPUP_BORDER_WIDTH, LCD_1IN54_WIDTH - POPUP_MARGIN_X + POPUP_BORDER_WIDTH, LCD_1IN54_HEIGHT - POPUP_MARGIN_Y + POPUP_BORDER_WIDTH, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(POPUP_MARGIN_X, POPUP_MARGIN_Y, LCD_1IN54_WIDTH - POPUP_MARGIN_X, LCD_1IN54_HEIGHT - POPUP_MARGIN_Y, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

  //draw volume text
}

static void draw_period_popup(){
  Paint_DrawRectangle(POPUP_MARGIN_X - POPUP_BORDER_WIDTH, POPUP_MARGIN_Y - POPUP_BORDER_WIDTH, LCD_1IN54_WIDTH - POPUP_MARGIN_X + POPUP_BORDER_WIDTH, LCD_1IN54_HEIGHT - POPUP_MARGIN_Y + POPUP_BORDER_WIDTH, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(POPUP_MARGIN_X, POPUP_MARGIN_Y, LCD_1IN54_WIDTH - POPUP_MARGIN_X, LCD_1IN54_HEIGHT - POPUP_MARGIN_Y, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

  //draw period text
}

static void draw_waveform_popup(){
  Paint_DrawRectangle(POPUP_MARGIN_X - POPUP_BORDER_WIDTH, POPUP_MARGIN_Y - POPUP_BORDER_WIDTH, LCD_1IN54_WIDTH - POPUP_MARGIN_X + POPUP_BORDER_WIDTH, LCD_1IN54_HEIGHT - POPUP_MARGIN_Y + POPUP_BORDER_WIDTH, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(POPUP_MARGIN_X, POPUP_MARGIN_Y, LCD_1IN54_WIDTH - POPUP_MARGIN_X, LCD_1IN54_HEIGHT - POPUP_MARGIN_Y, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

  //draw waveform text + waveform visualization
}

static void draw_brightness_popup(){
  Paint_DrawRectangle(POPUP_MARGIN_X - POPUP_BORDER_WIDTH, POPUP_MARGIN_Y - POPUP_BORDER_WIDTH, LCD_1IN54_WIDTH - POPUP_MARGIN_X + POPUP_BORDER_WIDTH, LCD_1IN54_HEIGHT - POPUP_MARGIN_Y + POPUP_BORDER_WIDTH, BLACK, DOT_PIXEL_1X1, DRAW_FILL_FULL);
  Paint_DrawRectangle(POPUP_MARGIN_X, POPUP_MARGIN_Y, LCD_1IN54_WIDTH - POPUP_MARGIN_X, LCD_1IN54_HEIGHT - POPUP_MARGIN_Y, WHITE, DOT_PIXEL_1X1, DRAW_FILL_FULL);

  //draw brightness text 
}
