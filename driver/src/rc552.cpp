/**
 * @file include/rc552.cpp
 * @brief Drivers for RC552 RFID reader
 */

#include "rc552.hpp"

#include <chrono>
#include <cstdint>

#include "defines.hpp"

rc552::rc552(
	const gpiod::chip &chip, uint32_t inter_pin, uint8_t dev_addr, int adapter)
	: interrupt(chip.get_line(inter_pin)),
	  i2cd(i2c(dev_addr, adapter)) {
        
}
