/**
 * @file src/calibrate.cpp
 * @brief Calibrate tasks.
 */
#include "calibrate.hpp"

#include <chrono>
#include <future>
#include <iostream>
#include <vector>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/pinmap.hpp>
#include <shared/menu.hpp>
#include <shared/tdma.hpp>
#include <shared/utils.hpp>

static void assist_calibrate();

static const std::vector<menu_item> calib_options = {
	{.text = "Assist TDMA calibration", .action = &assist_calibrate}};

void calibration_submenu() {
	show_menu("Calibration", calib_options, true);
}

void assist_calibrate() {
	auto rf_module =
		std::make_shared<drf7020d20>(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0);

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

	std::atomic<bool> finish;
	auto exit_future = std::async(std::launch::async, [&finish]() {
		std::getchar();
		finish = true;
	});

	raw_tty();
	std::cout << "Starting assiting...\n";
	std::cout << "Hit any key to exit\n";
	while (!finish) {
		auto request = rf_module->receive(std::chrono::milliseconds(50));
		if (request.empty()) {
			continue;
		}

		rf_module->transmit(std::to_string(generate_ms() - 13));
		std::cout << "Hit\n";
	}

	restore_tty();
}
