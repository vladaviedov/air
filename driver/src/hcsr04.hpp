/**
 * @file driver/hcsr04.hpp
 * @brief Driver header for the HC-SR04 ultrasonice sensor.
 */
#pragma once

#include <cstdint>
#include <thread>
#include <functional>
#include <mutex>
#include <gpiod.hpp>

class hc_sr04 {

public:
	/**
	 * @brief Constructor.
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] trig_pin - US_TRIG libgpiod pin number.
	 * @param[in] echo_pin - US_ECHO libgpiod pin number.
	 */
	hc_sr04(gpiod::chip &chip, uint32_t trig_pin, uint32_t echo_pin);

	~hc_sr04();

	/**
	 * @brief Get distance reading pulse from the sensor.
	 *
	 * @return Pulse length (us).
	 */
	uint64_t pulse() const;

private:
	const gpiod::line trig;
	const gpiod::line echo;
};
