/**
 * @file include/servo.hpp
 * @brief Servo motor header.
 */
#pragma once

#include <cstdint>

#include <gpiod.hpp>

#include "pwm.hpp"

class servo {
public:
	/**
	 * @brief Constructor.
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] pin - Servo PWM pin.
	 */
	servo(const gpiod::chip &chip, uint32_t pin);

	/**
	 * @brief Set servo to desired position.
	 *
	 * @param[in] position - Position in degrees (from 0 to 180).
	 */
	void set(uint32_t position);

private:
	pwm_worker pwm;
};
