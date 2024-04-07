/**
 * @file src/common.hpp
 * @brief For anything that needs to be shared.
 */
#pragma once

#include "profile.hpp"

/** Calibration profile */
const std::string default_profile = "/etc/air/profile";
// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
extern profile car_profile;
