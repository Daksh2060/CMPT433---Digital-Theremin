/*
 * This file implements the distance articulator module, which is responsible for
 * converting the distance sensor data into a volume level for the sine mixer.
 */

#include "distance_articulator.h"
#include "distance_sensor.h"
#include "dial_controls.h"
#include "sine_mixer.h"
#include "utils.h"
#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <math.h>

#define DEFAULT_VOLUME 80 // Default volume level for the distance articulator
#define NUM_SAMPLES 40    // Number of distance samples to average for smoothness

#define MIN_DISTANCE 3  // Minimum distance for volume calculation
#define MAX_DISTANCE 50 // Maximum distance for volume calculation
#define STEEPNESS 0.01  // Steepness of the volume curve

// The maximum volume level for the distance articulator
static int max_volume = DEFAULT_VOLUME;

// Flag to indicate if the theremin is muted
static bool muted = false;

// Buffer to store distance samples for averaging
static int distance_samples[NUM_SAMPLES];
static int total_samples = 0;

// Thread control variables
static bool is_initialized = false;
static pthread_t articulator_runner;

// Helper function prototypes
static int dist_to_vol(int distance);
static int averageSample(void);
static void *articulator_runnerFn(void *args);

void distance_articulator_init(void)
{
    assert(!is_initialized);
    is_initialized = true;
    pthread_create(&articulator_runner, NULL, articulator_runnerFn, NULL);
}

void distance_articulator_cleanup(void)
{
    assert(is_initialized);
    is_initialized = false;
    pthread_join(articulator_runner, NULL);
}

void distance_articulator_set_volume(int volume)
{
    max_volume = volume;
}

int distance_acticulator_get_volume(void)
{
    return max_volume;
}

void distance_articulator_set_mute(bool isMuted)
{
    muted = isMuted;
}

// Function to set the volume based on distance using linear scaling.
static int dist_to_vol(int distance)
{
    if (distance < MIN_DISTANCE){
        return max_volume;
    }
    if (distance > MAX_DISTANCE){
        return 0;
    }
    double normalized_distance = (distance - MIN_DISTANCE) / (double)(MAX_DISTANCE - MIN_DISTANCE);
    double volume = max_volume * (1 - normalized_distance);
    return (int)volume;
}

// Function to calculate the rolling average of the distance samples.
static int averageSample(void)
{
    int sum = 0;
    for (int i = 0; i < NUM_SAMPLES; i++){
        sum += distance_samples[i];
    }
    return sum / NUM_SAMPLES;
}

// Thread function that continuously reads the distance sensor and updates the volume.
static void *articulator_runnerFn(void *args)
{
    (void)args;
    while (is_initialized){
        max_volume = get_volume();
        int sample = get_distance();

        if (sample <= MAX_DISTANCE){
            distance_samples[total_samples % NUM_SAMPLES] = sample;
            total_samples++;
        }
        if (muted){
            sine_mixer_set_volume(0);
        }
        else{
            int distance = averageSample();
            int vol = dist_to_vol(distance);
            sine_mixer_set_volume(vol);
        }
        sleep_for_ms(5);
    }
    return NULL;
}