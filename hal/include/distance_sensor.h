#ifndef _DISTANCE_SENSOR_H_
#define _DISTANCE_SENSOR_H_

#include <stdio.h>
#include <stdlib.h>

void distance_sensor_init();

int get_distance();

void distance_sensor_cleanup();

#endif


// Backup

// #ifndef _DISTANCE_SENSOR_H_
// #define _DISTANCE_SENSOR_H_

// #include <stdint.h>
// #include <stdbool.h>


// int distance_sensor_init(void);

// void distance_sensor_cleanup(void);

// double get_distance(void);

// #endif



