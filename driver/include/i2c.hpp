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
    i2c(const uint8_t addr, const int adapter_nr);

    ~i2c();

    /** 
     * @brief I2C read.
     *
     * @param[out] data - Data buffer to write to.
     * @param[in] size - Size of data buffer.
     * @return number of bytes read
     */
    const int read(char* data, const uint32_t size);

    /** 
     * @brief I2C write.
     *
     * @param[in] reg - Register to read from.
     * @param[in] data - Data buffer to write.
     * @param[in] size - Size of data buffer.
     * @return number of bytes read
     */
    const boolean write(const char* data, const uint32_t size);

private:
    int fd;
    const uint8_t dev_addr;
}
