#include <gpiod.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define GPIOCHIP1 "/dev/gpiochip1"  // GPIO chip for Echo
#define GPIOCHIP2 "/dev/gpiochip2"  // GPIO chip for Trigger
#define TRIGGER_PIN 17
#define ECHO_PIN 38

pthread_t sensor_pulse_thread;
pthread_t sensor_read_thread;

pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;
int current_distance = 0;

bool pulse_thread_running = true;
bool read_thread_running = true;

// Function to calculate the distance based on time difference
void get_distance_cm(struct gpiod_line *echo) {
    long double start_time = 0;
    long double end_time = 0;
    long double length_of_time = 0;
    long double distance_in_cm = 0;

    // Wait for Echo pin to go HIGH
    while (gpiod_line_get_value(echo) == 0) {
        printf("Waiting for Echo HIGH\n");  // Debug
        start_time = (long double)clock();
    }

    // Echo pin is HIGH, start the timer
    printf("Echo HIGH\n");  // Debug
    while (gpiod_line_get_value(echo) == 1) {
        end_time = (long double)clock();
    }

    // Calculate time difference
    length_of_time = end_time - start_time;
    distance_in_cm = length_of_time * 0.000017150;

    if (distance_in_cm <= 0) {
        distance_in_cm = 0;
    }

    current_distance = (int)distance_in_cm;
}


static void *pulse_loop(void *arg) {
    (void)arg;
    struct gpiod_chip *chip2;
    struct gpiod_line *trigger;

    chip2 = gpiod_chip_open(GPIOCHIP2);
    if (!chip2) {
        perror("Failed to open gpiochip2");
        exit(EXIT_FAILURE);
    }

    trigger = gpiod_chip_get_line(chip2, TRIGGER_PIN);
    if (!trigger) {
        perror("Failed to get Trigger line");
        exit(EXIT_FAILURE);
    }

    if (gpiod_line_request_output(trigger, "Trigger", 0) < 0) {
        perror("Failed to request Trigger pin");
        exit(EXIT_FAILURE);
    }

    while (pulse_thread_running) {
        gpiod_line_set_value(trigger, 1);
        printf("Trigger HIGH\n"); 
        usleep(300000); 
        gpiod_line_set_value(trigger, 0);
        printf("Trigger LOW\n");  
        usleep(300000);  
    }  
    
    gpiod_line_release(trigger);
    gpiod_chip_close(chip2);
    return NULL;
}


void distance_sensor_init() {
    sleep(1);

    pulse_thread_running = true;
    read_thread_running = true;

    if (pthread_create(&sensor_pulse_thread, NULL, pulse_loop, NULL) != 0) {
        perror("Error creating pulse thread");
        exit(EXIT_FAILURE);
    }
}


int get_distance() {
    int distance;
    pthread_mutex_lock(&sensor_mutex);
    {
        distance = current_distance;
    }
    pthread_mutex_unlock(&sensor_mutex);
    return distance;
}


void distance_sensor_cleanup() {
    pulse_thread_running = false;
    read_thread_running = false;

    pthread_join(sensor_pulse_thread, NULL);
    pthread_join(sensor_read_thread, NULL);

    pthread_mutex_destroy(&sensor_mutex);
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
