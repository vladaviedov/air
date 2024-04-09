/**
 * @file src/messageworker.cpp
 * @brief handler for control messages
 */

#include "messageworker.hpp"

#include <chrono>
#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

#include <shared/messages.hpp>

static constexpr std::string MSG_HEADER = "AIRv1.0";
static constexpr std::string CHECK = "CHK";
static constexpr std::string ACKNOWLEDGE = "ACK";
static constexpr std::string UNSUPPORTED = "UN";
static constexpr std::string STANDBY = "SBY";
static constexpr std::string GO_REQUESTED = "GRQ";
static constexpr std::string CLEAR = "CLR";
static constexpr std::string FINAL = "FIN";

#define MESSAGE_TIMEOUT 4 /*amount of time to wait for message (in frames)*/

message_worker::message_worker(const std::shared_ptr<tdma> &tdma_handler_in)
	: tdma_handler(tdma_handler_in) {
	control_id = get_id();
}

void message_worker::await_request(
	std::function<void(uint8_t, uint8_t, std::string, message_worker)>
		callback) {
	while (true) {
		std::string rx_msg = tdma_handler->rx_sync(MESSAGE_TIMEOUT);

		std::istringstream parts(rx_msg);
		std::string header;
		std::string check;

		parts >> header;

		if (parts.eof() || !validate_header(header)) {
			continue;
		}

		parts >> check;
		if (!parts.eof() || check != "CHK") {
			continue;
		}

		tdma_handler->tx_sync(*control_id);

		std::optional<std::tuple<uint8_t, uint8_t, std::string>> request_data =
			get_request();
		if (!request_data.has_value()) {
			continue;
		}

		callback(std::get<0>(*request_data), std::get<1>(*request_data),
			std::get<2>(*request_data), *this);

		break;
	}
}

std::optional<std::tuple<uint8_t, uint8_t, std::string>>
message_worker::get_request() {
	std::string rx_msg = tdma_handler->rx_sync(MESSAGE_TIMEOUT);

	std::istringstream parts(rx_msg);
	std::string car_id;
	std::string request;

	parts >> car_id;
	if (parts.eof() || !validate_id(car_id)) {
		return std::nullopt;
	}
	parts >> request;

	request.c_str();
	uint8_t current_pos = (uint8_t)request[0];
	uint8_t desired_pos = (uint8_t)request[1];

	return std::make_tuple(current_pos, desired_pos, car_id);
}

void message_worker::await_clear(
	std::function<void(bool, message_worker)> callback) {
	std::string rx_msg = tdma_handler->rx_sync(MESSAGE_TIMEOUT);

	if (rx_msg.empty() || rx_msg != CLEAR) {
		std::cout << "Clear was not received. Clearing anyway...";
		callback(false, *this);
	}

	send_command(FINAL);
	callback(true, *this);
}

void message_worker::send_checkin() {
	std::string checkin_msg = MSG_HEADER + " " + CHECK;
	tdma_handler->tx_sync(checkin_msg);
}

void message_worker::send_unsupported() {
	std::string unsupported_msg = UNSUPPORTED + " " + *control_id;
	tdma_handler->tx_sync(unsupported_msg);
}

void message_worker::send_command(const std::string &command) {
	std::string command_msg = ACKNOWLEDGE + " " + command;
	tdma_handler->tx_sync(command_msg);
}

void message_worker::send_standby() {
	send_command(STANDBY);
}

void message_worker::send_go_requested() {
	send_command(GO_REQUESTED);
}
