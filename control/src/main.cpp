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
#include <intersection.hpp>

#define RECEIVING_TIMEOUT 100

int main() {
	/*config pin*/
	drf7020d20 rf_test(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0);
	rf_test.enable();
	rf_test.configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	/*Car init*/
	car_t car_1;
	car_t car_2;
	car_t car_3;
	car_t cars[3] = {car_1, car_2, car_3};

	/*control*/

	std::string new_msg_str = rf_test.receive(std::chrono::seconds(RECEIVING_TIMEOUT));
	msg_t new_msg = parse_message(new_msg_str);

	std::string c_id = new_msg.caller_id;
	std::string body = new_msg.body;

	uint8_t curr_car_index = find_car(cars, c_id);

	if (body == "CHK") {
		//CHK
	} else if (body == "RQ1") {

	} else if (body == "RQ2") {	
		
	} else if (body == "RQ3") {
		
	} else if (body == "RQ4") {
		
	} else if (body == "RQU") {
		
	} else if (body == "CLR") {
		
	} else if (body == "PS1") {
		update_position(cars[curr_car_index], body);
	} else if (body == "PS2") {
		update_position(cars[curr_car_index], body);
	} else if (body == "PS3") {
		update_position(cars[curr_car_index], body);
	} else if (body == "PS4") {
		update_position(cars[curr_car_index], body);
	}

	/*messages
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
	*/ 
	
}
