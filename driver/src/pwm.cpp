/**
 * @file driver/pwm.cpp
 * @brief Software PWM implementation.
 */
#include "pwm.hpp"

#include <memory>
#include <chrono>
#include <thread>

// 500 Hz PWM -> 2000us / period
#define TOTAL_TIME 2000
#define TIME_PER_PERCENT (TOTAL_TIME / 100)

#define GPIO_USER "robot2_pwm"

pwm_worker::pwm_worker(gpiod::chip &chip, uint32_t pin) :
	line(chip.get_line(pin)) {
	// Set pin to output
	line.request({
		.consumer = GPIO_USER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0
	}, 0);

	// Thread function
	auto executor = [&](){
		while (active) {
			// sleep_for(0) would still create a small HIGH spike
			if (duty_percent != 0) {
				line.set_value(1);
				std::this_thread::sleep_for(
					std::chrono::microseconds(duty_percent * TIME_PER_PERCENT));
			}

			// Same here
			if (duty_percent != 100) {
				line.set_value(0);
				std::this_thread::sleep_for(
					std::chrono::microseconds((100 - duty_percent) * TIME_PER_PERCENT));
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
	line.set_value(0);
	line.release();
}
