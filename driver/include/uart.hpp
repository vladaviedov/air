/**
 * @file include/uart.hpp
 * @brief UART Serial header.
 */
#pragma once

#include <cstdint>
#include <string>
#include <termios.h>

class uart {
public:
	/**
	 * @brief Constructor.
	 *
	 * @param[in] port - UART port as defined by the OS.
	 */
	uart(uint32_t port);

	~uart();

	/**
	 * @brief Write to serial.
	 *
	 * @param[in] data - Data to write.
	 * @return Boolean result.
	 */
	bool write(const std::string &data) const;

	/**
	 * @brief Write to serial.
	 *
	 * @param[in] data - Data buffer to write.
	 * @param[in] length - Length of message.
	 * @return Boolean result.
	 */
	bool write(const char *data, uint32_t length) const;

	/**
	 * @brief Read from serial.
	 *
	 * @return Serial output. Empty string on error.
	 */
	std::string read() const;

	/**
	 * @brief Read from serial.
	 *
	 * @param[out] buffer - Data buffer to write to.
	 * @param[in] max_length - Maximum bytes to write to buffer.
	 * @return Bytes read from serial.
	 */
<<<<<<< HEAD
	int64_t read(char *buffer, uint32_t max_length) const;
=======
	ssize_t read(char *buffer, uint32_t max_length) const;
>>>>>>> 8bc0ad2ea7aeae8481714a860d9920671597084e

private:
	int fd;
	struct termios old_settings;
};
