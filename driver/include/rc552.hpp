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

    /** 
     * @brief Read RFID information
     * 
     * @param[in] data - Buffer to store read information
     * @return Number of bytes read
    */
    int read_card(uint8_t blockAddr, uint8_t *data) const;

    // Pointer to the data to transfer to the FIFO for CRC calculation.
    // The number of bytes to transfer.
    // Pointer to result buffer. Result is written to result[0..1], low byte first.
    int rc552::CalculateCRC(uint8_t *data, uint8_t length, uint8_t *result);
					 

private:
	gpiod::line interrupt;
	std::unique_ptr<std::thread> interrupt_thread;
	std::atomic_bool active = true;
	i2c i2cd;
};
