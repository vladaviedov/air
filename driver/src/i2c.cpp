/**
 * @file src/i2c.cpp
 * @brief I2C driver.
 */
#include "i2c.hpp"

#include <linux/i2c-dev.h>
#include <i2c/smbus.h>

i2c::i2c(const uint8_t addr, const int adapter_nr) {

    dev_addr = addr;

    char filename[32];

    snprintf(filename, 32, "/dev/i2c-%d", adapter_nr);
    fd = open(filename, O_RDWR);
    if (fd < 0) {
        throw std::runtime_error("Failed to open i2c port");
    }

    if (ioctl(fd, I2C_SLAVE, dev_addr) < 0) {
        throw std::runtime_error("Failed to open i2c port (ioctl)");
    }
}

i2c::~i2c() {
    fd.close();
}

int i2c::read(char* data, const uint32_t size) {
    return read(fd, data, size);
}

boolean i2c::write(const char* data, const uint32_t size) {
    return write(fd, data, size) == size;
}
