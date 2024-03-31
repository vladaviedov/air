/**
 * @file include/messages.hpp
 * @brief header for messaging utilities for parsing and formatting messages
 */

#pragma once

#include <cstdint>
#include <memory>
#include <string>

struct msg_t {
	std::string caller_id;
	std::string receiver_id;
	std::string body;
};

bool validate_header(const std::string &str);
bool validate_id(const std::string &str);
const std::shared_ptr<std::string> &read_id();
