/**
 * @file src/calibrate.cpp
 * @brief Car sensor and drive calibration.
 */
#include "calibrate.hpp"

#include <cstdint>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <vector>

#include <driver/device.hpp>
#include <driver/pinmap.hpp>
#include <driver/servo.hpp>
#include <shared/menu.hpp>
#include <shared/utils.hpp>

#include "common.hpp"
#include "profile.hpp"

static void load_default();
static void load_from();
static void load(const std::string &from_file);

static void save_default();
static void save_as();
static void save(const std::string &to_file);

static void print_calibration();

static void calibrate_servo();
static std::optional<uint32_t> servo_value_select(
	servo &servo, uint32_t start_value);

static const std::vector<menu_item> calib_options = {
	{.text = "Print calibration", .action = &print_calibration},
	{.text = "Calibrate servo", .action = &calibrate_servo},
	{.text = "Reload default profile", .action = &load_default},
	{.text = "Load profile from...", .action = &load_from},
	{.text = "Save default profile", .action = &save_default},
	{.text = "Save profile as...", .action = &save_as}};

void calibration_submenu() {
	show_menu("Calibration", calib_options, true);
}

/**
 * @brief Load default profile.
 *
 */
void load_default() {
	load(default_profile);
}

/**
 * @brief Load from another file.
 *
 */
void load_from() {
	std::cout << "Input filename: ";

	std::string input;
	std::getline(std::cin, input);

	load(input);
}

/**
 * @brief Generic load command.
 *
 * @param[in] from_file - Filename.
 */
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

/**
 * @brief Save into default file.
 *
 */
void save_default() {
	save(default_profile);
}

/**
 * @brief Save into another file.
 *
 */
void save_as() {
	std::cout << "Input filename: ";

	std::string input;
	std::getline(std::cin, input);

	save(input);
}

/**
 * @brief Generic save calibration.
 *
 * @param[in] to_file - Output file.
 */
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

/**
 * @brief Calibrate servo.
 *
 */
void calibrate_servo() {
	auto current = car_profile.get_servo();
	profile::servo new_profile = {90, 90, 90};
	raw_tty();

	// Print current values
	if (current.has_value()) {
		std::cout << "Current values:\n\n";
		std::cout << "Left: " << current->max_left << '\n';
		std::cout << "Center: " << current->center << '\n';
		std::cout << "Right: " << current->max_right << '\n';

		new_profile = current.value();
	} else {
		std::cout << "No calibration data\n";
	}

	servo test_servo(gpio_pins, RASPI_32);

	// Calibrate center point
	std::cout << "\nStarting center value calibration...\n";
	auto selection = servo_value_select(test_servo, new_profile.center);
	if (selection.has_value()) {
		new_profile.center = selection.value();
	} else {
		restore_tty();
		prompt_enter();
		return;
	}

	// Calibrate left point
	std::cout << "\nStarting left value calibration...\n";
	selection = servo_value_select(test_servo, new_profile.max_left);
	if (selection.has_value()) {
		new_profile.max_left = selection.value();
	} else {
		restore_tty();
		prompt_enter();
		return;
	}

	// Calibrate right point
	std::cout << "\nStarting right value calibration...\n";
	selection = servo_value_select(test_servo, new_profile.max_right);
	if (selection.has_value()) {
		new_profile.max_right = selection.value();
	} else {
		restore_tty();
		prompt_enter();
		return;
	}

	std::cout << "\nNew values:\n\n";
	std::cout << "Left: " << new_profile.max_left << '\n';
	std::cout << "Center: " << new_profile.center << '\n';
	std::cout << "Right: " << new_profile.max_right << '\n';

	car_profile.set_servo(new_profile);

	restore_tty();
	prompt_enter();
}

/**
 * @brief Select value for a servo setting.
 *
 * @param[in] value - Starting value.
 * @return Final value.
 */
std::optional<uint32_t> servo_value_select(servo &servo, uint32_t value) {
	std::cout << "'a' - left, 'd' - right, 's' - set, 'q' - exit" << std::endl;

	while (true) {
		servo.set(value);
		printf("Current value: %03u", value);
		int input = std::getchar();

		if (input == 'a' && value > 0) {
			value--;
		}
		if (input == 'd' && value < 180) {
			value++;
		}
		if (input == 's') {
			std::cout << '\n';
			return value;
		}
		if (input == 'q') {
			break;
		}

		std::cout << '\r';
	}

	std::cout << '\n';
	return std::nullopt;
}
