#include "messageworker.hpp"

#include <chrono>
#include <fstream>
#include <sstream>
#include <stdexcept>

#include <shared/messages.hpp>

message_worker::message_worker(const std::shared_ptr<tdma> &tdma_handler_in)
	: tdma_handler(tdma_handler_in) {
	std::ifstream infile;
	infile.open("/etc/airid");

	infile >> control_id;
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

	tdma_handler->tx_sync("COMMAND"); // TO-DO command logic

	return std::pair<std::string, uint32_t>(
		car_id, tdma_handler->get_timeslot());
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
	return UNSUPPORTED + " ";
}

std::string message_worker::format_command(std::string command) {
	return ACKNOWLEDGE + " " + command;
}
