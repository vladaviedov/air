/**
 * @file driver/lightsens.hpp
 * @brief Infrared sensor driver header.
 */
#pragma once

#include <cstdint>
#include <gpiod.hpp>

class light_sens {

public:
	/**
	 * @brief Constructor.
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] input_pin - IRx libgpiod pin number.
	 */
	light_sens(gpiod::chip &chip, uint32_t input_pin);

	~light_sens();

	/**
	 * @brief Check if light sensor detects an edge.
	 *
	 * @return Boolean result.
	 */
	bool read() const;

private:
	const gpiod::line input;

};
