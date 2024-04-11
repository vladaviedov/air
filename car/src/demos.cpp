/**
 * @file src/demos.cpp
 * @brief Car demos.
 */
#include "demos.hpp"

#include <atomic>
#include <chrono>
#include <cstdlib>
#include <future>
#include <iostream>
#include <memory>
#include <thread>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/lightsens.hpp>
#include <driver/motors.hpp>
#include <driver/pinmap.hpp>
#include <driver/servo.hpp>
#include <shared/menu.hpp>
#include <shared/messages.hpp>
#include <shared/tdma.hpp>
#include <shared/utils.hpp>

#include "common.hpp"
#include "messageworker.hpp"

static void tdma_slots();

static void manual_drive();
static void manual_drive_wasd();
static void simple_lines();
static void message_worker_test();
static void turning();

static const std::vector<menu_item> demos = {
	{.text = "TDMA slots", .action = &tdma_slots},
	{.text = "Manual drive", .action = &manual_drive},
	{.text = "Manual drive WASD", .action = &manual_drive_wasd},
	{.text = "Simple line following", .action = &simple_lines},
	{.text = "Message worker", .action = &message_worker_test},
	{.text = "Turning", .action = &turning},
	{.text = "Simple line following", .action = &simple_lines}};

void demo_submenu() {
	show_menu("Car Demos", demos, true);
}

/**
 * @brief Send ID on a specified TDMA slot.
 *
 */
void tdma_slots() {
	auto tdma_profile = car_profile.get_tdma();
	if (!tdma_profile.has_value()) {
		std::cout << "TDMA not configured. Exiting...\n";
		prompt_enter();
		return;
	}

	auto rf_module = std::make_shared<drf7020d20>(
		gpio_pins, RASPI_12, RASPI_16, RASPI_18, 0);

	// Configure RF Chip
	std::cout << "Configuring RF Chip... " << std::flush;
	rf_module->enable();
	if (!rf_module->configure(FREQ_DEMO, drf7020d20::DR9600, 9,
			drf7020d20::DR9600, drf7020d20::NONE)) {
		std::cout << "Failed\n";
		prompt_enter();
		return;
	}
	std::cout << "Success\n";

	// Select scheme
	std::string input;
	tdma::scheme selected_scheme = tdma::AIR_A;
	uint32_t n_slots = 4;

	std::cout << "AIR TDMA Scheme (default - A): ";
	std::getline(std::cin, input);
	if (!input.empty()) {
		switch (input[0]) {
		case 'A': // fallthrough
		case 'a':
			// Already set
			break;
		case 'B': // fallthrough
		case 'b':
			selected_scheme = tdma::AIR_B;
			n_slots = 8;
			break;
		case 'C': // fallthrough
		case 'c':
			selected_scheme = tdma::AIR_C;
			n_slots = 16;
			break;
		default:
			std::cout << "Unknown scheme\n";
			prompt_enter();
			return;
		}
	}

	// Select timeslot
	uint32_t slot = 0;
	printf("Timeslot [0-%u] (default - 0): ", n_slots - 1);
	std::getline(std::cin, input);
	if (!input.empty()) {
		try {
			int32_t slot_input = std::stoi(input);
			if (slot_input < 0 || (uint32_t)slot_input >= n_slots) {
				std::cout << "Invalid Timeslot\n";
				prompt_enter();
				return;
			}

			slot = (uint32_t)slot_input;
		} catch (std::exception &ex) {
			std::cout << "Invalid Timeslot\n";
			std::cerr << ex.what() << std::endl;

			prompt_enter();
			return;
		}
	}

	tdma tdma_slot(rf_module, slot, selected_scheme);
	tdma_slot.rx_set_offset(-5);
	tdma_slot.tx_set_offset(-70);

	raw_tty();

	// Set up exit condition
	std::atomic<bool> finish = false;
	auto exit_future = std::async(std::launch::async, [&finish]() {
		std::getchar();
		finish = true;
	});

	std::cout << "\nStarting demo.\n";
	std::cout << "Hit the space key at any time to exit.\n\n";
	const auto &car_id = get_id();
	while (!finish) {
		std::cout << "Sending: " << *car_id << '\n';
		tdma_slot.tx_sync(*get_id());

		auto response = tdma_slot.rx_sync(5);
		if (!response.empty()) {
			std::cout << "Received: " << response << '\n';
		} else {
			std::cout << "No response" << '\n';
		}
	}

	restore_tty();
}

