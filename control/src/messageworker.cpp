/**
 * @file src/messageworker.cpp
 * @brief handler for control messages
 */

#include "messageworker.hpp"

#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <shared/messages.hpp>

static constexpr std::string MSG_HEADER = "AIRv1.0";
static constexpr std::string CHECK = "CHK";
static constexpr std::string ACKNOWLEDGE = "ACK";
static constexpr std::string UNSUPPORTED = "UN";
static constexpr std::string STANDBY = "SBY";
static constexpr std::string CLEAR = "CLR";
static constexpr std::string FINAL = "FIN";

#define MESSAGE_TIMEOUT 4 /*amountof time to wait for message (in frames)*/

message_worker::message_worker(const std::shared_ptr<tdma> &tdma_handler_in)
	: tdma_handler(tdma_handler_in) {
	control_id = get_id();
}

bool message_worker::await_checkin() {
	
	std::string rx_msg = tdma_handler->rx_sync(MESSAGE_TIMEOUT);

	std::istringstream parts(rx_msg);
	std::string header;
	std::string check;

	parts >> header;

	if (!validate_header(header)) {
		tdma_handler->tx_sync(format_unsupported());
		return false;
	}

	parts >> check;
	if (parts.eof() || check != "CHK") {
		tdma_handler->tx_sync(format_command(STANDBY));
		return false;
	}

	tdma_handler->tx_sync(format_checkin());
	return true;
}

std::pair<uint8_t, uint8_t> message_worker::await_request() {
	std::string rx_msg = tdma_handler->rx_sync(MESSAGE_TIMEOUT);
	// how should I handle if message isn't received here?

	std::istringstream parts(rx_msg);

	std::string car_id;
	std::string request;

	parts >> car_id;
	if (parts.eof() || !validate_id(car_id)) {
		tdma_handler->tx_sync(format_command(STANDBY));
	}
	parts >> request;

	request.c_str();
	uint8_t current_pos =
		(uint8_t)request[0]; /*TO-DO: this may need to be tested*/
	uint8_t desired_pos = (uint8_t)request[1];

	return std::pair<uint8_t, uint8_t>(current_pos, desired_pos);
}

bool message_worker::await_clear() {
	std::string rx_msg = tdma_handler->rx_sync(MESSAGE_TIMEOUT);

	if (rx_msg != CLEAR) {
		// TO-DO: handle error
		return false;
	}

	tdma_handler->tx_sync(FINAL);
	return true;
}

std::string message_worker::format_checkin() {
	return MSG_HEADER + " " + CHECK;
}

std::string message_worker::format_unsupported() {
	return UNSUPPORTED + " " + *control_id;
}

std::string message_worker::format_command(const std::string &command) {
	return ACKNOWLEDGE + " " + command;
}
