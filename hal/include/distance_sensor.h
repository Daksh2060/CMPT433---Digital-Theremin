/*
 * This module is used to interface with a Distance Sensor.
 * It allows the creation of a pthread which actively reads
 * and adjusts the value of the distance sensor based on the location
 * of the directly object in front of it.
 */


#ifndef _DISTANCE_SENSOR_H_
#define _DISTANCE_SENSOR_H_

#include <stdlib.h>
#include <stdio.h>


/*
 * This function initializes the distance sensor module and starts the read thread.
 */
void distance_sensor_init();


/**
 * This function retrieves the current distance value from the sensor.
 * 
 * @return The current distance in cm.
 */
int get_distance();


/*
 * This function cleans up the distance sensor module and stops the read thread.
 */
void distance_sensor_cleanup();

#endif