void manual_drive() {
	auto servo_profile = car_profile.get_servo();
	if (!servo_profile.has_value()) {
		std::cout << "No servo calibration data. Exiting...\n";
		prompt_enter();
		return;
	}

	// Init hardware
	motor m_1(gpio_pins, RASPI_15, RASPI_13, RASPI_11);
	motor m_2(gpio_pins, RASPI_33, RASPI_35, RASPI_37);
	servo m_sv(gpio_pins, RASPI_32);

	raw_tty();

	std::cout << "Hardware initialized.\n";
	std::cout << "Starting manual control.\n\n";

	printf("%-25s %-25s\n", "Directional controls", "Speed controls");
	printf("%-25s %-25s\n", "q w e", "u");
	printf("%-25s\n", "  s  ");
	printf("%-25s %-25s\n", "z x c", "j");
	printf("\nHit space to exit\n");

	uint32_t speed = 0;
	uint32_t servo = servo_profile->center;
	direction dir = FORWARD;

	while (true) {
		// Set motors
		if (speed == 0) {
			m_1.stop();
			m_2.stop();

			printf("Current speed: stp; ");
		} else {
			m_1.set((float)speed, dir);
			m_2.set((float)speed, dir);

			printf("Current speed: %03u; ", speed);
		}

		// Set servo
		m_sv.set(servo);
		printf("Direction: %s ", dir == FORWARD ? "forw" : "back");
		if (servo == servo_profile->max_left) {
			printf("%-20s", "left  ");
		} else if (servo == servo_profile->center) {
			printf("%-20s", "center");
		} else {
			printf("%-20s", "right");
		}

		int input = std::getchar();

		switch (input) {
		case ' ':
			restore_tty();
			return;
		case 'q':
			dir = FORWARD;
			servo = servo_profile->max_left;
			break;
		case 'w':
			dir = FORWARD;
			servo = servo_profile->center;
			break;
		case 'e':
			dir = FORWARD;
			servo = servo_profile->max_right;
			break;
		case 's':
			speed = 0;
			break;
		case 'z':
			dir = BACKWARD;
			servo = servo_profile->max_left;
			break;
		case 'x':
			dir = BACKWARD;
			servo = servo_profile->center;
			break;
		case 'c':
			dir = BACKWARD;
			servo = servo_profile->max_right;
			break;
		case 'u':
			if (speed < 100) {
				speed += 10;
			}
			break;
		case 'j':
			if (speed > 0) {
				speed -= 10;
			}
			break;
		}

		std::cout << '\r';
	}
}

void simple_lines() {
	auto servo_profile = car_profile.get_servo();
	if (!servo_profile.has_value()) {
		std::cout << "No servo calibration data. Exiting...\n";
		prompt_enter();
		return;
	}

	// Init hardware
	motor m_1(gpio_pins, RASPI_15, RASPI_13, RASPI_11);
	motor m_2(gpio_pins, RASPI_33, RASPI_35, RASPI_37);
	servo m_sv(gpio_pins, RASPI_32);
	light_sens ir_left(gpio_pins, RASPI_22);
	light_sens ir_right(gpio_pins, RASPI_24);

	raw_tty();

	std::cout << "Hardware initialized.\n";
	std::cout << "Starting line following.\n";
	std::cout << "Hit the space key at any time to exit.\n";

	m_1.set(100, FORWARD);
	m_2.set(100, FORWARD);
	m_sv.set(servo_profile->center);

	std::atomic<bool> finish = false;
	auto exit_future = std::async(std::launch::async, [&finish]() {
		std::getchar();
		finish = true;
	});

	while (!finish) {
		bool left_reading = ir_left.read();
		bool right_reading = ir_right.read();

		if (left_reading && right_reading) {
			m_1.stop();
			m_2.stop();

			std::cout << "Car is off-road.\n";
			prompt_enter();
			restore_tty();
			return;
		}

		if (left_reading) {
			m_sv.set(servo_profile->max_right);
		} else if (right_reading) {
			m_sv.set(servo_profile->max_left);
		} else {
			m_sv.set(servo_profile->center);
		}

		std::this_thread::sleep_for(std::chrono::milliseconds(10));
	}

	restore_tty();
}

