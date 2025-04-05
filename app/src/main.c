#include "program_manager.h"
#include "utils.h"
#include <stdbool.h>
#include "distance_sensor.h"
#include <stdio.h> 

int main() 
{
    program_manager_init();
    program_wait_to_end();
    program_manager_cleanup();
    
    return 0;
}