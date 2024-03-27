#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

#include <gpiod.hpp>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/pinmap.hpp>
#include <shared/messages.hpp>

// NOLINTBEGIN: temporary code
#define CALLSIGN "?"
#define TAG "/2"

// NOLINTEND

int main() {
	drf7020d20 rf_test(gpio_pins, RASPI_40, RASPI_37, RASPI_38, 0);
	rf_test.enable();
	rf_test.configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	std::string hello_msg_body("HELLO");
	std::string ack_msg_body("ACK");
	std::string bye_msg_body("BYE");

	msg_t ack_msg = {
		.caller_id = CALLSIGN TAG,
		.receiver_id = CALLSIGN TAG,
		.body = ack_msg_body,
	};

	msg_t bye_msg = {
		.caller_id = CALLSIGN TAG,
		.receiver_id = CALLSIGN TAG,
		.body = bye_msg_body,
	};

	msg_t hello_msg = {
		.caller_id = CALLSIGN TAG,
		.receiver_id = CALLSIGN TAG,
		.body = hello_msg_body,
	};

	std::string input;
	std::cin >> input;

	return 0;
}
