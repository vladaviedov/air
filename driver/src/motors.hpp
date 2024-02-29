/**
 * @file driver/motor.hpp
 * @brief Motor driver header for the provided motor controller.
 */
#pragma once

#include <cstdint>
#include <gpiod.hpp>

#include "pwm.hpp"

enum direction {
	FORWARD,
	BACKWARD
};

class motor {

public:
	/**
	 * @brief Constructor.
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] in1_pin - MxIN1 libgpiod pin number.
	 * @param[in] in2_pin - MxIN2 libgpiod pin number.
	 * @param[in] pwm_pin - MxPWM libgpiod pin number.
	 */
	motor(gpiod::chip &chip, uint32_t in1_pin, uint32_t in2_pin, uint32_t pwm_pin);

	~motor();

	/**
	 * @brief Stop the motor.
	 */
	void stop();

	/**
	 * @brief Set motor to desired speed.
	 *
	 * @param[in] speed - Motor speed (1-100).
	 * @param[in] dir - Motor direction.
	 */
	void set(uint32_t speed, direction dir);

	/**
	 * @brief Invert motor direction when using 'set'.
	 *
	 */
	inline void invert() {
		inverted = !inverted;
	}

private:
	const gpiod::line ain1;
	const gpiod::line ain2;
	pwm_worker pwm;
	bool inverted;

};
