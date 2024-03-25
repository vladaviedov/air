/**
 * @file include/device.hpp
 * @brief This header contains constants related to the device.
 */
#pragma once

#include <gpiod.hpp>

constexpr std::string gpiochip0 = "gpiochip0";

extern const gpiod::chip gpio_pins;

constexpr int I2C_ADAPTER_NUMBER = 0;

constexpr uint8_t GY521_DEV_ADDR = 0b1101000;
