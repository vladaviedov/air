/**
 * @file src/air.cpp
 * @brief The real control code.
 */
#include "air.hpp"

#include <atomic>
#include <iostream>
#include <thread>

#include "controller.hpp"

constexpr uint8_t INTERSECTION_SIZE = 4;

static void control_logic();

// NOLINTBEGIN: state flags
static std::atomic<bool> active = true;

// NOLINTEND

void run_air() {
	auto control_thread = std::thread(control_logic);
}

void control_logic() {
	controller control(INTERSECTION_SIZE, tdma::scheme::AIR_A);

	while (active) {
		control.process_requests();
	}
}
