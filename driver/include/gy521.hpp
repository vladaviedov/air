/**
 * @file include/gy521.hpp
 * @brief Drivers for the GY-521 6 axis accelerometer.
 */

#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>

#include <gpiod.hpp>

#include "i2c.hpp"

class gy521 {
public:
	/**
	 * @brief Initializer
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] int_pin - Interrupt libgpiod pin number.
	 */
	gy521(const gpiod::chip &chip,
		uint32_t int_pin,
		uint8_t dev_addr,
		int adapter);

	~gy521();

	/**
	 * @brief Call callback on interrupt.
	 *
	 * @param[in] callback - Function to call on interrupt
	 */
	void on_interrupt(std::function<void()> callback);

	/**
	 * @brief Read x-axis acceleration
	 *
	 * @return X axis acceleration.
	 */
	uint16_t read_x_axis() const;

	/**
	 * @brief Read y-axis acceleration
	 *
	 * @return Y axis acceleration.
	 */
	uint16_t read_y_axis() const;

	/**
	 * @brief Read z-axis acceleration
	 *
	 * @return Z axis acceleration.
	 */
	uint16_t read_z_axis() const;

	/**
	 * @brief Read x-axis acceleration
	 *
	 * @return X rotation.
	 */
	uint16_t read_x_rot() const;

	/**
	 * @brief Read y-axis acceleration
	 *
	 * @param[out] data - Word to read.
	 * @return Y rotation.
	 */
	uint16_t read_y_rot() const;

	/**
	 * @brief Read z-axis acceleration
	 *
	 * @return Z rotation.
	 */
	uint16_t read_z_rot() const;

private:
	gpiod::line interrupt;
	std::unique_ptr<std::thread> int_thread;
	std::atomic_bool active = true;
	i2c i2cd;
};
