/**
 * @file src/servo.cpp
 * @brief Servo motor implementation.
 */
#include "servo.hpp"

#include <cstdint>

#include <gpiod.hpp>

#include "pwm.hpp"

// Standard servo use 20ms intervals
static constexpr uint32_t INTERVAL_US = 20000;
static constexpr uint32_t SERVO_PWM_FREQ = 50;

// Minimum and maximum PWM writes
static constexpr float MIN_PWM_US = 500.0F;
static constexpr float MAX_PWM_US = 2500.0F;

servo::servo(const gpiod::chip &chip, uint32_t pin)
	: pwm(chip, pin, SERVO_PWM_FREQ) {
	set(90);
}

void servo::set(uint32_t position) {
	float duty_us =
		MIN_PWM_US + (MAX_PWM_US - MIN_PWM_US) * (float)position / 180.0F;
	pwm.set_duty(duty_us / INTERVAL_US * 100);
}
