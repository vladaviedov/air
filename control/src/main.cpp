#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>
#include <fstream>

#include <gpiod.hpp>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/pinmap.hpp>
#include <shared/messages.hpp>

int main() {
	drf7020d20 rf_test(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0);
	rf_test.enable();
	rf_test.configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	std::ofstream outfile;
	outfile.open("/etc/airid.txt");

	msg_t ack_msg = {
		.caller_id = "1",
		.receiver_id = "1",
		.body = "ACK",
	};

	std::string input;
	std::cin >> input;

	std::string new_msg_str;
	msg_t new_msg;

	if (input == "rx") {
		new_msg_str = rf_test.receive(std::chrono::seconds(100));
		try {
			new_msg = parse_message(new_msg_str);
			outfile << new_msg.caller_id;
		} catch (std::exception &e) {
			std::cout << e.what();
		}
	}

	return 0;
}
