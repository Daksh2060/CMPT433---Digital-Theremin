#include "volume_controls.h"
#include "audio_mixer.h"
#include "utils.h"
#include <pthread.h>
#include <stdbool.h>

#define VOLUME_MAX 100     // Maximum volume value
#define VOLUME_MIN 0       // Minimum volume value
#define VOLUME_INCREMENT 5 // Volume increment value

// Thread control and mutex variables
static pthread_t controller_thread;
static pthread_mutex_t controller_mutex = PTHREAD_MUTEX_INITIALIZER;

// Current volume and screen values
static int current_volume = 80;


int get_volume()
{
    int volume;
    pthread_mutex_lock(&controller_mutex);
    {
        volume = current_volume;
    }
    pthread_mutex_unlock(&controller_mutex);
    return volume;
}


void increase_volume()
{
    pthread_mutex_lock(&controller_mutex);
    {
        if (current_volume < VOLUME_MAX){
            current_volume += VOLUME_INCREMENT;
            set_volume();
        }
    }
    pthread_mutex_unlock(&controller_mutex);
}


void decrease_volume()
{
    pthread_mutex_lock(&controller_mutex);
    {
        if (current_volume > VOLUME_MIN){
            current_volume -= VOLUME_INCREMENT;
            set_volume();
        }
    }
    pthread_mutex_unlock(&controller_mutex);
}


void set_volume()
{
    audio_mixer_set_volume(current_volume);
}
 