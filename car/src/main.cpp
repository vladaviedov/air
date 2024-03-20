#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

#include <gpiod.hpp>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/pinmap.hpp>

// NOLINTBEGIN: temporary code
#define CALLSIGN ""
#define TAG "/2"

// NOLINTEND

int main() {
	drf7020d20 rf_test(gpio_pins, RASPI_40, RASPI_37, RASPI_38, 0);
	rf_test.enable();
	rf_test.configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	std::string hello_msg("HELLO " CALLSIGN TAG "\n");
	std::string ack_msg("ACK " CALLSIGN TAG "\n");
	std::string bye_msg("BYE " CALLSIGN TAG "\n");

	std::string input;
	std::cin >> input;

	if (input == "rx") {
		std::cout << rf_test.receive(std::chrono::seconds(100));
		rf_test.transmit(ack_msg);
		std::cout << rf_test.receive(std::chrono::seconds(100));
		rf_test.transmit(ack_msg);
	}

	if (input == "tx") {
		rf_test.transmit(hello_msg);
		std::cout << rf_test.receive(std::chrono::seconds(100));
		rf_test.transmit(bye_msg);
		std::cout << rf_test.receive(std::chrono::seconds(100));
	}

	return 0;
}
