/**
 * @file src/messageworker.cpp
 * @brief Message handler for car
 */
#include "messageworker.hpp"

#include <functional>
#include <sstream>
#include <stdexcept>

#include <shared/messages.hpp>

static constexpr std::string MSG_HEADER = "AIRv1.0";
static constexpr std::string CHECK = "CHK";
static constexpr std::string ACKNOWLEDGE = "ACK";
static constexpr std::string STANDBY = "SBY";
static constexpr std::string GO_REQUESTED = "GRQ";
static constexpr std::string CLEAR = "CLR";
static constexpr std::string FINAL = "FIN";

static constexpr uint8_t MESSAGE_TIMEOUT =
	4; /*time to wait for message (in frames)*/

message_worker::message_worker(const std::shared_ptr<tdma> &tdma_handler_in,
	std::atomic<bool> &active_flag_in)
	: tdma_handler(tdma_handler_in),
	  active_flag(active_flag_in),
	  car_id(get_id()) {}

std::string message_worker::await_checkin() {
	tdma_handler->tx_sync(format_checkin()); // send check in

	std::string control_id;

	while (active_flag) {
		control_id = tdma_handler->rx_sync(MESSAGE_TIMEOUT); // receive check in

		if (!validate_id(control_id)) {
			continue;
		}

		break;
	}

	uint8_t desired_pos = 0; // choose random pos

	return control_id;
}

message_worker::command message_worker::send_request(uint8_t desired_pos) {
	tdma_handler->tx_sync(format_request(desired_pos));
	std::string command;
	uint32_t i = 0;

	while (i < 3) {
		i++;

		std::string response =
			tdma_handler->rx_sync(MESSAGE_TIMEOUT); // receive check in

		std::istringstream parts(response);
		std::string ack;

		parts >> ack;
		if (parts.eof() || ack != ACKNOWLEDGE) {
			continue;
		}
		parts >> command;
		if (!parts.eof()) {
			continue;
		}
	}

	tdma_handler->tx_sync(ACKNOWLEDGE);

	if (command == STANDBY) {
		return SBY;
	} else if (command == GO_REQUESTED) {
		return GRQ;
	} else {
		throw std::invalid_argument("Unsupported");
	}
}

void message_worker::send_clear() {
	tdma_handler->tx_sync(CLEAR);
}

std::string message_worker::format_checkin() {
	return MSG_HEADER + " " + CHECK;
}

std::string message_worker::format_request(uint8_t desired_pos) {
	std::string formatted_request;
	formatted_request.append(*car_id + " ");
	formatted_request.push_back(current_pos);
	formatted_request.push_back(desired_pos);
	return formatted_request;
}
