#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <pthread.h>
#include <gpiod.h>
#include <stdbool.h>

#define GPIOCHIP1 "/dev/gpiochip1"
#define GPIOCHIP2 "/dev/gpiochip2"
#define TRIGGER_PIN 17
#define ECHO_PIN 38

#define TIMEOUT_US 30000
#define MAX_DISTANCE 400

static const char HIGH = 1;
static const char LOW = 0;

pthread_t sensor_pulse_thread;
pthread_t sensor_read_thread;

pthread_mutex_t sensor_mutex = PTHREAD_MUTEX_INITIALIZER;
int current_distance = 0;

bool pulse_thread_running = true;
bool read_thread_running = true;

struct gpiod_chip *chip1, *chip2;
struct gpiod_line *trigger, *echo;

static void *read_loop(void *arg);
static int get_distance_cm(struct gpiod_line *echo);

void distance_sensor_init() 
{
    sleep(1);
    read_thread_running = true;
    
    if (pthread_create(&sensor_read_thread, NULL, read_loop, NULL) != 0) {
        perror("Error creating read thread");
        exit(EXIT_FAILURE);
    }
    usleep(500000);
}

int get_distance() 
{
    int distance;
    pthread_mutex_lock(&sensor_mutex);
    distance = current_distance;
    pthread_mutex_unlock(&sensor_mutex);
    return distance;
}

void distance_sensor_cleanup() 
{
    read_thread_running = false;
    pthread_join(sensor_read_thread, NULL);
    pthread_mutex_destroy(&sensor_mutex);
}

static int get_distance_cm(struct gpiod_line *echo) 
{
    struct timespec start_time, end_time, current_time;
    long long timeout_ns;
    int distance_in_cm = 0;
    
    gpiod_line_set_value(trigger, HIGH);
    usleep(10);
    gpiod_line_set_value(trigger, LOW);
    
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    timeout_ns = start_time.tv_sec * 1000000000LL + start_time.tv_nsec + (TIMEOUT_US * 1000);
    
    while (gpiod_line_get_value(echo) == LOW) {
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        if ((current_time.tv_sec * 1000000000LL + current_time.tv_nsec) > timeout_ns) {
            return -1;
        }
        usleep(5);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &start_time);
    
    timeout_ns = start_time.tv_sec * 1000000000LL + start_time.tv_nsec + (TIMEOUT_US * 1000);
    
    while (gpiod_line_get_value(echo) == HIGH) {
        clock_gettime(CLOCK_MONOTONIC, &current_time);
        if ((current_time.tv_sec * 1000000000LL + current_time.tv_nsec) > timeout_ns) {
            return -1;
        }
        usleep(5);
    }
    
    clock_gettime(CLOCK_MONOTONIC, &end_time);
    
    long long start_ns = start_time.tv_sec * 1000000000LL + start_time.tv_nsec;
    long long end_ns = end_time.tv_sec * 1000000000LL + end_time.tv_nsec;
    long long duration_ns = end_ns - start_ns;
    long long duration_us = duration_ns / 1000;
    
    distance_in_cm = (int)(duration_us * 0.01715);
    
    if (distance_in_cm > MAX_DISTANCE || distance_in_cm < 0) {
        return -1;
    }
    
    return distance_in_cm;
}


static void *read_loop(void *arg) 
{
    (void)arg; 
    int distance_value;
    
    chip1 = gpiod_chip_open(GPIOCHIP1);
    if (!chip1) {
        perror("Failed to open gpiochip1");
        exit(EXIT_FAILURE);
    }
    
    echo = gpiod_chip_get_line(chip1, ECHO_PIN);
    if (!echo) {
        perror("Failed to get Echo line");
        exit(EXIT_FAILURE);
    }
    
    if (gpiod_line_request_input(echo, "Echo") < 0) {
        perror("Failed to request Echo pin");
        exit(EXIT_FAILURE);
    }
    
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
    
    if (gpiod_line_request_output(trigger, "Trigger", LOW) < 0) {
        perror("Failed to request Trigger pin");
        exit(EXIT_FAILURE);
    }
    
    usleep(100000);
    
    while (read_thread_running) {
        distance_value = get_distance_cm(echo);
        
        if (distance_value >= 0) {
            pthread_mutex_lock(&sensor_mutex);
            current_distance = distance_value;
            pthread_mutex_unlock(&sensor_mutex);
        }
        
        usleep(60000);
    }
    
    gpiod_line_release(echo);
    gpiod_chip_close(chip1);
    gpiod_line_release(trigger);
    gpiod_chip_close(chip2);
    return NULL;
}
