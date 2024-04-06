#include "messageworker.hpp"

#include <sstream>
#include <stdexcept>

#include <shared/messages.hpp>

static constexpr std::string MSG_HEADER = "AIRv1.0";
static constexpr std::string CHECK = "CHK";
static constexpr std::string ACKNOWLEDGE = "ACK";

message_worker::message_worker(const std::shared_ptr<tdma> &tdma_handler_in)
	: tdma_handler(tdma_handler_in) {
	car_id = get_id();
}

void message_worker::await_checkin() {
	tdma_handler->tx_sync(format_checkin());

	std::string rx_msg;
	while (rx_msg.empty()) {
		rx_msg = tdma_handler->rx_sync(UINT32_MAX);
	}

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
}

std::string message_worker::format_checkin() {
	return MSG_HEADER + " " + CHECK;
}

std::string message_worker::format_request(uint8_t desired_pos) {
	return *car_id + " " + std::to_string(current_pos | desired_pos);
}

std::string message_worker::format_acknowledge(std::string command) {
	return ACKNOWLEDGE + " " + command;
}
