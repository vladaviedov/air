/**
 * @file src/common.cpp
 * @brief For anything that needs to be shared.
 */
#include "common.hpp"

#include <iostream>

#include "profile.hpp"

void prompt_enter() {
	std::cout << "\nPress enter to continue." << std::flush;
	std::getchar();
}

// NOLINTNEXTLINE(cppcoreguidelines-avoid-non-const-global-variables)
profile car_profile;
