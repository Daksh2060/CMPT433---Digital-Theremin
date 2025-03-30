#include "program_manager.h"
#include "utils.h"
#include <stdbool.h>
#include "distance_sensor.h"
#include <stdio.h> 

volatile bool exit_theremin_program = false;


int main() {

    distance_sensor_init();
    sleep(2); 

    while (1) {
        int distance = get_distance();
        if (distance != -1) {
            printf("Distance: %d cm\n", distance);
        } else {
            printf("Error reading distance\n");
        }
        usleep(50000); 
    }
    
    return 0;
}