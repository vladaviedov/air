/**
 * @file include/device.hpp
 * @brief This header contains constants related to the device.
 */
#pragma once

#include <gpiod.hpp>

constexpr std::string gpiochip0 = "gpiochip0";

extern const gpiod::chip gpio_pins; 
