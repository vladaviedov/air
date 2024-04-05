/**
 * @file src/demos.cpp
 * @brief Car demos.
 */
#include "demos.hpp"

#include <atomic>
#include <cstdlib>
#include <future>
#include <iostream>
#include <memory>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/pinmap.hpp>
#include <shared/menu.hpp>
#include <shared/messages.hpp>
#include <shared/tdma.hpp>

static void prompt_enter();
static void tdma_slots();

static const std::vector<menu_item> demos = {
	{.text = "TDMA Slots", .action = &tdma_slots}};

void demo_submenu() {
	show_menu("Car Demos", demos, true);
}

/**
 * @brief Prompt user to press any key.
 *
 */
void prompt_enter() {
	std::cout << "\nPress enter to exit." << std::flush;
	std::getchar();
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
