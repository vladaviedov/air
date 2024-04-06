/**
 * @file include/spi.hpp
 * @brief Drivers for SPI
 */
#pragma once

#include <cstdint>
#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <linux/types.h>
#include <string>
#include <sys/ioctl.h>
#include <unistd.h>

class spi {
public:
	/**
	 * @brief Constructor. Initializes I2C device.
	 *
	 * @param[in] mode - SPI mode
	 * @param[in] bpw - SPI bits per word
	 * @param[in] speed - SPI speed
	 * @param[in] adapter - The number of the I2C adapter.
	 */
	spi(uint8_t mode, uint8_t bpw, uint32_t speed, char *adapter);

	~spi();

	int transfer(const uint8_t *write_buf,
		const uint8_t *read_buf,
		uint32_t buf_len) const;

	/*
	 * Read data from a register of a SPI device.
	 *
	 * @param dev points to the SPI device to be read from
	 * @param reg the register to read from
	 * @param buf points to the start of buffer to be read into
	 * @param buf_len length of the buffer to be read
	 *
	 * @return - 0 if the read procedure succeeded
	 *         - negative if the read procedure failed
	 */
	int read(uint8_t reg, uint8_t *buf, int buf_len) const;

	int write(uint8_t reg, uint8_t *buf, int buf_len) const;

	uint8_t read_byte(uint8_t reg);

	int write_byte(uint8_t reg, uint8_t value);

private:
	int fd;
	uint8_t bpw;
	uint32_t speed;
};