void manual_drive_wasd() {
	auto servo_profile = car_profile.get_servo();
	if (!servo_profile.has_value()) {
		std::cout << "No servo calibration data. Exiting...\n";
		prompt_enter();
		return;
	}

	// Init hardware
	motor m_1(gpio_pins, RASPI_15, RASPI_13, RASPI_11);
	motor m_2(gpio_pins, RASPI_33, RASPI_35, RASPI_37);
	servo m_sv(gpio_pins, RASPI_32);

	raw_tty();

	std::cout << "Hardware initialized.\n";
	std::cout << "Starting manual control.\n\n";

	std::cout << "Controls: WASD; Hit E to stop the car.\n";
	std::cout << "\nHit space to exit\n";

	int32_t speed = 0;
	uint32_t servo = servo_profile->center;

	while (true) {
		// Set motors
		if (speed == 0) {
			m_1.stop();
			m_2.stop();

			printf("Current speed: stp; ");
		} else {
			auto dir = speed > 0 ? FORWARD : BACKWARD;
			uint32_t speed_abs = std::abs(speed);

			m_1.set((float)speed_abs, dir);
			m_2.set((float)speed_abs, dir);

			printf("Current speed: %03u; ", speed_abs);
		}

		// Set servo
		m_sv.set(servo);

		auto offset = (int32_t)servo - (int32_t)servo_profile->center;
		printf("Servo offset: %+2d ", offset);

		int input = std::getchar();

		switch (input) {
		case ' ':
			restore_tty();
			return;
		case 'w':
			speed += 10;
			break;
		case 's':
			speed -= 10;
			break;
		case 'a':
			servo -= 10;
			break;
		case 'd':
			servo += 10;
			break;
		case 'e':
			speed = 0;
			break;
		}

		// Don't go over max values
		if (servo > servo_profile->max_right) {
			servo = servo_profile->max_right;
		}
		if (servo < servo_profile->max_left) {
			servo = servo_profile->max_left;
		}
		if (speed > 100) {
			speed = 100;
		}
		if (speed < -100) {
			speed = -100;
		}

		std::cout << '\r';
	}
}

