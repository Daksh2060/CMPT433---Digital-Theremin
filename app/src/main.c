/*
 * The main function of the Digital Theremin project.
 * It initializes the program manager, waits for the program to end,
 * and then cleans up the resources before exiting.
 */

#include "program_manager.h"
#include <stdbool.h>
#include <stdio.h> 

int main() 
{
    program_manager_init();

    program_wait_to_end();

    program_manager_cleanup();
    
    return 0;
}