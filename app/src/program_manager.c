/*
 * This file implements the program manager module, handling the initialization,
 * cleanup, and running of the Digital Theremin. It manages the lifecycle of the program
 * after being called from the main function.
 */

#include "distance_articulator.h"
#include "program_manager.h"
#include "button_controls.h"
#include "distance_sensor.h"
#include "hand_commands.h"
#include "dial_controls.h"
#include "udp_controls.h"
#include "sine_mixer.h"
#include "lcd_menus.h"
#include "utils.h"
#include "gpio.h"
#include <stdio.h>

// Global variable to signal the end of the program
volatile bool exit_theremin_program = false;

void program_manager_init(void)
{
    sine_mixer_init();
    lcd_menu_init();
    udp_init();
    distance_sensor_init();
    distance_articulator_init();
    command_handler_init();
    dial_controls_init();
    button_controls_init();
}

void program_wait_to_end()
{
    while (exit_theremin_program == false){
        sleep_for_ms(111);
    }
}

void program_manager_cleanup(void)
{
    udp_cleanup();
    button_controls_cleanup();
    dial_controls_cleanup();
    gpio_cleanup();
    distance_articulator_cleanup();
    distance_sensor_cleanup();
    command_handler_cleanup();
    lcd_menu_cleanup();
    sine_mixer_cleanup();
}