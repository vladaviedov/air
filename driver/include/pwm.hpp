/**
 * @file include/pwm.hpp
 * @brief Software PWM header.
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <memory>
#include <thread>

#include <gpiod.hpp>

class pwm_worker {
public:
	/**
	 * @brief Constructor.
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] pin - libgpiod pin number.
	 */
	pwm_worker(const gpiod::chip &chip, uint32_t pin);

	~pwm_worker();

	/**
	 * @brief Set PWM duty cycle.
	 *
	 * @param[in] percent - New duty cycle (in percent).
	 */
	inline void set_duty(uint32_t percent) {
		duty_percent = percent;
	}

private:
	gpiod::line line;
	std::unique_ptr<std::thread> pwm_thread;
	std::atomic_bool active = true;
	uint32_t duty_percent = 0;
};
