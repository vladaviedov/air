#include "uart.hpp"

#include <cstdio>
#include <cstdint>
#include <string>
#include <stdexcept>
#include <unistd.h>
#include <fcntl.h>
#include <termios.h>

uart::uart(uint32_t port) {
	char serial[32];
	std::snprintf(serial, 32, "/dev/serial%u", port);
	if ((fd = open(serial, O_RDWR)) < 0) {
		throw std::runtime_error("Failed to open serial port");
	}

	// Get current settings
	tcgetattr(fd, &old_settings);

	// Copy and fix settings
	struct termios settings = old_settings;
	settings.c_lflag &= ~(ECHO | ICANON);
	tcsetattr(fd, TCSANOW, &settings);
}

uart::~uart() {
	// Restore settings
	tcsetattr(fd, TCSANOW, &old_settings);

	// Close serial
	close(fd);
}

bool uart::write(const std::string &data) const {
	return ::write(fd, data.c_str(), data.length()) < 0;
}

bool uart::write(const char *data, uint32_t length) const {
	return ::write(fd, data, length) < 0;
}

std::string uart::read() const {
	char buffer[256];
	if (::read(fd, buffer, 256) < 0) {
		return "";
	}

	return std::string(buffer);
}

int32_t uart::read(char *buffer, uint32_t max_length) const {
	return ::read(fd, buffer, max_length);
}