void message_worker_test() {
	auto tdma_profile = car_profile.get_tdma();
	if (!tdma_profile.has_value()) {
		std::cout << "TDMA not configured. Exiting...\n";
		prompt_enter();
		return;
	}

	auto rf_module = std::make_shared<drf7020d20>(
		gpio_pins, RASPI_12, RASPI_16, RASPI_18, 0);

	// Configure RF Chip
	std::cout << "Configuring RF Chip... " << std::flush;
	rf_module->enable();
	if (!rf_module->configure(FREQ_DEMO, drf7020d20::DR9600, 9,
			drf7020d20::DR9600, drf7020d20::NONE)) {
		std::cout << "Failed\n";
		prompt_enter();
		return;
	}
	std::cout << "Success\n";

	// Select scheme
	std::string input;
	tdma::scheme selected_scheme = tdma::AIR_A;
	uint32_t n_slots = 4;

	std::cout << "AIR TDMA Scheme (default - A): ";
	std::getline(std::cin, input);
	if (!input.empty()) {
		switch (input[0]) {
		case 'A': // fallthrough
		case 'a':
			// Already set
			break;
		case 'B': // fallthrough
		case 'b':
			selected_scheme = tdma::AIR_B;
			n_slots = 8;
			break;
		case 'C': // fallthrough
		case 'c':
			selected_scheme = tdma::AIR_C;
			n_slots = 16;
			break;
		default:
			std::cout << "Unknown scheme\n";
			prompt_enter();
			return;
		}
	}

	// Select timeslot
	uint32_t slot = 0;
	printf("Timeslot [0-%u] (default - 0): ", n_slots - 1);
	std::getline(std::cin, input);
	if (!input.empty()) {
		try {
			int32_t slot_input = std::stoi(input);
			if (slot_input < 0 || (uint32_t)slot_input >= n_slots) {
				std::cout << "Invalid Timeslot\n";
				prompt_enter();
				return;
			}

			slot = (uint32_t)slot_input;
		} catch (std::exception &ex) {
			std::cout << "Invalid Timeslot\n";
			std::cerr << ex.what() << std::endl;

			prompt_enter();
			return;
		}
	}

	auto tdma_slot = std::make_shared<tdma>(rf_module, slot, selected_scheme);
	tdma_slot->rx_set_offset(tdma_profile->rx_offset_ms);
	tdma_slot->tx_set_offset(tdma_profile->tx_offset_ms);

	message_worker worker(tdma_slot);

	std::cout << "Enter the desired position: \n";
	std::getline(std::cin, input);
	uint8_t desired_pos = std::stoi(input);

	std::cout << "Awaiting checkin...\n";
	auto control_id = worker.send_checkin();
	if (!control_id.has_value()) {
		std::cout << "Control id not received. Cancelling...";
		prompt_enter();
		return;
	}
	std::cout << "Control Id: " << control_id.value() << std::endl;

	std::cout << "Sending request to " << desired_pos << " ...\n";
	message_worker::command command = worker.send_request(desired_pos);

	std::this_thread::sleep_for(std::chrono::milliseconds(10));

	worker.send_clear();

	if (command == message_worker::SBY) {
		std::cout << "STANDBY\n";
	} else if (command == message_worker::GRQ) {
		std::cout << "GO AS REQUESTED\n";
	}
}

/**
 * @brief Try a turn.
 *
 */
void turning() {
	auto servo_profile = car_profile.get_servo();
	auto turn_profile = car_profile.get_turn();
	if (!servo_profile.has_value()) {
		std::cout << "No servo calibration data. Exiting...\n";
		prompt_enter();
		return;
	}
	if (!turn_profile.has_value()) {
		std::cout << "No turning calibration data. Exiting...\n";
		prompt_enter();
		return;
	}

	// Init hardware
	motor m_1(gpio_pins, RASPI_15, RASPI_13, RASPI_11);
	motor m_2(gpio_pins, RASPI_33, RASPI_35, RASPI_37);
	servo m_sv(gpio_pins, RASPI_32);

	std::cout << "Hardware initialized.\n";
	std::cout << "Hit 'a' to turn left or 'd' to turn right.\n";
	std::cout << "Hit space to exit\n'";

	raw_tty();
	while (true) {
		int input = std::getchar();

		if (input == 'a') {
			std::cout << "Starting left turn...\n";
			m_1.set(100, FORWARD);
			m_2.set(100, FORWARD);
			m_sv.set(servo_profile->center);

			std::this_thread::sleep_for(
				std::chrono::milliseconds(turn_profile->left_delay_ms));
			m_sv.set(servo_profile->max_left);
			std::this_thread::sleep_for(
				std::chrono::milliseconds(turn_profile->left_ms));

			m_sv.set(servo_profile->center);
			m_1.stop();
			m_2.stop();
			std::cout << "Left turn finished\n";
		}
		if (input == 'd') {
			std::cout << "Starting right turn...\n";
			m_1.set(100, FORWARD);
			m_2.set(100, FORWARD);
			m_sv.set(servo_profile->center);

			std::this_thread::sleep_for(
				std::chrono::milliseconds(turn_profile->right_delay_ms));
			m_sv.set(servo_profile->max_right);
			std::this_thread::sleep_for(
				std::chrono::milliseconds(turn_profile->right_ms));

			m_sv.set(servo_profile->center);
			m_1.stop();
			m_2.stop();
			std::cout << "Right turn finished\n";
		}
		if (input == ' ') {
			break;
		}
	}

	restore_tty();
	prompt_enter();
}
