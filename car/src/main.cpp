/**
 * @file src/main.cpp
 * @brief Car entry point.
 */
#include <vector>
#include <iostream>

#include <shared/menu.hpp>

#include "common.hpp"
#include "demos.hpp"
#include "calibrate.hpp"
#include "profile.hpp"

static const std::vector<menu_item> car_menu = {
	{.text = "Demos", .action = &demo_submenu},
	{.text = "Calibration", .action = &calibration_submenu}
};

int main() {
	car_profile.load(default_profile);
	if (!car_profile.is_done()) {
		std::cout << "No calibration data for:\n\n";

		if (!car_profile.get_servo().has_value()) {
			std::cout << "Servo\n";
		}
		if (!car_profile.get_tdma().has_value()) {
			std::cout << "TDMA\n";
		}

		std::cout << "\nPlease visit the calibration menu\n";

		prompt_enter();
	}

	show_menu("Car Actions", car_menu, false);
	return 0;
}
