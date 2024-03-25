/**
 * @file src/messages.cpp
 * @brief Messaging utilities for parsing and formatting messages
 */

#include "messages.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

constexpr std::string MSG_HEADER = "AIRv1.0";
constexpr std::string START_MSG = "SM";
constexpr std::string END_MSG = "EM";

bool check_line1(const std::string &line1, std::string &r_id);
bool check_line3(const std::string &line3, std::string &c_id);

std::string format_message(const msg_t &msg) {
	std::string str_msg(MSG_HEADER + " " + msg.receiver_id + " " + START_MSG +
						"\n" + msg.body + "\n" + END_MSG + " " + msg.caller_id);
	return str_msg;
}

bool check_line1(const std::string &line1, std::string &r_id) {
	std::istringstream parts(line1);
	std::string token;

	parts >> token;
	if (parts.eof() || token != MSG_HEADER) {
		return false;
	}

	parts >> token;
	if (parts.eof() || token == START_MSG || token == END_MSG) {
		return false;
	}
	r_id = token;

	parts >> token;
	if (token != START_MSG || !parts.eof()) {
		return false;
	}

	return true;
}

bool check_line3(const std::string &line3, std::string &c_id) {
	std::istringstream parts(line3);
	std::string token;

	parts >> token;
	if (parts.eof() || token != END_MSG) {
		return false;
	}

	parts >> token;
	if (token == START_MSG || token == END_MSG || !parts.eof()) {
		return false;
	}

	c_id = token;
	return true;
}

msg_t parse_message(const std::string &str_msg) {
	std::istringstream lines(str_msg);

	std::string line1;
	std::string line2;
	std::string line3;

	/* Read lines */
	std::getline(lines, line1, '\n');
	if (lines.eof()) {
		throw std::invalid_argument("line 2 not found");
	}
	std::getline(lines, line2, '\n');

	if (lines.eof()) {
		throw std::invalid_argument("line 3 not found");
	}
	std::getline(lines, line3, '\n');

	std::string receiver_id;
	std::string caller_id;

	/* Check validity of lines and initialize ids*/
	if (!check_line1(line1, receiver_id)) {
		throw std::invalid_argument("Line 1 is invalid");
	}

	if (!check_line3(line3, caller_id)) {
		throw std::invalid_argument("Line 3 is invalid");
	}

	for (size_t i = 0; i < receiver_id.length(); i++) {
		char c = receiver_id[i];
		if (!isalnum(c) && c != '/' && c != '-') {
			throw std::invalid_argument("Receiver ID is invalid");
		}
	}

	for (size_t i = 0; i < caller_id.length(); i++) {
		char c = caller_id[i];
		if (!isalnum(c) && c != '/' && c != '-') {
			throw std::invalid_argument("Caller ID is invalid");
		}
	}

	/* Parse Messages */
	msg_t msg = {
		.caller_id = caller_id,
		.receiver_id = receiver_id,
		.body = line2,
	};

	return msg;
}
