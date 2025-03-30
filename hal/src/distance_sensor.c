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


//# BACKUP

// #include "distance_sensor.h"

// #include <stdio.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <fcntl.h>
// #include <stdbool.h>
// #include <time.h>
// #include <limits.h>

// #define ECHO_GPIO_PIN "4"
// #define TRIGGER_GPIO_PIN "6"
// #define HIGH '1'
// #define LOW '0'

// #define SPEED_OF_SOUND 343.0
// #define MAX_TIME 13000000
// #define UNIT_SCALE 10000000.0 

// static int trigger_fd = -1;
// static int echo_fd = -1;

// static uint64_t get_time_ns(void) {
//     struct timespec ts;
//     clock_gettime(CLOCK_MONOTONIC, &ts);
//     return (uint64_t)ts.tv_sec * 1e9 + ts.tv_nsec;
// }


// int distancedistance_sensor_init_init(void) {

//     char trigger_path[64];
//     snprintf(trigger_path, sizeof(trigger_path), "/sys/class/gpio/gpio%s/value", TRIGGER_GPIO_PIN);
//     trigger_fd = open(trigger_path, O_RDWR);
//     if (trigger_fd == -1) {
//         perror("Failed to open trigger");
//         return -1;
//     }

//     char echo_path[64];
//     snprintf(echo_path, sizeof(echo_path), "/sys/class/gpio/gpio%s/value", ECHO_GPIO_PIN);
//     echo_fd = open(echo_path, O_RDONLY);
//     if (echo_fd == -1) {
//         perror("Failed to open echo");
//         close(trigger_fd);
//         return -1;
//     }

//     return 0;
// }

// void distance_sensor_cleanup(void) {
//     if (trigger_fd != -1) {
//         close(trigger_fd);
//         trigger_fd = -1;
//     }

//     if (echo_fd != -1) {
//         close(echo_fd);
//         echo_fd = -1;
//     }
// }

// static bool read_echo(void) {
//     char echo_value;
//     if (read(echo_fd, &echo_value, 1) == -1) {
//         perror("Failed to read echo file");
//         exit(EXIT_FAILURE);
//     }

//     lseek(echo_fd, 0, SEEK_SET);

//     return echo_value == HIGH;
// }


// double get_distance(void) {

//     char high_value = HIGH;
//     if (write(trigger_fd, &high_value, 1) == -1) {
//         perror("Failed to write HIGH to trigger");
//         exit(EXIT_FAILURE);
//     }
//     usleep(10);

    // char low_value = LOW;
    // if (write(trigger_fd, &low_value, 1) == -1) {
    //     perror("Failed to write LOW to trigger");
    //     exit(EXIT_FAILURE);
    // }

//     uint64_t real_start = get_time_ns();
//     uint64_t start = real_start, end = real_start;

//     while (!read_echo()) {
//         start = get_time_ns();
//         if (start - real_start > MAX_TIME) {
//             return -1;
//         }
//     }

//     while (read_echo()) {
//         end = get_time_ns();
//         if (end - real_start > MAX_TIME) {
//             return -1;
//         }
//     }

//     uint64_t diff = end - start;
//     return (diff * SPEED_OF_SOUND) / UNIT_SCALE / 2.0;
// }
