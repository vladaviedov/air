/**
 * @file driver/hcsr04.cpp
 * @brief Driver implementation for HC-SR04.
 */
#include "hcsr04.hpp"

#include <cstdint>
#include <stdexcept>
#include <thread>
#include <chrono>
#include <mutex>
#include <gpiod.hpp>

#define GPIO_USER "robot2_sensor"

hc_sr04::hc_sr04(gpiod::chip &chip, uint32_t trig_pin, uint32_t echo_pin) :
	trig(chip.get_line(trig_pin)),
	echo(chip.get_line(echo_pin)) {
	// Set trig pin to output
	trig.request({
		.consumer = GPIO_USER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0
	}, 0);
	
	// Set echo pin to input
	echo.request({
		.consumer = GPIO_USER,
		.request_type = gpiod::line_request::DIRECTION_INPUT,
		.flags = 0
	});
}

hc_sr04::~hc_sr04() {
	trig.release();
	echo.release();
}

uint64_t hc_sr04::pulse() const {
	// Send trig pulse
	trig.set_value(1);
	std::this_thread::sleep_for(std::chrono::microseconds(10));
	trig.set_value(0);
	
	// Wait for pulse start
	while (!echo.get_value()) {
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}

	// Get initial time
	auto initial = std::chrono::steady_clock::now();

	// Wait for pulse
	while (echo.get_value()) {
		std::this_thread::sleep_for(std::chrono::microseconds(10));
	}

	// Get final time
	auto final = std::chrono::steady_clock::now();

	// Compute & return delta
	return std::chrono::duration_cast<std::chrono::microseconds>(final - initial).count();
}
