/**
 * @file src/pwm.cpp
 * @brief Software PWM implementation.
 */
#include "pwm.hpp"

#include <chrono>
#include <memory>
#include <thread>

#include "defines.hpp"

static constexpr float SECOND_US = 1000000.0F;

pwm_worker::pwm_worker(const gpiod::chip &chip, uint32_t pin, uint32_t freq)
	: line(chip.get_line(pin)) {
	// Set pin to output
	line.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0,
	});

	// Time for 1% duty
	float percent_us = SECOND_US / (float)freq / 100.0F;

	// Thread function
	auto executor = [&, percent_us]() {
		while (active) {
			// sleep_for(0) would still create a small HIGH spike
			if (duty_percent != 0) {
				line.set_value(HIGH);
				auto duration = (uint64_t)(duty_percent * percent_us);
				std::this_thread::sleep_for(std::chrono::microseconds(duration));
			}

			// Same here
			if (duty_percent != 100) {
				line.set_value(LOW);
				auto duration = (uint64_t)((100.0F - duty_percent) * percent_us);
				std::this_thread::sleep_for(std::chrono::microseconds(duration));
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
