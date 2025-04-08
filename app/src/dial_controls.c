#include "dial_controls.h"
#include "rotary_encoder.h"
#include "joystick.h"
#include "utils.h"
#include "sine_mixer.h"
#include <pthread.h>
#include <stdio.h>
#include "sine_mixer.h"
#include <math.h>

static int volume = 50;
static int octave = 0;
static int waveform = 0;
static double distortion = 0.00;

// Printing variables
static int last_volume = -1;
static int last_octave = -1;
static int last_waveform = -1;
static int last_distortion = -1;
static int last_mute = -1;

static bool exit_thread = false;

static Control current_control = REST;

static pthread_t control_thread;
static pthread_t value_thread;
static pthread_mutex_t control_mutex = PTHREAD_MUTEX_INITIALIZER;

static bool mute = false;

Joystick joystick;
RotaryEncoder encoder;

static void *value_thread_func(void *arg);
static void *control_thread_func(void *arg);
static void set_value();
static void set_direction();

void dial_controls_init()
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

Control get_current_control()
{
    Control control;
    pthread_mutex_lock(&control_mutex);
    {
        control = current_control;
    }
    pthread_mutex_unlock(&control_mutex);

    return control;
}

bool is_mute()
{
    bool is_muted;
    pthread_mutex_lock(&control_mutex);
    {
        is_muted = mute;
    }
    pthread_mutex_unlock(&control_mutex);

    return is_muted;
}

int get_volume()
{
    int vol;
    pthread_mutex_lock(&control_mutex);
    {
        vol = volume;
    }
    pthread_mutex_unlock(&control_mutex);

    return vol;
}

int get_octave()
{
    int oct;
    pthread_mutex_lock(&control_mutex);
    {
        oct = octave;
    }
    pthread_mutex_unlock(&control_mutex);

    return oct;
}

int get_waveform()
{
    int wave;
    pthread_mutex_lock(&control_mutex);
    {
        wave = waveform;
    }
    pthread_mutex_unlock(&control_mutex);

    return wave;
}

int get_distortion()
{
    int dist;
    pthread_mutex_lock(&control_mutex);
    {
        dist = distortion;
    }
    pthread_mutex_unlock(&control_mutex);

    return dist;
}

void get_dial_volume(int *vol)
{
    pthread_mutex_lock(&control_mutex);
    {
        *vol = volume;
    }
    pthread_mutex_unlock(&control_mutex);
}

void set_dial_volume(int vol)
{
    pthread_mutex_lock(&control_mutex);
    {
        volume = vol;
        rotary_encoder_set_value(volume);
    }
    pthread_mutex_unlock(&control_mutex);
}

void toggle_mute()
{
    pthread_mutex_lock(&control_mutex);
    {
        mute = !mute;
    }
    pthread_mutex_unlock(&control_mutex);
}

void dial_controls_cleanup()
{
    exit_thread = true;
    pthread_join(control_thread, NULL);
    pthread_join(value_thread, NULL);
    joystick_cleanup(&joystick);
    rotary_encoder_cleanup(&encoder);
}

static void set_direction()
{
    int x = joystick_read_input(&joystick, JOYSTICK_X);
    sleep_for_ms(3);
    int y = joystick_read_input(&joystick, JOYSTICK_Y);

    int direction_change;

    if (y < -65 && (x < 40 && x > -40)) // up
    {
        direction_change = VOLUME;
    }
    else if (y > 65 && (x < 40 && x > -40)) // down
    {
        direction_change = OCTAVE;
    }
    else if (x > 65 && (y < 40 && y > -40)) // right
    {
        direction_change = WAVEFORM;
    }
    else if (x < -65 && (y < 40 && y > -40)) // left
    {
        direction_change = DISTORTION;
    }
    else
    {
        direction_change = REST;
    }

    pthread_mutex_lock(&control_mutex);
    {
        current_control = direction_change;
    }
    pthread_mutex_unlock(&control_mutex);
}

static void print_stats()
{
    if (volume != last_volume || octave != last_octave || waveform != last_waveform ||
        distortion != last_distortion || mute != last_mute)
    {

        printf("\r\033[KVolume: %d | Octave: %d | Waveform: %d | Distortion: %f | Mute: %s",
               volume, octave, waveform, distortion, mute ? "ON" : "OFF");
        fflush(stdout);

        last_volume = volume;
        last_octave = octave;
        last_waveform = waveform;
        last_distortion = distortion;
        last_mute = mute;
    }
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
            bool is_muted;
            pthread_mutex_lock(&control_mutex);
            {
                is_muted = mute;
            }
            pthread_mutex_unlock(&control_mutex);

            if (is_muted)
            {
                volume = 0;
                print_stats();
                sleep_for_ms(10);
                continue;
            }
            int new_vol = rotary_encoder_get_value(&encoder);

            if(new_vol > 100)
            {
                new_vol = 100;
                rotary_encoder_set_value(new_vol);
            }

            if (new_vol != volume)
            {
                volume = new_vol;
                sleep_for_ms(10);
            }
            print_stats();
        }
    }

    else if (current_state == OCTAVE)
    {
        rotary_encoder_set_value(octave);
        while (current_control == OCTAVE)
        {
            int new_octave = rotary_encoder_get_value(&encoder);

            if (new_octave != octave)
            {
                octave = new_octave;
                sleep_for_ms(10);
            }
            print_stats();
        }
    }

    else if (current_state == WAVEFORM)
    {
        rotary_encoder_set_value(waveform);
        while (current_control == WAVEFORM)
        {
            int new_waveform = rotary_encoder_get_value(&encoder);

            new_waveform = (new_waveform % SINEMIXER_WAVE_COUNT);

            if (new_waveform != waveform)
            {
                waveform = new_waveform;
                sleep_for_ms(10);
            }
            print_stats();
        }
    }

    else if (current_state == DISTORTION)
    {
        int distortion_int = (int)(distortion * 100);
        rotary_encoder_set_value(distortion_int);

        while (current_control == DISTORTION)
        {
            int new_distortion_int = rotary_encoder_get_value(&encoder);

            if (new_distortion_int > 10)
            {
                new_distortion_int = 10;
                rotary_encoder_set_value(new_distortion_int);
            }
            
            double new_distortion_scaled = new_distortion_int / 100.0;

            distortion = new_distortion_scaled;
            sleep_for_ms(10);
            print_stats();
        }
    }

    print_stats();
    sleep_for_ms(50);
}

static void *control_thread_func(void *arg)
{
    (void)arg;
    while (!exit_thread)
    {
        set_direction();
    }
    return NULL;
}

static void *value_thread_func(void *arg)
{
    (void)arg;
    while (!exit_thread)
    {
        set_value();
    }
    return NULL;
}
