#include "program_manager.h"
#include "utils.h"
#include <stdbool.h>
#include "distance_sensor.h"
#include <stdio.h> 

volatile bool exit_theremin_program = false;

#include <gpiod.h>
#include <pthread.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

// int main()
// {
//     distance_sensor_init();
//     while(1){

//         int distance = get_distance();
//         printf("Distance: %d cm\r", distance);
//         fflush(stdout);
//         sleep_for_ms(10);
//     }
//     return 0;
// }



#define GPIOCHIP1 "/dev/gpiochip1"
#define GPIOCHIP2 "/dev/gpiochip2"
#define TRIGGER_PIN 17
#define ECHO_PIN 38

pthread_t sensor_pulse_thread;

void *pulse_loop(void *arg) {
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

    while (1) {
        gpiod_line_set_value(trigger, 1);
        printf("Trigger HIGH\n"); 
        usleep(300000); 
        gpiod_line_set_value(trigger, 0);
        printf("Trigger LOW\n");  
        usleep(300000);  
    }  
    
    return NULL;
}



int main() {

    if (pthread_create(&sensor_pulse_thread, NULL, pulse_loop, NULL) != 0) {
        perror("Error creating pulse thread");
        exit(EXIT_FAILURE);
    }

    struct gpiod_chip *chip1;
    struct gpiod_line *echo;

    chip1 = gpiod_chip_open(GPIOCHIP1);
    if (!chip1) {
        perror("Failed to open gpiochip1");
        exit(EXIT_FAILURE);
    }

    long double start_time = 0;
    long double end_time = 0;
    long double length_of_time = 0;
    long double distance_in_cm = 0;

    while (1) {
        
        echo = gpiod_chip_get_line(chip1, ECHO_PIN);
        if (!echo) {
            perror("Failed to get Echo line");
            exit(EXIT_FAILURE);
        }

        if (gpiod_line_request_input(echo, "Echo") < 0) {
            perror("Failed to request Echo pin");
            exit(EXIT_FAILURE);
        }

        while (gpiod_line_get_value(echo) == 0) {
            start_time = (long double)clock();
        }
    
        while (gpiod_line_get_value(echo) == 1) {
            end_time = (long double)clock();
        }

        length_of_time = end_time - start_time;
        distance_in_cm = length_of_time * 0.000017150;

        if (distance_in_cm <= 0) {
            distance_in_cm = 0;
        }

        printf("Distance: %.2Lf cm\n", distance_in_cm);
    }

    return 0;
}