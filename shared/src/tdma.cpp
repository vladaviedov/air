/**
 * @file src/tdma.cpp
 * @brief TDMA wrapper for RF communications.
 */
#include "tdma.hpp"

#include <chrono>
#include <cstdint>
#include <string>

#include <driver/drf7020d20.hpp>

static constexpr auto TIMESLOT_DURATION = std::chrono::milliseconds(20);

tdma::tdma(drf7020d20 &rf_dev_in, uint32_t timeslot, scheme div) : rf_dev(rf_dev_in), slot(timeslot), sch(div) {}

bool tdma::tx_sync(const std::string &msg) const {
	if (msg.length() > 15) {
		return false;
	}

	sleep_until_next_slot();
	return rf_dev.transmit(msg.data(), 15);
}

std::string tdma::rx_sync(uint32_t max_frames) const {
	// TODO: need to clear events outside of ts?
	for (uint32_t i = 0; i < max_frames; i++) {
		sleep_until_next_slot();
		std::string res = rf_dev.receive(TIMESLOT_DURATION);

		if (res != "") {
			return res;
		}
	}

	return "";
}

void tdma::sleep_until_next_slot() const {
	// TODO: implement
}
