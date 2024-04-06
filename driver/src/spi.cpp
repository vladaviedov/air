/**
 * @file src/spi.cpp
 * @brief Drivers for SPI
 */

#include "spi.hpp"

#include <fcntl.h>
#include <linux/spi/spidev.h>
#include <stdexcept>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <unistd.h>

spi::spi(uint8_t mode, uint8_t bpw, uint32_t speed, char *adapter)
	: bpw(bpw),
	  speed(speed) {
	char filename[32];

	snprintf(filename, 32, "/dev/spidev%s", adapter);
	fd = open(filename, O_RDWR);
	if (fd < 0) {
		throw std::runtime_error("failed to start SPI\r\n");
	}

	/*
	 * Set the given SPI mode.
	 */
	int res = ioctl(fd, SPI_IOC_WR_MODE, mode);
	if (res < 0) {
		throw std::runtime_error("failed to set SPI write mode\r\n");
	}

	res = ioctl(fd, SPI_IOC_RD_MODE, mode);
	if (res < 0) {
		throw std::runtime_error("failed to set SPI read mode\r\n");
	}

	/*
	 * Set the given SPI bits-per-word.
	 */
	res = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, bpw);
	if (res < 0) {
		throw std::runtime_error("failed to set SPI write bits-per-word\r\n");
	}

	res = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, bpw);
	if (res < 0) {
		throw std::runtime_error("failed to set SPI read bits-per-word\r\n");
	}

	/*
	 * Set the given SPI speed.
	 */
	res = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, speed);
	if (res < 0) {
		throw std::runtime_error("failed to set SPI write speed\r\n");
	}

	res = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, speed);
	if (res < 0) {
		throw std::runtime_error("failed to set SPI read speed\r\n");
	}
}

spi::~spi() {
	close(fd);
}

int spi::transfer(
	const uint8_t *write_buf, const uint8_t *read_buf, uint32_t buf_len) const {
	struct spi_ioc_transfer transfer;
	int res;

	memset(&transfer, 0, sizeof(transfer));
	transfer.tx_buf = (unsigned long long)write_buf;
	transfer.rx_buf = (unsigned long long)read_buf;
	transfer.len = buf_len;
	transfer.speed_hz = speed;
	transfer.bits_per_word = bpw;
	transfer.cs_change = 1;

	res = ioctl(fd, SPI_IOC_MESSAGE(1), &transfer);
	if (res < 0) {
		printf("%s: failed to start SPI transfer\r\n", __func__);
		return res;
	}

	return res;
}

int spi::read(uint8_t reg, uint8_t *buf, int buf_len) const {
	int full_buf_len;
	int res;

	/*
	 * Allocate a buffer that contains the instruction and
	 * the register address as the first element.
	 */
	full_buf_len = buf_len + 2;
	uint8_t full_buf[full_buf_len];
	full_buf[0] = 0x0B;
	full_buf[1] = reg;

	/*
	 * Transfer the instruction, register address and data.
	 */
	res = transfer(full_buf, full_buf, full_buf_len);

	/*
	 * Copy the read data into the buffer.
	 */
	memcpy(buf, full_buf + 2, buf_len);

	return res;
}

uint8_t spi::read_byte(uint8_t reg) {
	uint8_t data = 0;
	if (read(reg, &data, 1) != 0) {
		throw std::runtime_error("SPI read fail");
	}
	return data;
}

int spi::write(uint8_t reg, uint8_t *buf, int buf_len) const {
	int full_buf_len;

	/*
	 * Allocate a buffer that contains the instruction and
	 * the register address as the first two elements.
	 */
	full_buf_len = buf_len + 2;
	uint8_t full_buf[full_buf_len];
	full_buf[0] = 0xA;
	full_buf[1] = reg;

	/*
	 * Copy the data to be written into the full buffer.
	 */
	memcpy(full_buf + 2, buf, buf_len);

	/*
	 * Transfer the instruction, register address and data.
	 */
	return transfer(full_buf, nullptr, full_buf_len);
}

int spi::write_byte(uint8_t reg, uint8_t value) {
	return write(reg, &value, 1);
}
