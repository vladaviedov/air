/**
 * @file include/i2c.hpp
 * @brief Drivers for I2C using the kernel smbus driver.
 */
#pragma once

class i2c {

public:

    /**
     * @brief Constructor. Initializes I2C device.
     *
     * @param[in] dev_addr - address of the I2C slave device.
     */
    i2c(const uint8_t dev_addr);

    /** 
     * @brief I2C read.
     *
     * @param[in] data - Data buffer to write to.
     * @param[in] size - Size of data buffer.
     * @return error code
     */
    int read(char* data, uint8_t size);

    /** 
     * @brief I2C write.
     *
     * @param[in] reg - Register to read from.
     * @param[in] data - Data buffer to write.
     * @param[in] size - Size of data buffer.
     * @return error code
     */
    int write(uint8_t reg, char* data, uint8_t size);

private:
    int fd;
    const uint8_t dev_addr;
    
}