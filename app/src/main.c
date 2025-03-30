#include "program_manager.h"
#include "utils.h"
#include <stdbool.h>
#include "distance_sensor.h"

volatile bool exit_theremin_program = false;

int main()
{
    distance_sensor_init();
    while(1){

        int distance = get_distance();
        printf("Distance: %d cm\r", distance);
        fflush(stdout);
        sleep_for_ms(10);
    }
    return 0;
}