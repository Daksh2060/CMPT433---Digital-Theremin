
#include "button_controls.h"
#include "hand_commands.h"
#include "knob_controls.h"
#include "udp_controls.h"
#include "utils.h"
#include "program_manager.h"
#include "sine_mixer.h"
#include "gpio.h"
#include <stdio.h>


// Audio mixer not included
void program_manager_init(void)
{
    udp_init();
    command_handler_init();
    knob_controls_init();
    button_controls_init();
}

void program_manager_cleanup(void)
{
    button_controls_cleanup();
    printf("Button controls cleaned up\n");
    knob_controls_cleanup();
    printf("Knob controls cleaned up\n");
    gpio_cleanup();
    printf("GPIO cleaned up\n");
    udp_cleanup();
    printf("UDP cleaned up\n");
    command_handler_cleanup();
    printf("Command handler cleaned up\n");
}