#include "knob_controls.h"
#include "rotary_encoder.h"
#include "joystick.h"
#include "utils.h"
#include "sine_mixer.h"
#include <pthread.h>
#include <stdio.h>

static int volume = 50;
static int period = 50;
static int waveform = 50;
static int brightness = 50;

typedef enum
{
    REST,
    VOLUME,
    PERIOD,
    WAVEFORM,
    BRIGHTNESS
} Control;

static Control current_control = REST;

static pthread_t control_thread;
static pthread_t value_thread;
static pthread_mutex_t control_mutex = PTHREAD_MUTEX_INITIALIZER;

Joystick joystick;
RotaryEncoder encoder;

static void *value_thread_func(void *arg);
static void *control_thread_func(void *arg);
static void set_value();
static void set_direction();

void knob_controls_init()
{
    joystick_init(&joystick);
    rotary_encoder_init(&encoder);

    if (pthread_create(&control_thread, NULL, control_thread_func, NULL) != 0)
    {
        perror("Failed to create control thread");
    }
    if (pthread_create(&value_thread, NULL, value_thread_func, NULL) != 0)
    {
        perror("Failed to create value thread");
    }
}

void knob_controls_cleanup()
{
    joystick_cleanup(&joystick);
    rotary_encoder_cleanup(&encoder);
}

static void set_direction()
{
    int x = joystick_read_input(&joystick, JOYSTICK_X);
    sleep_for_ms(3);
    int y = joystick_read_input(&joystick, JOYSTICK_Y);

    // printf("X: %d, Y: %d\n", x, y);

    int direction_change;

    if (y < -50 && (x < 25 && x > -25))
    {
        direction_change = VOLUME;
    }
    else if (y > 50 && (x < 25 && x > -25))
    {
        direction_change = PERIOD;
    }
    else if (x > 50 && (y < 25 && y > -25))
    {
        direction_change = WAVEFORM;
    }
    else if (x < -50 && (y < 25 && y > -25))
    {
        direction_change = BRIGHTNESS;
    }
    else
    {
        direction_change = REST;
    }

    //printf("Direction: %d\n", direction_change);

    pthread_mutex_lock(&control_mutex);
    {
        current_control = direction_change;
    }
    pthread_mutex_unlock(&control_mutex);
}

static void set_value()
{

    int current_state;
    pthread_mutex_lock(&control_mutex);
    {
        current_state = current_control;
    }
    pthread_mutex_unlock(&control_mutex);

    if (current_state == VOLUME)
    {
        rotary_encoder_set_value(volume);
        while (current_control == VOLUME)
        {
            int new_vol = rotary_encoder_get_value(&encoder);
            
            if (new_vol != volume)
            {
                volume = new_vol;
                //SineMixer_setVolume(volume);
                sleep_for_ms(10);
                printf("Volume: %d\n", volume);
            }
        }
    }

    else if (current_state == PERIOD)
    {
        rotary_encoder_set_value(period);
        while (current_control == PERIOD)
        {
            int new_period = rotary_encoder_get_value(&encoder);
            if (new_period != period)
            {
                period = new_period;
                // SineMixer_queueFrequency(1.0/period);
                sleep_for_ms(10);
                printf("Period: %d\n", period);
            }
        }
    }

    else if (current_state == WAVEFORM)
    {
        rotary_encoder_set_value(waveform);
        while (current_control == WAVEFORM)
        {
            int new_waveform = rotary_encoder_get_value(&encoder);
            if (new_waveform != waveform)
            {
                waveform = new_waveform;
                // Waveform
                sleep_for_ms(10);
                printf("Waveform: %d\n", waveform);
            }
        }
    }

    else if (current_state == BRIGHTNESS)
    {
        rotary_encoder_set_value(brightness);
        while (current_control == BRIGHTNESS)
        {
            int new_brightness = rotary_encoder_get_value(&encoder);
            if (new_brightness != brightness)
            {
                brightness = new_brightness;
                // Brightness
                sleep_for_ms(10);
                printf("Brightness: %d\n", brightness);
            }
        }
    }
    sleep_for_ms(100);
}

static void *control_thread_func(void *arg)
{
    (void)arg;
    while (1)
    {
        set_direction();
    }
    return NULL;
}

static void *value_thread_func(void *arg)
{
    (void)arg;
    while (1)
    {
        set_value();
    }
    return NULL;
}
