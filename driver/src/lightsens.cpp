/**
 * @file src/lightsens.cpp
 * @brief Infrared sensor driver implementation.
 */
#include "lightsens.hpp"

#include <cstdint>

#include <gpiod.hpp>

#include "defines.hpp"

light_sens::light_sens(const gpiod::chip &chip, uint32_t input_pin)
	: input(chip.get_line(input_pin)) {
	// Set input pin to input
	input.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::DIRECTION_INPUT,
		.flags = 0,
	});
}

light_sens::~light_sens() {
	input.release();
}

bool light_sens::read() const {
	// Sensor goes LOW when edge is detected
	return input.get_value() == LOW;
}
