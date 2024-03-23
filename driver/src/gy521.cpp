/**
 * @file include/gy521.cpp
 * @brief Drivers for the GY-521 6 axis accelerometer.
 */

#include "gy521.hpp"

#include <chrono>
#include <cstdint>

#include "defines.hpp"
#include "device.hpp"

constexpr uint8_t REG_INTERRUPT_STATUS = 0x3A;
constexpr uint8_t REG_ACCEL_X = 0x3B;
constexpr uint8_t REG_ACCEL_Y = 0x3D;
constexpr uint8_t REG_ACCEL_Z = 0x3F;
constexpr uint8_t REG_GYRO_X = 0x43;
constexpr uint8_t REG_GYRO_Y = 0x45;
constexpr uint8_t REG_GYRO_Z = 0x47;

gy521::gy521(const gpiod::chip &chip, uint32_t int_pin)
	: interrupt(chip.get_line(int_pin)),
	  i2cd(i2c(GY521_DEV_ADDR, I2C_ADAPTER_NUMBER)) {
	interrupt.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::EVENT_RISING_EDGE,
		.flags = 0,
	});
}

gy521::~gy521() {
	active = false;
	int_thread->join();
	interrupt.release();
}

void gy521::on_interrupt(std::function<void()> callback) {
	// Thread function
	auto executor = [&]() {
		while (active) {
			if (interrupt.event_wait(std::chrono::milliseconds(100))) {
				interrupt.event_read();
				i2cd.read_byte(REG_INTERRUPT_STATUS);
				callback();
			}
		}
	};

	int_thread = std::make_unique<std::thread>(executor);
}

/**
 * @brief Read x-axis acceleration
 *
 * @param[out] data - Word to read.
 * @return Number of btyes read
 */
uint16_t gy521::read_x_axis() const {
	return i2cd.read_word(REG_ACCEL_X);
}

/**
 * @brief Read y-axis acceleration
 *
 * @param[out] data - Word to read.
 * @return Number of btyes read
 */
uint16_t gy521::read_y_axis() const {
	return i2cd.read_word(REG_ACCEL_Y);
}

/**
 * @brief Read z-axis acceleration
 *
 * @param[out] data -Word to read.
 * @return Number of btyes read
 */
uint16_t gy521::read_z_axis() const {
	return i2cd.read_word(REG_ACCEL_Z);
}

/**
 * @brief Read x-axis acceleration
 *
 * @param[out] data - Word to read.
 * @return Number of btyes read
 */
uint16_t gy521::read_x_rot() const {
	return i2cd.read_word(REG_GYRO_X);
}

/**
 * @brief Read y-axis acceleration
 *
 * @param[out] data - Word to read.
 * @return Number of btyes read
 */
uint16_t gy521::read_y_rot() const {
	return i2cd.read_word(REG_GYRO_Y);
}

/**
 * @brief Read z-axis acceleration
 *
 * @param[out] data - Word to read.
 * @return Number of btyes read
 */
uint16_t gy521::read_z_rot() const {
	return i2cd.read_word(REG_GYRO_Z);
}
