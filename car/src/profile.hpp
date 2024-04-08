/**
 * @file src/profile.hpp
 * @brief Car calibration profile.
 */
#pragma once

#include <cstdint>
#include <optional>
#include <string>

class profile {
public:
	struct servo {
		uint32_t max_left;
		uint32_t max_right;
		uint32_t center;
	};

	struct tdma {
		int32_t tx_offset_ms;
		int32_t rx_offset_ms;
	};

	struct us {
		uint32_t threshold;
	};

	/**
	 * @brief Load profile from file.
	 *
	 * @param[in] filename - Input file.
	 */
	void load(const std::string &filename);

	/**
	 * @brief Save profile into file.
	 *
	 * @param[in] filename - Output file.
	 */
	void save(const std::string &filename) const;

	/**
	 * @brief Check if the calibration is completed.
	 *
	 */
	inline bool is_done() const {
		return servo_profile.has_value() && tdma_profile.has_value();
	}

	/** Get/Set */

	inline std::optional<servo> get_servo() const {
		return servo_profile;
	}

	inline void set_servo(servo &opts) {
		servo_profile = opts;
	}

	inline std::optional<tdma> get_tdma() const {
		return tdma_profile;
	}

	inline void set_tdma(tdma &opts) {
		tdma_profile = opts;
	}

	inline std::optional<us> get_us() const {
		return us_profile;
	}

	inline void set_us(us &opts) {
		us_profile = opts;
	}

private:
	std::optional<servo> servo_profile = std::nullopt;
	std::optional<tdma> tdma_profile = std::nullopt;
	std::optional<us> us_profile = std::nullopt;
};
