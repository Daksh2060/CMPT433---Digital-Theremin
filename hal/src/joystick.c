/*
 * This file implements the joystick module for the BeagleBone.
 */

#include "joystick.h"
#include "utils.h"
#include "i2c.h"
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdio.h>
#include <fcntl.h>

#define I2CDRV_LINUX_BUS "/dev/i2c-1"   // The I2C bus to communicate with the joystick.
#define I2C_DEVICE_ADDRESS 0x48         // The I2C address of the joystick device.

#define REG_CONFIGURATION 0x01          // Register address for configuration.
#define REG_DATA 0x00                   // Register address for data.

#define TLA2024_CHANNEL_CONF_X 0x83D2   // Configuration value for X-axis reading.
#define TLA2024_CHANNEL_CONF_Y 0x83C2   // Configuration value for Y-axis reading.

// // Joystick scaling values
#define JOYSTICK_X_MIN 1
#define JOYSTICK_X_MAX 1630
#define JOYSTICK_Y_MIN 23
#define JOYSTICK_Y_MAX 1647
#define JOYSTICK_SCALE_RANGE 200
#define JOYSTICK_OFFSET 100

// Flag to track if joystick module has been initialized.
static bool is_initialized = false;

void joystick_init(Joystick *joystick) 
{
    assert(joystick != NULL);
    assert(!joystick->is_initialized);
    assert(!is_initialized);

    joystick->i2c_file_desc = init_i2c_bus(I2CDRV_LINUX_BUS, I2C_DEVICE_ADDRESS);
    if (joystick->i2c_file_desc < 0) {
        perror("Failed to initialize I2C");
        exit(EXIT_FAILURE);
    }
    joystick->is_initialized = true;
    is_initialized = true;
}

int joystick_read_input(Joystick *joystick, JoystickDirection dir) 
{
    assert(joystick != NULL);
    assert(joystick->is_initialized);
    assert(dir == JOYSTICK_X || dir == JOYSTICK_Y);
    assert(is_initialized);

    uint16_t config;
    if (dir == JOYSTICK_X) {
        config = TLA2024_CHANNEL_CONF_X;
    } 
    else {
        config = TLA2024_CHANNEL_CONF_Y;
    }
    write_i2c_reg16(joystick->i2c_file_desc, REG_CONFIGURATION, config);
    sleep_for_ms(5);

    // Read the raw value from the joystick, obtained from tla2024_demo.c
    uint16_t raw_read = read_i2c_reg16(joystick->i2c_file_desc, REG_DATA);
    uint16_t value = ((raw_read  & 0xFF) << 8) | ((raw_read  & 0xFF00) >> 8);
    value = value >> 4;
    
    // Scales value to [-100, 100] based on observed min and max values for usability.
    if (dir == JOYSTICK_X) {
        return ((value - JOYSTICK_X_MIN) * JOYSTICK_SCALE_RANGE) / 
               (JOYSTICK_X_MAX - JOYSTICK_X_MIN) - JOYSTICK_OFFSET;
    }
    else {
        return ((value - JOYSTICK_Y_MIN) * JOYSTICK_SCALE_RANGE) / 
               (JOYSTICK_Y_MAX - JOYSTICK_Y_MIN) - JOYSTICK_OFFSET;
    }
}

void joystick_cleanup(Joystick *joystick) 
{
    assert(joystick != NULL);
    assert(joystick->is_initialized);
    assert(is_initialized);

    close(joystick->i2c_file_desc);
    joystick->is_initialized = false;
    is_initialized = false;
}