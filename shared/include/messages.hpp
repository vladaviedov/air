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

bool validate_header(const std::string &str);
bool validateId(const std::string &str);
