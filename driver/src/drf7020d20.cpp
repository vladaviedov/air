/**
 * @file src/drf7020d20.cpp
 * @brief Driver implementation for the DRF7020D20 board
 */
#include "drf7020d20.hpp"

#include <chrono>
#include <cstdio>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <stdexcept>
#include <string>
#include <thread>
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
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0
	});
	aux.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::DIRECTION_INPUT,
		.flags = 0
	});
	set.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0
	});

	en.set_value(0);
	set.set_value(1);

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

void drf7020d20::enable() {
	en.set_value(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(50));
	enable_flag = true;
}

void drf7020d20::disable() {
	en.set_value(0);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	enable_flag = false;
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
	std::this_thread::sleep_for(std::chrono::milliseconds(1));

	// Send command
	char buf[21];
	std::snprintf(buf, 21, "WR %u %u %u %u %u\n",
		freq, fsk_rate, power_level, uart_rate, parity);
	if (write(serial_fd, buf, std::strlen(buf)) < 0) {
		return false;
	}

	// Expected response
	char expect[21];
	std::snprintf(expect, 21, "PARA %u %u %u %u %u\n",
		freq, fsk_rate, power_level, uart_rate, parity);

	// Get response
	if (read(serial_fd, buf, 20) < 0) {
		return false;
	}

	// Verify response
	if (std::strncmp(buf, expect, 20) != 0) {
		return false;
	}

	set.set_value(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	return true;
}

bool drf7020d20::transmit(const char *msg, uint32_t length) const {
	if (!enable_flag) {
		return false;
	}

	return write(serial_fd, msg, length);
}
