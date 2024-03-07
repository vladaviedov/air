#pragma once

#include <cstdint>
#include <termios.h>
#include <string>

class uart {

public:
	uart(uint32_t port);
	~uart();

	bool write(const std::string &data) const;
	bool write(const char *data, uint32_t length) const;
	std::string read() const;
	int32_t read(char *buffer, uint32_t max_length) const;

private:
	int fd;
	struct termios old_settings;

};
