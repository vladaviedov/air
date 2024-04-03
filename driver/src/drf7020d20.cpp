/**
 * @file src/drf7020d20.cpp
 * @brief Driver implementation for the DRF7020D20 board
 */
#include "drf7020d20.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <fcntl.h>
#include <memory>
#include <stdexcept>
#include <string>
#include <thread>
#include <unistd.h>

#include <gpiod.hpp>

#include "defines.hpp"

drf7020d20::drf7020d20(const gpiod::chip &chip,
	uint32_t en_pin,
	uint32_t aux_pin,
	uint32_t set_pin,
	uint32_t uart_port)
	: serial(uart_port),
	  en(chip.get_line(en_pin)),
	  aux(chip.get_line(aux_pin)),
	  set(chip.get_line(set_pin)) {
	en.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0,
	});
	aux.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::EVENT_FALLING_EDGE,
		.flags = 0,
	});
	set.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0,
	});

	en.set_value(0);
	set.set_value(1);
}

drf7020d20::~drf7020d20() {
	rejecter_off();
	en.release();
	aux.release();
	set.release();
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

void drf7020d20::rejecter_on() {
	if (rejecter_thread != nullptr) {
		return;
	}

	auto executor = [&]() {
		while (rejecter) {
			if (aux.event_wait(std::chrono::milliseconds(100))) {
				aux.event_read();
			}
		}
	};

	rejecter = true;
	rejecter_thread = std::make_unique<std::thread>(executor);
}

void drf7020d20::rejecter_off() {
	rejecter = false;
	rejecter_thread->join();
	rejecter_thread.reset();
}

bool drf7020d20::configure(uint32_t freq,
	rate fsk_rate,
	uint32_t power_level,
	rate uart_rate,
	parity parity) const {
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
	char buf[22];
	std::snprintf(buf, 22, "WR %u %u %u %u %u\r\n", freq, fsk_rate, power_level,
		uart_rate, parity);
	serial.write(buf, std::strlen(buf));

	// Expected response
	char expect[22];
	std::snprintf(expect, 22, "PARA %u %u %u %u %u\r\n", freq, fsk_rate,
		power_level, uart_rate, parity);

	// Get response
	std::this_thread::sleep_for(std::chrono::milliseconds(200));
	if (serial.read(buf, 21) < 0) {
		return false;
	}

	// Verify response
	bool verify = std::strncmp(buf, expect, 21) == 0;

	// Reset
	set.set_value(1);
	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	return verify;
}

bool drf7020d20::transmit(const std::string &msg) const {
	if (!enable_flag) {
		throw std::logic_error("Radio is disabled, cannot transmit");
	}

	return serial.write(msg);
}

bool drf7020d20::transmit(const char *msg, uint32_t length) const {
	if (!enable_flag) {
		throw std::logic_error("Radio is disabled, cannot transmit");
	}

	return serial.write(msg, length);
}

std::string drf7020d20::receive(std::chrono::milliseconds timeout) const {
	if (!enable_flag) {
		throw std::logic_error("Radio is disabled, cannot receive");
	}

	// AUX will fall when data is ready to read
	if (!aux.event_wait(timeout)) {
		return "";
	}

	// Clear event
	if (!rejecter) {
		aux.event_read();
	}

	return serial.read();
}
