/**
 * @file src/i2c.cpp
 * @brief I2C driver.
 */
#include "i2c.hpp"

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

i2c::i2c(uint8_t dev_addr) {
    int fd;
    int adapter_nr = 1;
    char filename[32];

    snprintf(filename, 31, "/dev/i2c-%d", adapter_nr);
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        throw std::runtime_error("Failed to open i2c port");
    }

    if (ioctl(fd, I2C_SLAVE, addr) < 0) {
        throw std::runtime_error("Failed to open i2c port (ioctl)");
    }
}

i2c::read(char* data, uint8_t size) {
    size += 1;
    if (read(fd, data, size) != size) {
        throw std::runtime_error("Failed to read i2c");
    }
}

i2c::write(uint8_t reg, char* data, uint8_t size) {
    char* buf;
    *buf[0] = reg;
    std::copy(data, data + size, buf + 1);
    // size is now the size of buf
    size += 1;
    if (write(fd, data, size) != size) {
        throw std::runtime_error("Failed to write i2c");
    }
}