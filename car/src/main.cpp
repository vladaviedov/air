/**
 * @file src/main.cpp
 * @brief Car entry point.
 */
#include <iostream>
#include <vector>

#include <shared/menu.hpp>
#include <shared/utils.hpp>

#include "calibrate.hpp"
#include "common.hpp"
#include "demos.hpp"
#include "profile.hpp"

const std::string about =R"(AIR Car Controller
Copyright (C) 2024 Vladyslav Aviedov, Caio DaSilva, Scott Abramson

This program is part of a project for GE1502 Cornerstone of Engineering at
Northeastern University (Spring 2024). Project group members:
- Aidan Hanson
- Caio DaSilva
- Scott Abramson
- Vladyslav Aviedov

License: GNU General Public License v2.0

This program is free software; you can redistribute it and/or modify it under
the terms of the GNU General Public License version 2 as published by the Free
Software Foundation.

This program is distributed in the hope that it will be useful, but WITHOUT ANY
WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A
PARTICULAR PURPOSE. See the GNU General Public License for more details.
)";

static void print_about();

static const std::vector<menu_item> car_menu = {
	{.text = "Demos", .action = &demo_submenu},
	{.text = "Calibration", .action = &calibration_submenu},
	{.text = "About this program", .action = &print_about}};

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

void print_about() {
	std::cout << about;
	prompt_enter();
}
