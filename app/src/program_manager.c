
#include "program_manager.h"
#include "button_controls.h"
#include "hand_commands.h"
#include "dial_controls.h"
#include "udp_controls.h"
#include "utils.h"
#include "sine_mixer.h"
#include "gpio.h"
#include "lcd_menus.h"
#include "distance_sensor.h"
#include "distance_articulator.h"
#include <stdio.h>

volatile bool exit_theremin_program = false;

void program_manager_init(void)
{
    // SineMixer_init();
    // lcd_menu_init();
    // udp_init();
    // distance_sensor_init();
    // distance_articulator_init();
    // command_handler_init();
    dial_controls_init();
    button_controls_init();
}

void program_wait_to_end()
{
    while (exit_theremin_program == false) {
        sleep_for_ms(111);
    }
}

void program_manager_cleanup(void)
{
    button_controls_cleanup();
    dial_controls_cleanup();
    // gpio_cleanup();
    // udp_cleanup();
    // distance_articulator_cleanup();
    // distance_sensor_cleanup();
    // command_handler_cleanup();
    // lcd_menu_cleanup();
    // SineMixer_cleanup();
}