/**
 * @file src/i2c.cpp
 * @brief I2C driver.
 */
#include "i2c.hpp"

#include <cstdio>
#include <fcntl.h>
#include <linux/i2c-dev.h>
#include <stdexcept>
#include <sys/ioctl.h>
#include <unistd.h>

#include <i2c/smbus.h>

i2c::i2c(uint8_t addr, int adapter_nr)
	: dev_addr(addr) {
	char filename[32];

	snprintf(filename, 32, "/dev/i2c-%d", adapter_nr);
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		throw std::runtime_error("Failed to open i2c port");
	}

	if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
		throw std::runtime_error("Failed to open i2c port (ioctl)");
	}
}

i2c::~i2c() {
	close(fd);
}

int i2c::read(uint8_t reg, uint8_t *data) const {
	return i2c_smbus_read_block_data(fd, reg, data);
}

bool i2c::write(uint8_t reg, const uint8_t *data, uint8_t size) const {
	return i2c_smbus_write_block_data(fd, reg, size, data) == 0;
}
