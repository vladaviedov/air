/**
 * @file src/calibrate.cpp
 * @brief Car sensor and drive calibration.
 */
#include "calibrate.hpp"

#include <chrono>
#include <cstdint>
#include <cstdio>
#include <exception>
#include <iostream>
#include <optional>
#include <string>
#include <thread>
#include <vector>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/hcsr04.hpp>
#include <driver/lightsens.hpp>
#include <driver/motors.hpp>
#include <driver/pinmap.hpp>
#include <driver/servo.hpp>
#include <shared/menu.hpp>
#include <shared/tdma.hpp>
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

static void calibrate_us();

static void calibrate_tdma();

static void calibrate_turning();
static void calibrate_position();

static const std::vector<menu_item> calib_options = {
	{.text = "Print calibration", .action = &print_calibration},
	{.text = "Calibrate servo", .action = &calibrate_servo},
	{.text = "Calibrate ultrasonic", .action = &calibrate_us},
	{.text = "Calibrate TDMA", .action = &calibrate_tdma},
	{.text = "Calibrate position", .action = &calibrate_position},
	{.text = "Calibrate turning", .action = &calibrate_turning},
	{.text = "Reload default profile", .action = &load_default},
	{.text = "Load profile from...", .action = &load_from},
	{.text = "Save default profile", .action = &save_default},
	{.text = "Save profile as...", .action = &save_as}};

