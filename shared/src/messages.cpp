/**
 * @file src/messages.cpp
 * @brief Messaging utilities for parsing and formatting messages
 */

#include "messages.hpp"

#include <iostream>
#include <sstream>
#include <stdexcept>

constexpr std::string MSG_HEADER = "AIRv1.0";
constexpr std::string UNSUPPORTED = "UN";

bool validate_header(const std::string &str) {
	if (str == MSG_HEADER) 
		return true;
	return false;
}

bool validateId(const std::string &str) {

	if (str.length() < 2) {
		return false;
	}

	if (str.substr(0,2) == UNSUPPORTED) {
		return false;
	}

	for (size_t i = 0; i < str.length(); i++) {
		char c = str[i];
		if (!isalnum(c) && c != '/' && c != '-') 
			return false;	
	}

	return true;
}


