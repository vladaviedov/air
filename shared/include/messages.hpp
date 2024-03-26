/**
 * @file include/messages.hpp
 * @brief header for messaging utilities for parsing and formatting messages
 */

#pragma once

#include <cstdint>
#include <string>

struct msg_t {
	std::string caller_id;
	std::string receiver_id;
	std::string body;
};

std::string format_message(const msg_t &msg);
msg_t parse_message(const std::string &str_msg);
void read_caller_id(msg_t &msg, const std::string filename);
