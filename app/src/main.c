#include "program_manager.h"
#include "utils.h"
#include <stdbool.h>
#include "distance_sensor.h"
#include <stdio.h> 

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


int main() {

    distance_sensor_init();

    for (int i = 0; i < 10; i++) { 
        printf("Current Distance: %d cm\r", get_distance());
        sleep_for_ms(10);
        fflush(stdout);
    }

    distance_sensor_cleanup();

    return 0;
}