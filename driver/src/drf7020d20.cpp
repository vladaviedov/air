/**
 * @file src/drf7020d20.cpp
 * @brief Driver implementation for the DRF7020D20 board
 */
#include "drf7020d20.hpp"

#include <cstdint>
#include <gpiod.hpp>

#include "defines.hpp"

drf7020d20::drf7020d20(
		gpiod::chip &chip,
		uint32_t en_pin,
		uint32_t aux_pin,
		uint32_t set_pin,
		uint32_t uart_port) :
	en(chip.get_line(en_pin)),
	aux(chip.get_line(aux_pin)),
	set(chip.get_line(set_pin)) {
	en.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line::DIRECTION_INPUT,
		.flags = 0
	});
	aux.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line::DIRECTION_OUTPUT,
		.flags = 0
	});
	set.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line::DIRECTION_INPUT,
		.flags = 0
	});

	// TODO: uart open
}

drf7020d20::~drf7020d20() {
	en.release();
	aux.release();
	set.release();
}

bool drf7020d20::configure(
	uint32_t freq,
	rate fsk_rate,
	uint32_t power_level,
	rate uart_rate,
	parity parity
) const {
	// Input validation
	if (freq < 418000 || freq > 455000) {
		return false;
	}
	if (fsk_rate < DR2400 || fsk_rate > DR19200) {
		return false;
	}
	if (power_level > 9) {
		return false;
	}

	// TODO: send
}
