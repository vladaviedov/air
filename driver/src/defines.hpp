/**
 * @file src/defines.h
 * @brief Common defines used throughout the driver library.
 */
#pragma once

#include <string>

// libgpiod consumer name
constexpr std::string GPIO_CONSUMER = "air_driver";

// Logic levels
constexpr int LOW = 0;
constexpr int HIGH = 1;
