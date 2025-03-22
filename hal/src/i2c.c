/*
 * This file implements I2C communication functions for interacting with 
 * I2C devices, including initialization, reading, and writing data to 
 * specific registers.
 */

#include "i2c.h"

int init_i2c_bus(char* bus, int address)
{
	// Open the I2C bus for read/write operations
	int i2c_file_desc = open(bus, O_RDWR);
	if (i2c_file_desc == -1) {
		printf("I2C DRV: Unable to open bus for read/write (%s)\n", bus);
		perror("Error is:");
		exit(EXIT_FAILURE);
	}

	// Set the I2C device to the specified slave address
	if (ioctl(i2c_file_desc, I2C_SLAVE, address) == -1) {
		perror("Unable to set I2C device to slave address.");
		exit(EXIT_FAILURE);
	}
	return i2c_file_desc;
}

void write_i2c_reg16(int i2c_file_desc, uint8_t reg_addr, uint16_t value)
{
	int tx_size = 1 + sizeof(value);
	uint8_t buff[tx_size];

	// Prepare data buffer to write: register address + 16-bit value (little-endian)
	buff[0] = reg_addr;
	buff[1] = (value & 0xFF);
	buff[2] = (value & 0xFF00) >> 8;

	// Write data to the I2C register
	int bytes_written = write(i2c_file_desc, buff, tx_size);
	if (bytes_written != tx_size) {
		perror("Unable to write i2c register");
		exit(EXIT_FAILURE);
	}
}

uint16_t read_i2c_reg16(int i2c_file_desc, uint8_t reg_addr)
{
	// Write the register address to the I2C bus
	int bytes_written = write(i2c_file_desc, &reg_addr, sizeof(reg_addr));
	if (bytes_written != sizeof(reg_addr)) {
		perror("Unable to write i2c register.");
		exit(EXIT_FAILURE);
	}

	uint16_t value = 0;

	// Read the 16-bit value from the I2C register
	int bytes_read = read(i2c_file_desc, &value, sizeof(value));
	if (bytes_read != sizeof(value)) {
		perror("Unable to read i2c register");
		exit(EXIT_FAILURE);
	}
	return value;
}
 