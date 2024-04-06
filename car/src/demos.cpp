/**
 * @file src/demos.cpp
 * @brief Car demos.
 */
#include "demos.hpp"

#include <atomic>
#include <future>
#include <iostream>
#include <memory>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/motors.hpp>
#include <driver/pinmap.hpp>
#include <driver/servo.hpp>
#include <shared/menu.hpp>
#include <shared/messages.hpp>
#include <shared/tdma.hpp>
#include <shared/utils.hpp>

#include "common.hpp"

static void tdma_slots();
static void manual_drive();

static const std::vector<menu_item> demos = {
	{.text = "TDMA slots", .action = &tdma_slots},
	{.text = "Manual drive", .action = &manual_drive}};

void demo_submenu() {
	show_menu("Car Demos", demos, true);
}

/**
 * @brief Send ID on a specified TDMA slot.
 *
 */
void tdma_slots() {
	/* auto rf_test = std::make_shared<drf7020d20>(gpio_pins, RASPI_40,
	 * RASPI_37, RASPI_38, 0); */
	auto rf_test = std::make_shared<drf7020d20>(
		gpio_pins, RASPI_12, RASPI_16, RASPI_18, 0);

	// Configure RF Chip
	std::cout << "Configuring RF Chip... " << std::flush;
	rf_test->enable();
	if (!rf_test->configure(433900, drf7020d20::DR9600, 9, drf7020d20::DR9600,
			drf7020d20::NONE)) {
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

	tdma tdma_slot(rf_test, slot, selected_scheme);
	tdma_slot.rx_set_offset(-5);
	tdma_slot.tx_set_offset(-70);

	// Set up exit condition
	std::atomic<bool> finish = false;
	auto exit_future = std::async(std::launch::async, [&finish]() {
		std::getchar();
		finish = true;
	});

	std::cout << "\nStarting demo. Hit the enter key at any time to exit.\n\n";
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

	uint32_t speed = 0.0F;
	uint32_t servo = servo_profile->center;
	direction dir = FORWARD;

	while (true) {
		if (speed == 0) {
			m_1.stop();
			m_2.stop();

			printf("Current speed: stp; ");
		} else {
			m_1.set((float)speed, dir);
			m_2.set((float)speed, dir);
			m_sv.set(servo);

			printf("Current speed: %03u; ", speed);
		}

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
