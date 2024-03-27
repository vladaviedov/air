/**
 * @file src/pwm.cpp
 * @brief Software PWM implementation.
 */
#include "pwm.hpp"

#include <chrono>
#include <memory>
#include <thread>

#include "defines.hpp"

static constexpr uint32_t SECOND_US = 1000000;

pwm_worker::pwm_worker(const gpiod::chip &chip, uint32_t pin, uint32_t freq)
	: line(chip.get_line(pin)) {
	// Set pin to output
	line.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0,
	});

	// Time for 1% duty
	uint32_t percent_us = SECOND_US / freq / 100;

	// Thread function
	auto executor = [&, percent_us]() {
		while (active) {
			// sleep_for(0) would still create a small HIGH spike
			if (duty_percent != 0) {
				line.set_value(HIGH);
				std::this_thread::sleep_for(
					std::chrono::microseconds(duty_percent * percent_us));
			}

			// Same here
			if (duty_percent != 100) {
				line.set_value(LOW);
				std::this_thread::sleep_for(std::chrono::microseconds(
					(100 - duty_percent) * percent_us));
			}
		}
	};

	// Create worker thread
	pwm_thread = std::make_unique<std::thread>(executor);
}

pwm_worker::~pwm_worker() {
	// Deactivate thread & wait for join
	active = false;
	pwm_thread->join();

	// Reset gpio line
	line.set_value(LOW);
	line.release();
}
