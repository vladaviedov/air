/**
 * @file include/drf7020d20.hpp
 * @brief Driver header for the DRF7020D20 board
 */
#pragma once

#include <chrono>
#include <cstdint>

#include <gpiod.hpp>

#include "uart.hpp"

class drf7020d20 {
public:
	enum rate {
		DR1200 = 0,
		DR2400 = 1,
		DR4800 = 2,
		DR9600 = 3,
		DR19200 = 4,
		DR38400 = 5,
		DR57600 = 6
	};

	enum parity {
		NONE = 0,
		EVEN = 1,
		ODD = 2
	};

	/**
	 * @brief Constructor.
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] en_pin - EN libgpiod pin number.
	 * @param[in] aux_pin - AUX libgpiod pin number.
	 * @param[in] set_pin - SET libgpiod pin number.
	 * @param[in] uart_port - Number ID of the UART port to use.
	 */
	drf7020d20(const gpiod::chip &chip,
		uint32_t en_pin,
		uint32_t aux_pin,
		uint32_t set_pin,
		uint32_t uart_port);

	~drf7020d20();

	/**
	 * @brief Enable module.
	 */
	void enable();

	/**
	 * @brief Disable module.
	 *
	 */
	void disable();

	/**
	 * @brief Configure RF module.
	 *
	 * @param[in] freq - Channel frequency in kHz.
	 * @note Range: 418000-455000 kHz.
	 * @param[in] fsk_rate - FSK bitrate.
	 * @note Only DR2400-DR19200 bps is allowed.
	 * @param[in] power_level - RF power level (0-9).
	 * @param[in] uart_rate - UART bitrate.
	 * @param[in] parity - Parity configuration.
	 * @return Boolean result.
	 */
	bool configure(uint32_t freq,
		rate fsk_rate,
		uint32_t power_level,
		rate uart_rate,
		parity parity) const;

	/**
	 * @brief Transmit message over radio.
	 *
	 * @param[in] msg - Message to send.
	 * @return Boolean result.
	 */
	bool transmit(const std::string &msg) const;

	/**
	 * @brief Receive message over radio.
	 * @note Blocks until a message is received or timeout.
	 *
	 * @param[in] timeout - Wait timeout.
	 * @return Received message. Empty string on timeout.
	 */
	std::string receive(std::chrono::milliseconds timeout) const;

private:
	uart serial;
	gpiod::line en;
	gpiod::line aux;
	gpiod::line set;

	bool enable_flag = false;
};
