#include "distance_sensor.h"
#include "gpio.h"
#include "utils.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <linux/i2c-dev.h>
#include <time.h>
#include <stdbool.h>
#include <pthread.h>

#define TRIGGER_DIRECTION "/sys/class/gpio/gpio6/direction"
#define TRIGGER_VALUE "/sys/class/gpio/gpio6/value"

#define ECHO_DIRECTION "/sys/class/gpio/gpio4/direction"
#define ECHO_ACTIVATE_LOW "/sys/class/gpio/gpio4/active_low"
#define ECHO_VALUE "/sys/class/gpio/gpio4/value"

#define A2D_FILE_VOLTAGE1 "/sys/bus/iio/devices/iio:device0/in_voltage1_raw"

static bool read_thread_running = false;
static bool pulse_thread_running = false;

static pthread_t sensor_pulse_thread;
static pthread_t sensor_read_thread;
static pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;

static int current_distance = 0;

static void *pulse_loop(void *arg);
static void *read_loop(void *arg);
static long double get_distance_cm();

void distance_sensor_init()
{
    sleep_for_ms(300);
    write_string_to_file(ECHO_DIRECTION, "in");
    sleep_for_ms(300);
    write_string_to_file(TRIGGER_DIRECTION, "out");
    sleep_for_ms(300);
    write_string_to_file(ECHO_ACTIVATE_LOW, "0");

    pulse_thread_running = true;
    read_thread_running = true;

    if (pthread_create(&sensor_pulse_thread, NULL, pulse_loop, NULL) != 0){
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }

    if (pthread_create(&sensor_read_thread, NULL, read_loop, NULL) != 0){
        perror("Error creating thread");
        exit(EXIT_FAILURE);
    }
}

int get_distance()
{
    int distance;
    pthread_mutex_lock(&sensor_mutex);
    {
        distance = current_distance;
    }
    pthread_mutex_unlock(&sensor_mutex);
    return distance;
}

void distance_sensor_cleanup()
{
    pulse_thread_running = false;
    read_thread_running = false;

    pthread_join(sensor_pulse_thread, NULL);
    pthread_join(sensor_read_thread, NULL);

    pthread_mutex_destroy(&sensor_mutex);
}


static void *pulse_loop(void *arg)
{
    (void)arg;
    while (pulse_thread_running){
        sleep_for_ms(300);
        write_int_to_file(TRIGGER_VALUE, 1);
        sleep_for_ms(300);
        write_int_to_file(TRIGGER_VALUE, 0);
    }
    return NULL;
}

static void *read_loop(void *arg)
{
    (void)arg;
    while (read_thread_running)
    {
        pthread_mutex_lock(&sensor_mutex);
        {
            current_distance = get_distance_cm();
        }
        pthread_mutex_unlock(&sensor_mutex);
        sleep_for_ms(50);
    }
    return NULL;
}

static long double get_distance_cm()
{
    long double start_time = 0;
    long double end_time = 0;
    long double length_of_time = 0;
    long double distance_in_cm = 0;

    while (get_data_from_file(ECHO_VALUE) == 6){
        start_time = get_time_in_ns();
    }
    while (get_data_from_file(ECHO_VALUE) == 4){
        end_time = get_time_in_ns();
    }

    length_of_time = end_time - start_time;
    distance_in_cm = length_of_time * 0.000017150;

    if (distance_in_cm <= 0){
        distance_in_cm = 0;
    }
    return distance_in_cm;
}
