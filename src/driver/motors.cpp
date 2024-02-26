/**
 * @file driver/motor.cpp
 * @brief Motor driver implementation with the provided motor controller.
 */
#include "motors.hpp"

#include <cstdint>
#include <gpiod.hpp>

#include "pwm.hpp"

#define GPIO_USER "robot2_motors"

motor::motor(gpiod::chip &chip, uint32_t in1_pin, uint32_t in2_pin, uint32_t pwm_pin) :
	ain1(chip.get_line(in1_pin)),
	ain2(chip.get_line(in2_pin)),
	pwm(chip, pwm_pin) {
	// Set control pins to output
	ain1.request({
		.consumer = GPIO_USER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0
	}, 0);
	ain2.request({
		.consumer = GPIO_USER,
		.request_type = gpiod::line_request::DIRECTION_OUTPUT,
		.flags = 0
	}, 0);
}

motor::~motor() {
	ain1.release();
	ain2.release();
}

void motor::stop() {
	ain1.set_value(0);
	ain2.set_value(0);
	pwm.set_duty(0);
}

void motor::set(uint32_t speed, direction dir) {
	if (inverted) {
		dir = direction::FORWARD ? direction::BACKWARD : direction::FORWARD;
	}

	if (dir == direction::FORWARD) {
		ain1.set_value(1);
		ain2.set_value(0);
	} else {
		ain1.set_value(0);
		ain2.set_value(1);
	}

	pwm.set_duty(speed);
}
