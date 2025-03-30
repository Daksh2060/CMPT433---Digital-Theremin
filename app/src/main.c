#include "program_manager.h"
#include "utils.h"
#include <stdbool.h>
#include "distance_sensor.h"

volatile bool exit_theremin_program = false;

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


#include <gpiod.h>
#include <unistd.h>
#include <stdio.h>

#define GPIOCHIP1 "/dev/gpiochip1"  // GPIO chip for Echo
#define GPIOCHIP2 "/dev/gpiochip2"  // GPIO chip for Trigger
#define TRIGGER_PIN 17
#define ECHO_PIN 38

int main() {
    struct gpiod_chip *chip1, *chip2;
    struct gpiod_line *trigger, *echo;
    
    chip1 = gpiod_chip_open(GPIOCHIP1);
    if (!chip1) {
        perror("Failed to open gpiochip1");
        return 1;
    }
    
    chip2 = gpiod_chip_open(GPIOCHIP2);
    if (!chip2) {
        perror("Failed to open gpiochip2");
        return 1;
    }
    
    trigger = gpiod_chip_get_line(chip2, TRIGGER_PIN);
    echo = gpiod_chip_get_line(chip1, ECHO_PIN);

    if (!trigger || !echo) {
        perror("Failed to get GPIO lines");
        return 1;
    }

    if (gpiod_line_request_output(trigger, "Trigger", 0) < 0) {
        perror("Failed to request Trigger pin");
        return 1;
    }

    if (gpiod_line_request_input(echo, "Echo") < 0) {
        perror("Failed to request Echo pin");
        return 1;
    }

    gpiod_line_set_value(trigger, 1);
    usleep(10); 
    gpiod_line_set_value(trigger, 0);

    
    while (gpiod_line_get_value(echo) == 0);
    printf("Echo HIGH\n");

  
    while (gpiod_line_get_value(echo) == 1);
    printf("Echo LOW\n");

    gpiod_line_release(trigger);
    gpiod_line_release(echo);
    gpiod_chip_close(chip1);
    gpiod_chip_close(chip2);

    return 0;
}
