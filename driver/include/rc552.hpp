/**
 * @file include/rc552.hpp
 * @brief Drivers for RC552 RFID reader
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>

#include <gpiod.hpp>

#include "i2c.hpp"

class rc552 {
public:
	/**
	 * @brief Initializer
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] inter_pin - Interrupt libgpiod pin number.
	 * @param[in] dev_addr - I2C device address.
	 * @param[in] adapter - I2C ioctl adapter number.
	 */
	rc552(const gpiod::chip &chip,
		uint32_t inter_pin,
		uint8_t dev_addr,
		int adapter);

	~rc552();

	/**
	 * @brief Call callback on interrupt.
	 *
	 * @param[in] callback - Function to call on interrupt
	 */
	void on_interrupt(std::function<void()> callback);

    

private:
	gpiod::line interrupt;
	std::unique_ptr<std::thread> int_thread;
	std::atomic_bool active = true;
	i2c i2cd;
};
