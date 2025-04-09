#include "distance_articulator.h"
#include "sine_mixer.h"
#include "distance_sensor.h"
#include "utils.h"

#include <stdbool.h>
#include <assert.h>
#include <pthread.h>
#include <math.h>

#define DEFAULT_VOLUME 80;

static int maxVolume = DEFAULT_VOLUME;

static bool muted = false;

static bool isInitialized = false;
static pthread_t articulatorRunner;
static void* articulatorRunnerFn(void* args);

void distance_articulator_init(void)
{   
    assert(!isInitialized);
    isInitialized = true;
    pthread_create(&articulatorRunner,NULL,articulatorRunnerFn,NULL);
}

void distance_articulator_cleanup(void)
{
    assert(isInitialized)
    isInitialized = false;
    pthread_join(articulatorRunner,NULL);
}

void distance_articulator_set_volume(int volume)
{
    maxVolume = volume;
}

void distance_acticulator_get_volume(void)
{
    return maxVolume;
}

#define MIN_DISTANCE 1
#define MAX_DISTANCE 15
#define STEEPNESS 0.05

static int dist_to_vol(int distance) 
{
    if(distance < MIN_DISTANCE) {
        return 100;
    } 
    if(distance > MAX_DISTANCE) {
        return 0;
    }
    double normalized_distance = (distance - MIN_DISTANCE) / (MAX_DISTANCE - MIN_DISTANCE);
    double volume = maxVolume*exp(-100*STEEPNESS*normalized_distance);
    return (int)volume;
}

static void* articulatorRunnerFn(void* args)
{
    while(isInitialized) {
        if(muted) {
            SineMixer_setVolume(0);
        } else {
            int vol = dist_to_vol(get_distance());
            SineMixer_setVolume(vol);
        }
        sleep_for_ms(5);
    }
}