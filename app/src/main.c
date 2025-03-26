#include "program_manager.h"
#include "utils.h"
#include <stdbool.h>

volatile bool exit_theremin_program = false;

int main()
{
    program_manager_init();

    while (!exit_theremin_program)
    {
        sleep_for_ms(1);
    }
    
    program_manager_cleanup();

    return 0;
}