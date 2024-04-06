/**
 * @file src/calibrate.cpp
 * @brief Car sensor and drive calibration.
 */
#include "calibrate.hpp"

#include <exception>
#include <iostream>
#include <string>
#include <vector>

#include <shared/menu.hpp>

#include "common.hpp"
#include "profile.hpp"

static void load_default();
static void load_from();
static void load(const std::string &from_file);

static void save_default();
static void save_as();
static void save(const std::string &to_file);

static void print_calibration();

static const std::vector<menu_item> calib_options = {
	{.text = "Print calibration", .action = &print_calibration},
	{.text = "Reload default profile", .action = &load_default},
	{.text = "Load profile from...", .action = &load_from},
	{.text = "Save default profile", .action = &save_default},
	{.text = "Save profile as...", .action = &save_as}};

void calibration_submenu() {
	show_menu("Calibration", calib_options, true);
}

void load_default() {
	load(default_profile);
}

void load_from() {
	std::cout << "Input filename: ";

	std::string input;
	std::getline(std::cin, input);

	load(input);
}

void load(const std::string &from_file) {
	std::cout << "Loading profile from: " << from_file << std::endl;
	try {
		car_profile.load(from_file);
	} catch (std::exception &ex) {
		std::cout << "Failed to load profile" << std::endl;
		std::cerr << ex.what() << std::endl;

		prompt_enter();
		return;
	}

	std::cout << "\nLoaded settings: \n";
	print_calibration();
}

void save_default() {
	save(default_profile);
}

void save_as() {
	std::cout << "Input filename: ";

	std::string input;
	std::getline(std::cin, input);

	save(input);
}

void save(const std::string &to_file) {
	std::cout << "Saving profile to: " << to_file << std::endl;
	try {
		car_profile.save(to_file);
	} catch (std::exception &ex) {
		std::cout << "Failed to save profile" << std::endl;
		std::cerr << ex.what() << std::endl;

		prompt_enter();
		return;
	}

	std::cout << "Profile saved" << std::endl;
	prompt_enter();
}

/**
 * @brief Print out calibration values from the profile.
 *
 */
void print_calibration() {
	auto servo_profile = car_profile.get_servo();
	auto tdma_profile = car_profile.get_tdma();

	std::cout << "----------\n";
	if (servo_profile.has_value()) {
		std::cout << "Servo:\n\n";
		std::cout << "Max left: " << servo_profile->max_left << '\n';
		std::cout << "Max right: " << servo_profile->max_right << '\n';
		std::cout << "Center: " << servo_profile->center << '\n';
		std::cout << '\n';
	} else {
		std::cout << "Servo: not defined\n";
	}

	if (tdma_profile.has_value()) {
		std::cout << "TDMA:\n\n";
		std::cout << "TX offset: " << tdma_profile->rx_offset_ms << " ms\n";
		std::cout << "RX offset: " << tdma_profile->tx_offset_ms << " ms\n";
	} else {
		std::cout << "TDMA: not defined\n";
	}
	std::cout << "----------\n";

	prompt_enter();
}
