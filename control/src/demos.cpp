/**
 * @file src/demos.cpp
 * @brief Control demos.
 */
#include "demos.hpp"

#include <algorithm>
#include <atomic>
#include <iostream>
#include <memory>
#include <sstream>
#include <vector>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/pinmap.hpp>
#include <shared/menu.hpp>
#include <shared/messages.hpp>
#include <shared/tdma.hpp>
#include <shared/utils.hpp>

#include "controller.hpp"
#include "messageworker.hpp"

static void tdma_control();
static void control_template(std::function<void(std::shared_ptr<drf7020d20>,
		uint32_t,
		tdma::scheme,
		std::atomic<bool> &)> inner_func);
static void message_worker_test();

static const std::vector<menu_item> demos = {
	{.text = "TDMA control", .action = &tdma_control},
	{.text = "Message worker", .action = &message_worker_test}};

void demo_submenu() {
	show_menu("Control Demos", demos, true);
}

/**
 * @brief TDMA multi-slot control demo.
 *
 */
void control_template(std::function<void(std::shared_ptr<drf7020d20>,
		uint32_t,
		tdma::scheme,
		std::atomic<bool> &)> inner_func) {
	auto rf_test =
		std::make_shared<drf7020d20>(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0);

	// Configure RF Chip
	std::cout << "Configuring RF Chip... " << std::flush;
	rf_test->enable();
	if (!rf_test->configure(FREQ_DEMO, drf7020d20::DR9600, 9,
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

	std::cout << "Which timeslots to monitor? (space separated): ";
	std::getline(std::cin, input);
	std::istringstream tokens(input);

	std::vector<uint32_t> slots;
	uint32_t buffer;

	while (tokens >> buffer) {
		if (buffer < n_slots) {
			slots.push_back(buffer);
		} else {
			std::cout << "Invalid timeslot: " << buffer << ". Allowed: 0-"
					  << n_slots - 1 << ".\n";
			prompt_enter();
			return;
		}
	}

	// Remove duplicates
	std::sort(slots.begin(), slots.end());
	auto iter = std::unique(slots.begin(), slots.end());
	slots.erase(iter, slots.end());

	// For proper destruction
	std::cout << "Starting to monitor. Hit space to exit\n";

	// Executor function
	std::atomic<bool> active = true;
	auto executor = [&](uint32_t slot) {
		inner_func(rf_test, slot, selected_scheme, active);
	};

	std::vector<std::thread> threads;
	threads.reserve(slots.size());
	for (auto &slot : slots) {
		threads.push_back(std::thread(executor, slot));
	}

	raw_tty();
	while (std::getchar() != ' ') {
	}

	std::cout << "Stopping monitoring...\n";
	active = false;
	for (auto &thread : threads) {
		thread.join();
	}

	restore_tty();
}

void tdma_control() {
	auto inner_func = [](const std::shared_ptr<drf7020d20> &rf_test,
						  uint32_t slot, tdma::scheme selected_scheme,
						  std::atomic<bool> &active) {
		tdma tdma(rf_test, slot, selected_scheme);
		tdma.rx_set_offset(-5);
		tdma.tx_set_offset(-70);

		while (active) {
			auto received = tdma.rx_sync(5);
			if (!received.empty()) {
				std::cout << "Slot " << slot << ": " << received << '\n';
				tdma.tx_sync(*get_id());
			}
		}
	};

	control_template(inner_func);
}

void message_worker_test() {
	auto inner_func = [](const std::shared_ptr<drf7020d20> &rf_module,
						  uint32_t slot, tdma::scheme selected_scheme,
						  std::atomic<bool> &active) {
		auto tdma_slot =
			std::make_shared<tdma>(rf_module, slot, selected_scheme);
		tdma_slot->rx_set_offset(-5);
		tdma_slot->tx_set_offset(-70);

		while (active) {
			message_worker worker(
				tdma_slot, std::make_shared<std::atomic<bool>>(&active));

			std::cout << "Awaiting check-in...\n";
			auto request_data = worker.await_request_sync();
			if (!request_data.has_value()) {
				std::cout << "No data received. Cancelled...\n";
				break;
			}

			std::cout << "Received check-in:\n\n";
			printf("Current Position: %u\n", std::get<0>(request_data.value()));
			printf("Desired Position: %u\n", std::get<1>(request_data.value()));
			printf("Car Id: %s\n", std::get<2>(request_data.value()).c_str());

			std::cout << "Processing request...\n";
			worker.send_go_requested();

			bool receieved_ack = worker.check_acknowledge_sync();
			std::cout << "Received ACKNOWLEDGE: " << receieved_ack << std::endl;

			std::cout << "Awaiting clear from car...\n";
			bool sent_clear = worker.await_clear_sync();

			std::cout << "Sent clear: " << sent_clear << std::endl;
		}
	};

	control_template(inner_func);
}
