#ifndef _DISTANCE_ARTICULATOR_
#define _DISTANCE_ARTICULATOR_

#include "sine_mixer.h"

#include <stdbool.h>

#define DISTANCE_ARTICULATOR_VOLUME_MAX SINEMIXER_VOLUME_MAX
#define DISTANCE_ARTICULATOR_VOLUME_MIN SINEMIXER_VOLUME_MIN

void distance_articulator_init(void);
void distance_articulator_cleanup(void);
void distance_articulator_set_volume(int volume);
int distance_acticulator_get_volume(void);
void distance_articulator_set_mute(bool isMuted);

#endif