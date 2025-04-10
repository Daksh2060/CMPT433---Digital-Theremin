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

#define MIN_DISTANCE 3    // Minimum distance for volume calculation
#define MAX_DISTANCE 50   // Maximum distance for volume calculation
#define STEEPNESS 0.01    // Steepness of the volume curve

// The maximum volume level for the distance articulator
static int maxVolume = DEFAULT_VOLUME;

// Flag to indicate if the theremin is muted
static bool muted = false;

// Buffer to store distance samples for averaging
static int distanceSamples[NUM_SAMPLES];
static int totalSamples = 0;

// Thread control variables
static bool isInitialized = false;
static pthread_t articulatorRunner;

// Helper function prototypes
static int dist_to_vol(int distance);
static int averageSample(void);
static void* articulatorRunnerFn(void* args);

void distance_articulator_init(void)
{   
    assert(!isInitialized);
    isInitialized = true;
    pthread_create(&articulatorRunner,NULL,articulatorRunnerFn,NULL);
}

void distance_articulator_cleanup(void)
{
    assert(isInitialized);
    isInitialized = false;
    pthread_join(articulatorRunner,NULL);
}

void distance_articulator_set_volume(int volume)
{
    maxVolume = volume;
}

int distance_acticulator_get_volume(void)
{
    return maxVolume;
}

void distance_articulator_set_mute(bool isMuted)
{
    muted = isMuted;
}

// Function to set the volume based on distance using linear scaling.
static int dist_to_vol(int distance) 
{
    if(distance < MIN_DISTANCE) {
        return maxVolume;
    } 
    if(distance > MAX_DISTANCE) {
        return 0;
    }
    double normalized_distance = (distance - MIN_DISTANCE) / (double)(MAX_DISTANCE - MIN_DISTANCE);
    double volume = maxVolume*(1-normalized_distance);
    return (int)volume;
}

// Function to calculate the rolling average of the distance samples.
static int averageSample(void)
{
    int sum = 0;
    for(int i = 0; i < NUM_SAMPLES; i++) {
        sum += distanceSamples[i];
    }
    return sum/NUM_SAMPLES;
}

// Thread function that continuously reads the distance sensor and updates the volume.
static void* articulatorRunnerFn(void* args)
{
    (void)args;
    while(isInitialized) {
        maxVolume = get_volume();
        int sample = get_distance();
    
        if(sample <= MAX_DISTANCE) {
            distanceSamples[totalSamples%NUM_SAMPLES] = sample;
            totalSamples++;
        }
        if(muted) {
            SineMixer_setVolume(0);
        } 
        else {
            int distance = averageSample();
            int vol = dist_to_vol(distance);
            SineMixer_setVolume(vol);
        }
        sleep_for_ms(5);
    }
    return NULL;
}