static constexpr uint32_t ADJUSTMENT = 700;
static constexpr uint32_t SIDE_ADJUSTMENT = 100;

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
	auto us_profile = car_profile.get_us();
	auto turn_profile = car_profile.get_turn();

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
		std::cout << '\n';
	} else {
		std::cout << "TDMA: not defined\n";
	}

	if (us_profile.has_value()) {
		std::cout << "Ultrasonic:\n\n";
		std::cout << "Value threshold: " << us_profile->threshold << '\n';
		std::cout << '\n';
	} else {
		std::cout << "Ultrasonic: not defined\n";
	}

	if (turn_profile.has_value()) {
		std::cout << "Turning:\n\n";
		std::cout << "Right duration: " << turn_profile->right_ms << " ms\n";
		std::cout << "Right delay: " << turn_profile->right_delay_ms << " ms\n";
		std::cout << "Left duration: " << turn_profile->left_ms << " ms\n";
		std::cout << "Left delay: " << turn_profile->left_delay_ms << " ms\n";
	} else {
		std::cout << "Turning: not defined\n";
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

/**
 * @brief Calibrate ultrasonic sensor.
 *
 */
void calibrate_us() {
	auto current = car_profile.get_us();
	profile::us new_profile = {0};
	raw_tty();

	// Print current values
	if (current.has_value()) {
		std::cout << "Current value: " << current->threshold << '\n';
		new_profile = current.value();
	} else {
		std::cout << "No calibration data\n";
	}

	hc_sr04 sensor(gpio_pins, RASPI_29, RASPI_31);

	uint32_t order = 0;
	uint32_t increment = 1;

	std::cout << "\nStarting threshold value calibration...\n";
	std::cout << "'w' - increment, 's' - decrement\n"
			  << "'a' - lesser increment, 'd' - greater increment\n"
			  << "'e' - finish calibration, ' ' - update\n";

	printf("%10s %4s %10s %6s\n", "Threshold", "Increment", "Value", "Status");
	while (true) {
		auto reading = sensor.pulse();
		// NOLINTNEXTLINE: clang is being silly
		printf("%10u 10^%u %10llu %s", new_profile.threshold, order, reading,
			reading < new_profile.threshold ? "detected    " : "not detected");
		int input = std::getchar();
		if (input == 'e') {
			break;
		}

		switch (input) {
		case 'w':
			new_profile.threshold += increment;
			break;
		case 's':
			if (new_profile.threshold <= increment) {
				new_profile.threshold = 0;
			} else {
				new_profile.threshold -= increment;
			}
			break;
		case 'a':
			if (order > 0) {
				order--;
				increment /= 10;
			}
			break;
		case 'd':
			if (order < 9) {
				order++;
				increment *= 10;
			}
			break;
		}

		std::putchar('\r');
	}

	std::cout << "\nNew values:\n\n";
	std::cout << "Threshold: " << new_profile.threshold << '\n';

	car_profile.set_us(new_profile);

	restore_tty();
	prompt_enter();
}

void calibrate_tdma() {
	auto rf_module = std::make_shared<drf7020d20>(
		gpio_pins, RASPI_12, RASPI_16, RASPI_18, 0);

	// Configure RF Chip
	std::cout << "Configuring RF Chip... " << std::flush;
	rf_module->enable();
	if (!rf_module->configure(FREQ_CALIBRATION, drf7020d20::DR9600, 9,
			drf7020d20::DR9600, drf7020d20::NONE)) {
		std::cout << "Failed\n";
		prompt_enter();
		return;
	}
	std::cout << "Success\n";

	auto current = car_profile.get_tdma();
	profile::tdma new_profile = {0, -5};

	// Print current values
	if (current.has_value()) {
		std::cout << "Current values:\n\n";
		std::cout << "TX: " << current->tx_offset_ms << '\n';
		std::cout << "RX: " << current->rx_offset_ms << '\n';
		new_profile = current.value();
	} else {
		std::cout << "No calibration data\n";
	}

	tdma calib_slot(rf_module, 0, tdma::AIR_A);
	calib_slot.rx_set_offset(new_profile.rx_offset_ms);

	raw_tty();
	std::cout
		<< "Ready to calibrate. Put control into calibration assistance.\n";
	std::cout << "Hit space to continue, 's' to exit\n";

	while (true) {
		int input = std::getchar();
		if (input == 's') {
			restore_tty();
			prompt_enter();
			return;
		}
		if (input == ' ') {
			restore_tty();
			break;
		}
	}

	std::cout << "Starting calibration...\n";

	int32_t okay_msgs = 0;
	while (okay_msgs < 5) {
		calib_slot.tx_set_offset(new_profile.tx_offset_ms);

		int32_t sent = calib_slot.tx_ts_sync();
		std::cout << sent << '\n';
		auto response = rf_module->receive(std::chrono::milliseconds(500));
		if (response.empty()) {
			std::cout << "Calibration failure: helper did not respond\n";
			prompt_enter();
			return;
		}

		int32_t res = std::stoi(response);
		std::cout << res << '\n';
		int32_t diff = res - sent;

		// Adjust if second rollover
		if (std::abs(diff) > 500) {
			res = res + 1000;
			diff = res - sent;
		}

		std::cout << "Error: " << diff << "ms\n";
		if (std::abs(diff) < 5) {
			okay_msgs++;
		} else {
			okay_msgs = 0;
			new_profile.tx_offset_ms -= diff;
		}
	}

	std::cout << "Calibration completed.\n";

	std::cout << "\nNew values:\n\n";
	std::cout << "TX: " << new_profile.tx_offset_ms << '\n';
	std::cout << "RX: " << new_profile.rx_offset_ms << '\n';

	car_profile.set_tdma(new_profile);

	prompt_enter();
}

/**
 * @brief Calibrate turning timings.
 *
 */
void calibrate_turning() {
	auto servo_profile = car_profile.get_servo();
	if (!servo_profile.has_value()) {
		std::cout << "No servo calibration data. Exiting...\n";
		prompt_enter();
		return;
	}

	auto current = car_profile.get_turn();
	profile::turn new_profile = {0, 0, 0, 0};
	raw_tty();

	// Print current values
	if (current.has_value()) {
		std::cout << "Current values:\n\n";
		std::cout << "Right duration: " << current->right_ms << "ms\n";
		std::cout << "Right delay: " << current->right_delay_ms << "ms\n";
		std::cout << "Left duration: " << current->left_ms << "ms\n";
		std::cout << "Left delay: " << current->left_delay_ms << "ms\n";

		new_profile = current.value();
	} else {
		std::cout << "No calibration data\n";
	}

	motor motors(gpio_pins, RASPI_15, RASPI_13, RASPI_11);
	servo servo_m(gpio_pins, RASPI_32);
	light_sens ir_left(gpio_pins, RASPI_22);
	light_sens ir_right(gpio_pins, RASPI_24);

	// Calibrate right turn
	std::cout << "\nHit any key to begin right turn calibration.\n";
	std::cout << "You will need to hit a key again once the car does a 90 "
				 "degree right turn\n";
	std::getchar();

	servo_m.set(servo_profile->max_right);
	motors.set(100, FORWARD);

	// Measure time
	auto start_time = std::chrono::system_clock::now();
	std::getchar();
	auto end_time = std::chrono::system_clock::now();
	motors.stop();
	new_profile.right_ms = (end_time - start_time).count() / 1000000 - (ADJUSTMENT - SIDE_ADJUSTMENT);
	servo_m.set(servo_profile->center);
	std::cout << "Right turn calibration completed!\n";

	// Calibrate left offset
	std::cout << "\nPlace the car at an intersection exit.\n";
	std::cout << "Hit any key to begin right delay calibration.\n";
	std::getchar();

	servo_m.set(servo_profile->max_right);
	motors.set(100, BACKWARD);
	std::this_thread::sleep_for(std::chrono::milliseconds(new_profile.right_ms));
	servo_m.set(servo_profile->center);

	// Measure time
	start_time = std::chrono::system_clock::now();
	while (true) {
		if (ir_left.read() && ir_right.read()) {
			break;
		}
	}
	end_time = std::chrono::system_clock::now();
	motors.stop();
	new_profile.right_delay_ms = (end_time - start_time).count() / 1000000 - (ADJUSTMENT - SIDE_ADJUSTMENT);
	std::cout << "Right turn delay calibration completed!\n";

	// Calibrate left turn
	std::cout << "\nHit any key to begin left turn calibration.\n";
	std::cout << "You will need to hit a key again once the car does a 90 "
				 "degree left turn\n";
	std::getchar();

	servo_m.set(servo_profile->max_left);
	motors.set(100, FORWARD);

	// Measure time
	start_time = std::chrono::system_clock::now();
	std::getchar();
	end_time = std::chrono::system_clock::now();
	motors.stop();
	new_profile.left_ms = (end_time - start_time).count() / 1000000 - (ADJUSTMENT + SIDE_ADJUSTMENT);
	servo_m.set(servo_profile->center);
	std::cout << "Left turn calibration completed!\n";

	// Calibrate left offset
	std::cout << "\nPlace the car at an intersection exit.\n";
	std::cout << "Hit any key to begin left delay calibration.\n";
	std::getchar();

	servo_m.set(servo_profile->max_left);
	motors.set(100, BACKWARD);
	std::this_thread::sleep_for(std::chrono::milliseconds(new_profile.left_ms));
	servo_m.set(servo_profile->center);

	// Measure time
	start_time = std::chrono::system_clock::now();
	while (true) {
		if (ir_left.read() && ir_right.read()) {
			break;
		}
	}
	end_time = std::chrono::system_clock::now();
	motors.stop();
	new_profile.left_delay_ms = (end_time - start_time).count() / 1000000 - (ADJUSTMENT + SIDE_ADJUSTMENT);
	std::cout << "Left turn delay calibration completed!\n";

	std::cout << "Calibration completed.\n";

	std::cout << "\nNew values:\n\n";
	std::cout << "Right duration: " << new_profile.right_ms << '\n';
	std::cout << "Right delay: " << new_profile.right_delay_ms << '\n';
	std::cout << "Left duration: " << new_profile.left_ms << '\n';
	std::cout << "Left delay: " << new_profile.left_delay_ms << '\n';

	car_profile.set_turn(new_profile);

	restore_tty();
	prompt_enter();
}

void calibrate_position() {
	std::cout << "Enter the initial position: " << std::flush;
	std::cin >> position;
}
