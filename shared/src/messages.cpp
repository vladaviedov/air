/**
 * @file src/messages.cpp
 * @brief Messaging utilities for parsing and formatting messages
 */

#include "messages.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

constexpr std::string MSG_HEADER = "AIRv1.0";
constexpr std::string UN = "UN";

bool validate_header(const std::string &str) {
	if (str == MSG_HEADER)
		return true;
	return false;
}

id_status validateId(const std::string &str) {
	if (str.length() < 2 || str.length() > 12) {
		return ERROR;
	}

	if (str.substr(0, 2) == UN) {
		return UNSUPPORTED;
	}

	// clang-tidy cannot understand the greatness of C
	// NOLINTBEGIN(readability-identifier-length,
	// 		readability-implicit-bool-conversion)
	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		if (!isalnum(c) && c != '/' && c != '-') {
			return ERROR;
		}
	}
	// NOLINTEND(readability-identifier-length,
	// 		readability-implicit-bool-conversion)

	return OK;
}
