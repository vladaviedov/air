/**
 * @file src/drf7020d20.cpp
 * @brief Driver implementation for the DRF7020D20 board
 */
#include "drf7020d20.hpp"

#include <cstdio>
#include <cstdint>
#include <cstring>
#include <stdexcept>
#include <string>
#include <unistd.h>
#include <fcntl.h>
#include <gpiod.hpp>

#include "defines.hpp"

drf7020d20::drf7020d20(
		gpiod::chip &chip,
		uint32_t en_pin,
		uint32_t aux_pin,
		uint32_t set_pin,
		uint32_t uart_port) :
	en(chip.get_line(en_pin)),
	aux(chip.get_line(aux_pin)),
	set(chip.get_line(set_pin)) {
	en.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line::DIRECTION_INPUT,
		.flags = 0
	});
	aux.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line::DIRECTION_OUTPUT,
		.flags = 0
	});
	set.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line::DIRECTION_INPUT,
		.flags = 0
	});

	en.set_value(0);
	set.set_value(0);

	// Open UART serial
	std::string serial = "/dev/serial" + std::to_string(uart_port);
	serial_fd = open(serial.c_str(), O_RDWR);
	if (serial_fd < 0) {
		throw std::runtime_error("Failed to open serial port");
	}
}

drf7020d20::~drf7020d20() {
	en.release();
	aux.release();
	set.release();
	close(serial_fd);
}

void drf7020d20::enable() const {
	en.set_value(1);
}

void drf7020d20::disable() const {
	en.set_value(0);
}

bool drf7020d20::configure(
	uint32_t freq,
	rate fsk_rate,
	uint32_t power_level,
	rate uart_rate,
	parity parity
) const {
	// Input validation
	if (freq < 418000 || freq > 455000) {
		return false;
	}
	if (fsk_rate < DR2400 || fsk_rate > DR19200) {
		return false;
	}
	if (power_level > 9) {
		return false;
	}

	// Enable set mode
	set.set_value(0);

	// Send command
	char buf[32];
	std::snprintf(buf, 32, "WR %u %u %u %u %u\r\n",
		freq, fsk_rate, power_level, uart_rate, parity);
	if (write(serial_fd, buf, 32) < 0) {
		return false;
	}

	// Get response
	if (read(serial_fd, buf, 32) < 0) {
		return false;
	}

	// Verify response
	char expect[32];
	std::snprintf(expect, 32, "PARA %u %u %u %u %u\r\n",
		freq, fsk_rate, power_level, uart_rate, parity);
	if (std::strcmp(buf, expect) != 0) {
		return false;
	}

	return true;
}
