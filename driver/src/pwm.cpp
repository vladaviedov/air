/**
 * @file src/pwm.cpp
 * @brief Software PWM implementation.
 */
#include "pwm.hpp"

#include <chrono>
#include <memory>
#include <thread>

#include "defines.hpp"

// 500 Hz PWM -> 2000us / period
constexpr uint32_t TOTAL_TIME = 2000;
constexpr uint32_t TIME_PER_PERCENT = TOTAL_TIME / 100;

pwm_worker::pwm_worker(const gpiod::chip &chip, uint32_t pin)
	: line(chip.get_line(pin)) {
	// Set pin to output
	line.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0,
	});

	// Thread function
	auto executor = [&]() {
		while (active) {
			// sleep_for(0) would still create a small HIGH spike
			if (duty_percent != 0) {
				line.set_value(HIGH);
				std::this_thread::sleep_for(
					std::chrono::microseconds(duty_percent * TIME_PER_PERCENT));
			}

			// Same here
			if (duty_percent != 100) {
				line.set_value(LOW);
				std::this_thread::sleep_for(std::chrono::microseconds(
					(100 - duty_percent) * TIME_PER_PERCENT));
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
