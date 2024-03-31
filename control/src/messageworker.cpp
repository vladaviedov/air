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

message_worker::message_worker(const std::shared_ptr<tdma> &tdma_handler_in)
	: tdma_handler(tdma_handler_in) {
	control_id = read_id();
}

void message_worker::await_checkin() {
	std::string rx_msg;
	while (rx_msg.empty()) {
		rx_msg = tdma_handler->rx_sync(UINT32_MAX);
	}

	std::istringstream parts(rx_msg);
	std::string header;
	std::string check;

	parts >> header;
	parts >> check;

	if (!validate_header(header)) {
		tdma_handler->tx_sync(format_unsupported());
		return;
	}

	if (parts.eof() || check != "CHK") {
		tdma_handler->tx_sync(format_command(STANDBY));
		return;
	}

	tdma_handler->tx_sync(format_checkin());
}

std::pair<std::string, uint32_t> message_worker::await_request() {
	std::string rx_msg;
	while (rx_msg.empty()) {
		rx_msg = tdma_handler->rx_sync(UINT32_MAX);
	}

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
		(uint8_t)request[1]; /*TO-DO: this may need to be tested*/
	uint8_t desired_pos = (uint8_t)request[2];

	return std::pair<std::string, uint8_t>(car_id, desired_pos);
}

void message_worker::await_clear() {
	std::string rx_msg;
	while (rx_msg.empty()) {
		rx_msg = tdma_handler->rx_sync(UINT32_MAX);
	}

	if (rx_msg != CLEAR) {
		// TO-DO: handle error
		return;
	}

	tdma_handler->tx_sync(FINAL);
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
