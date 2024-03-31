/**
 * @file src/messages.cpp
 * @brief Messaging utilities for parsing and formatting messages
 */

#include "messages.hpp"

#include <fstream>
#include <iostream>
#include <sstream>
#include <stdexcept>

constexpr std::string MSG_HEADER = "AIRv1.0";
constexpr std::string UNSUPPORTED = "UN";

static std::shared_ptr<std::string> control_id = nullptr;

bool validate_header(const std::string &str) {
	return str == MSG_HEADER;
}

bool validate_id(const std::string &str) {
	if (str.length() > 12) {
		return false;
	}

	// clang-tidy cannot understand the greatness of C
	// NOLINTBEGIN(readability-identifier-length,
	// 		readability-implicit-bool-conversion)
	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		if (!isalnum(c) && c != '/' && c != '-') {
			return false;
		}
	}
	// NOLINTEND(readability-identifier-length,
	// 		readability-implicit-bool-conversion)

	return str.length() < 2 || str.substr(0, 2) != UNSUPPORTED;
}

const std::shared_ptr<std::string> &read_id() {
	if (control_id == nullptr) {
		control_id = std::make_shared<std::string>();

		std::ifstream infile;
		infile.open("/etc/airid");

		infile >> *control_id;
	}
	return control_id;
}
