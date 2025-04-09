#include <assert.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdatomic.h>
#include <pthread.h>

#include "hal/rotary_encoder.h"
#include "hal/kits.h"
#include "hal/gpio.h"


//so the encoder has access to the setter function from KITS to change the tempo directly, rather than having another thread manage

//DATA FOR THE ENCODER
#define GPIO_CHIP          GPIO_CHIP_2
#define GPIO_LINE_A        7
#define GPIO_LINE_B        8
static struct GpioLine* s_lineA = NULL;
static struct GpioLine* s_lineB = NULL;

static pthread_t rotaryEncoderThreadID;
static bool isInitialized = false;
void *rotary_encoder_doState();

struct rotaryStateEvent {
    struct rotaryState* pNextRotaryState;
    void (*action)();
};

struct rotaryState {
    struct rotaryStateEvent risingA;
    struct rotaryStateEvent fallingA;
    struct rotaryStateEvent risingB;
    struct rotaryStateEvent fallingB;
};

static void on_clockwise(void) {
    set_tempo(get_tempo() + 1);
}

static void on_counterclockwise(void) {
    set_tempo(get_tempo() - 1);
}
//clk == channel A DT == channel Ba
//Used rotary encoder rotaryencoder diagram at https://www.instructables.com/A-Complete-Arduino-Rotary-Solution/
struct rotaryState rotaryStates[] = {
    {
        // rotaryState 0: IDLE
        .risingA = {&rotaryStates[0], NULL},
        .fallingA = {&rotaryStates[4], NULL},
        .risingB = {&rotaryStates[0], NULL},
        .fallingB = {&rotaryStates[1], NULL},
        //Falling A and Falling B decide which loop we go into from IDLE. If falling B we start the loop for CCW
        //if falling A we start the loop for CW
        //ive labelled the left hand loop for falling B as B and the right hand loop for falling A as A   
    },
    {
        // rotaryState 1: B (1,0)
        .risingA = {&rotaryStates[1], NULL},
        .fallingA = {&rotaryStates[2], NULL},
        .risingB = {&rotaryStates[0], NULL},
        .fallingB = {&rotaryStates[1], NULL},   
    },
    {
        // rotaryState 2: B (0,0)
        .risingA = {&rotaryStates[1], NULL},
        .fallingA = {&rotaryStates[2], NULL},
        .risingB = {&rotaryStates[3], NULL},
        .fallingB = {&rotaryStates[2], NULL},
    },
    {
        // rotaryState 3: B (0,1)
        .risingA = {&rotaryStates[0], on_counterclockwise},
        .fallingA = {&rotaryStates[3], NULL},
        .risingB = {&rotaryStates[3], NULL},
        .fallingB = {&rotaryStates[2], NULL},
    },
    {
        // rotaryState 4: A (0,1)
        .risingA = {&rotaryStates[0], NULL},
        .fallingA = {&rotaryStates[4], NULL},
        .risingB = {&rotaryStates[4], NULL},
        .fallingB = {&rotaryStates[5], NULL},
    },
    {
        // rotaryState 5: A (0,0)
        .risingA = {&rotaryStates[6], NULL},
        .fallingA = {&rotaryStates[5], NULL},
        .risingB = {&rotaryStates[4], NULL},
        .fallingB = {&rotaryStates[5], NULL},
    },
    {
        // rotaryState 6: A (1,0)
        .risingA = {&rotaryStates[6], NULL},
        .fallingA = {&rotaryStates[5], NULL},
        .risingB = {&rotaryStates[0], on_clockwise},
        .fallingB = {&rotaryStates[6], NULL},
    },
};
static struct rotaryState* pCurrentRotaryState = &rotaryStates[0];


//inits gpio creates a thread which is rotary..dorotaryState()
void rotary_encoder_init(void){
    assert(!isInitialized);
    s_lineA = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_A);
    s_lineB = Gpio_openForEvents(GPIO_CHIP, GPIO_LINE_B);
    isInitialized = true;

    if(pthread_create(&rotaryEncoderThreadID, NULL, rotary_encoder_doState, NULL) != 0){
        fprintf(stderr, "ERROR: Could not initialize Rotary Encoder Thread");
        exit(EXIT_FAILURE);
    }
}

void rotary_encoder_cleanup(void) {
    assert(isInitialized);
    Gpio_close(s_lineA);
    Gpio_close(s_lineB);
    
    isInitialized = false;
    pthread_cancel(rotaryEncoderThreadID);
    pthread_join(rotaryEncoderThreadID, NULL);
    printf("Cleaned Rotary Encoder\n");
}


//while on, get any gpio changes on line A or B, processes and gets next rotaryState
void *rotary_encoder_doState()
{
    assert(isInitialized);
    while(isInitialized){
        struct gpiod_line_bulk bulkEvents;
        int numEvents = Gpio_waitForLineChange(s_lineA,s_lineB,&bulkEvents);
        //Iterate over events from both lines
        for (int i = 0; i < numEvents; i++)
        {
            struct gpiod_line *line_handle = gpiod_line_bulk_get_line(&bulkEvents, i);
            unsigned int this_line_number = gpiod_line_offset(line_handle);
            struct gpiod_line_event event;
            if (gpiod_line_event_read(line_handle, &event) == -1) {
                perror("Line Event");
                exit(EXIT_FAILURE);
            }
            bool isRising = event.event_type == GPIOD_LINE_EVENT_RISING_EDGE;
            struct rotaryStateEvent* pRotaryStateEvent = NULL;
            if (this_line_number == GPIO_LINE_A) {
                pRotaryStateEvent = isRising ? &pCurrentRotaryState->risingA : &pCurrentRotaryState->fallingA;
            } else if (this_line_number == GPIO_LINE_B) {
                pRotaryStateEvent = isRising ? &pCurrentRotaryState->risingB : &pCurrentRotaryState->fallingB;
            }
            if (pRotaryStateEvent && pRotaryStateEvent->action) {
            pRotaryStateEvent->action();
            }
            pCurrentRotaryState = pRotaryStateEvent->pNextRotaryState;    
        }
    }
    pthread_exit(NULL);
   
}