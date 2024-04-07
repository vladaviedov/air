/**
 * @file src/messageworker.cpp
 * @brief Message handler for car
*/
#include "messageworker.hpp"

#include <sstream>
#include <stdexcept>

#include <shared/messages.hpp>

static constexpr std::string MSG_HEADER = "AIRv1.0";
static constexpr std::string CHECK = "CHK";
static constexpr std::string ACKNOWLEDGE = "ACK";

#define MESSAGE_TIMEOUT 100 /*time to wait for message (in frames)*/

message_worker::message_worker(const std::shared_ptr<tdma> &tdma_handler_in)
	: tdma_handler(tdma_handler_in) {
	car_id = get_id();
}

void message_worker::await_checkin() {
	tdma_handler->tx_sync(format_checkin()); // send check in 

	std::string rx_msg = tdma_handler->rx_sync(MESSAGE_TIMEOUT); // receive check in

	/*
	std::istringstream parts(rx_msg);
	std::string header;
	std::string check;

	parts >> header;
	if (parts.eof() || !validate_header(header)) {
		throw std::invalid_argument("Header is invalid");
	}
	parts >> check;
	if (!parts.eof() || check != CHECK) {
		throw std::invalid_argument("Check is invalid");
	}
	*/
}

std::string message_worker::format_checkin() {
	return MSG_HEADER + " " + CHECK;
}

std::string message_worker::format_request(uint8_t desired_pos) {
	uint8_t request_data = current_pos | desired_pos;
	std::string formatted_request;
	formatted_request.append(*car_id + " ");
	formatted_request.push_back(request_data);
	return formatted_request;
}

std::string message_worker::format_acknowledge(std::string command) {
	return ACKNOWLEDGE + " " + command;
}
