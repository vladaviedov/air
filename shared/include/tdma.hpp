/**
 * @file include/tdma.hpp
 * @brief TDMA wrapper for RF communication.
 */
#pragma once

#include <cstdint>
#include <string>

#include <driver/drf7020d20.hpp>

class tdma {
public:
	enum scheme {
		AIR_A,
		AIR_B,
		AIR_C
	};

	tdma(drf7020d20 &rf_dev_in, uint32_t timeslot, scheme div);

	/**
	 * @brief Transmit message synchronously.
	 *
	 * @param[in] msg - Message, must be at most 15 bytes.
	 * @return Boolean result.
	 */
	bool tx_sync(const std::string &msg) const;

	/**
	 * @brief Receive message synchronously.
	 *
	 * @param[in] max_frames - Maximum frames before timeout
	 * @return Received message, or empty string on timeout.
	 */
	std::string rx_sync(uint32_t max_frames) const;

	/**
	 * @brief Adjust timing for clock desync.
	 *
	 * @param[in] new_offset - ms offset for message start.
	 */
	inline void set_offset(int32_t new_offset) {
		offset_ms = new_offset;
	}

private:
	void sleep_until_next_slot() const;

	drf7020d20 &rf_dev;
	uint32_t slot;
	scheme sch;
	int32_t offset_ms = 0;
};
