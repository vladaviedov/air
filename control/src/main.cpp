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

#define C_ID "2"
#define R_ID "1"

msg_t ack_msg;

int main() {

	/*config pin*/
	drf7020d20 rf_test(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0);
	rf_test.enable();
	rf_test.configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	/*messages*/
	ack_msg.receiver_id = R_ID;
	ack_msg.caller_id = C_ID;
	ack_msg.body = "ACK";

	std::string ack_str = format_message(ack_msg);

	std::string input;
	std::cin >> input;

	if (input == "rx") {
		std::string new_msg = rf_test.receive(std::chrono::seconds(100));
		rf_test.transmit(ack_str);

		msg_t air_msg = parse_message(new_msg);
		std::cout << "r-id: " << air_msg.receiver_id << std::endl;
		std::cout << "c-id: " << air_msg.caller_id << std::endl;
		std::cout << "body: " << air_msg.body << std::endl;
	}
	

}
