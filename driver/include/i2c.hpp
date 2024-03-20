/**
 * @file include/i2c.hpp
 * @brief Drivers for I2C using the kernel smbus driver.
 */
#pragma once

#include <cstdint>

class i2c {
public:
	/**
	 * @brief Constructor. Initializes I2C device.
	 *
	 * @param[in] addr - device address of the I2C slave device.
	 */
	i2c(uint8_t addr, int adapter_nr);

	~i2c();

	/**
	 * @brief Read up to 32 bytes.
	 *
	 * @param[in] reg - Register to read from.
	 * @param[out] data - Data buffer to write to.
	 * @param[in] size - Size of data buffer.
	 * @return number of bytes read.
	 */
	int read(uint8_t reg, uint8_t *data) const;

	/**
	 * @brief Write up to 32 bytes.
	 *
	 * @param[in] reg - Register to write to.
	 * @param[in] data - Data buffer to write.
	 * @param[in] size - Size of data buffer.
	 * @return Success (true) or failure (not true).
	 */
	bool write(uint8_t reg, const uint8_t *data, uint8_t size) const;

private:
	int fd;
	uint8_t dev_addr;
};
