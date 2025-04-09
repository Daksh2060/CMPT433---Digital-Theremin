#include "distance_articulator.h"
#include "sine_mixer.h"
#include "distance_sensor.h"
#include "utils.h"

#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <math.h>

#define DEFAULT_VOLUME 80

static int maxVolume = DEFAULT_VOLUME;

static bool muted = false;

static bool isInitialized = false;
static pthread_t articulatorRunner;
static void* articulatorRunnerFn(void* args);

#define NUM_SAMPLES 40
static int distanceSamples[NUM_SAMPLES];
static int totalSamples = 0;
static int averageSample(void);

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

#define MIN_DISTANCE 3
#define MAX_DISTANCE 20
#define STEEPNESS 0.01

static int dist_to_vol(int distance) 
{
    if(distance < MIN_DISTANCE) {
        return 100;
    } 
    if(distance > MAX_DISTANCE) {
        return 0;
    }
    double normalized_distance = (distance - MIN_DISTANCE) / (double)(MAX_DISTANCE - MIN_DISTANCE);
    //double volume = maxVolume*exp(-100*STEEPNESS*normalized_distance);
    double volume = maxVolume*(1-normalized_distance);
    return (int)volume;
}

static int averageSample(void)
{
    int sum = 0;
    for(int i = 0; i < NUM_SAMPLES; i++) {
        sum += distanceSamples[i];
    }
    return sum/NUM_SAMPLES;
}

static void* articulatorRunnerFn(void* args)
{
    (void)args;
    while(isInitialized) {
        int sample = get_distance();
        // throw away samples above 50cm
        if(sample <= MAX_DISTANCE) {
            distanceSamples[totalSamples%NUM_SAMPLES] = sample;
            totalSamples++;
        }
        if(muted) {
            SineMixer_setVolume(0);
        } else {
            int distance = averageSample();
            int vol = dist_to_vol(distance);
            // printf("%d\n", distance);
            printf("vol: %d\n", vol);
            SineMixer_setVolume(vol);
        }
        sleep_for_ms(5);
    }
    return NULL;
}