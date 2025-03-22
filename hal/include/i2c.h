/*
 * This module has helper functions for I2C, including
 * starting the I2C bus and reading/writing 16-bit registers.
 * Functions are from the CMPT433 course I2C Guide.
 */

#ifndef _I2C_H_
#define _I2C_H_

#include "utils.h"
#include <linux/i2c-dev.h>
#include <sys/ioctl.h>
#include <linux/i2c.h>
#include <stdbool.h>
#include <stdlib.h>
#include <unistd.h>
#include <assert.h>
#include <stdint.h>
#include <fcntl.h>
#include <stdio.h>

/**
 * Initializes the I2C bus for communication with a specific device.
 * 
 * @param bus The path to the I2C bus.
 * @param address The address of the I2C device.
 * @return The file descriptor for the I2C device, or -1 on failure.
 */
int init_i2c_bus(char *bus, int address);


/**
 * Writes a 16-bit value to a specified register of an I2C device.
 * 
 * @param i2c_file_desc The file descriptor for the I2C device.
 * @param reg_addr The register address to write to.
 * @param value The 16-bit value to write.
 */
void write_i2c_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value);


/**
 * Reads a 16-bit value from the specified register of an I2C device.
 * 
 * @param i2c_file_desc The file descriptor for the I2C device.
 * @param reg_addr The register address to read from.
 * @return The 16-bit value read from the register.
 */
uint16_t read_i2c_reg16(int i2c_file_desc, uint8_t reg_addr);

#endif
 