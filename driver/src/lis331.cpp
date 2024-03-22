/**
 * @file src/lis331.cpp
 * @brief LIS331 driver.
 */

#include "lis331.hpp"

#include <stdexcept>

lis331::lis331(i2c *i2c)
	: i2cd(i2c) {
	/* Set power to normal mode */
	uint8_t config[1] = {0b00100111};
	i2cd->write(CTRL_REG1, config, 1);

	/* Check to see if I2C is working */
	uint8_t data[1];
	i2cd->read(WHO_AM_I, data);
	if (data[0] != 0b00110010) {
		throw std::runtime_error("Failed to initialize LIS331");
    }
}

lis331::~lis331() {}

bool lis331::new_data() const {
	uint8_t data[1];
	/* DEBUGGING: If this doesn't work (maybe the lib takes care of it), write,
	 * then read according to the data sheet */
	i2cd->read(STATUS_REG, data);
	// DEBUGGING: Endianness may be reversed.
	return (0b00010000 & data[0]) > 0;
}

int lis331::read_x(uint8_t *data) const {
	i2cd->read(OUT_X_L, data);
	return i2cd->read(OUT_X_H, data + 1);
}

int lis331::read_y(uint8_t *data) const {
	i2cd->read(OUT_Y_L, data);
	return i2cd->read(OUT_Y_H, data + 1);
}

int lis331::read_z(uint8_t *data) const {
	i2cd->read(OUT_Z_L, data);
	return i2cd->read(OUT_Z_H, data + 1);
}
