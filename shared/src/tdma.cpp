/**
 * @file src/tdma.cpp
 * @brief TDMA wrapper for RF communications.
 */
#include "tdma.hpp"

#include <bits/chrono.h>
#include <chrono>
#include <cstdint>
#include <map>
#include <string>
#include <thread>

#include <driver/drf7020d20.hpp>

static constexpr auto TIMESLOT_DURATION = std::chrono::milliseconds(20);

static const std::map<tdma::scheme, tdma::scheme_info> SCHEME_MAP = {
	{tdma::AIR_A, {.frame_duration_ms = 80, .frames_per_second = 12}},
	{tdma::AIR_B, {.frame_duration_ms = 160, .frames_per_second = 6}},
	{tdma::AIR_C, {.frame_duration_ms = 320, .frames_per_second = 3}}};

tdma::tdma(
	const std::shared_ptr<drf7020d20> &rf_dev_in, uint32_t timeslot, scheme div)
	: rf_dev(rf_dev_in),
	  slot(timeslot),
	  sch_info(SCHEME_MAP.at(div)) {}

bool tdma::tx_sync(const std::string &msg) const {
	if (msg.length() > 15) {
		return false;
	}

	sleep_until_next_slot();
	return rf_dev->transmit(msg.data(), 15);
}

std::string tdma::rx_sync(uint32_t max_frames) const {
	// TODO: need to clear events outside of ts?
	for (uint32_t i = 0; i < max_frames; i++) {
		sleep_until_next_slot();
		std::string res = rf_dev->receive(TIMESLOT_DURATION);

		if (!res.empty()) {
			return res;
		}
	}

	return "";
}

void tdma::sleep_until_next_slot() const {
	auto now = std::chrono::system_clock::now();
	auto now_ms = now.time_since_epoch().count() / 1000000 % 1000;

	// Current position
	uint32_t cur_frame = now_ms / sch_info.frame_duration_ms;
	uint32_t cur_slot =
		(now_ms % sch_info.frame_duration_ms) / TIMESLOT_DURATION.count();

	// Same frame if timeslot not passed, otherwise next frame
	uint32_t send_frame = (cur_slot >= slot) ? cur_frame + 1 : cur_frame;

	// If above allowed frames, go to next second
	auto second = (send_frame >= sch_info.frames_per_second)
					  ? std::chrono::ceil<std::chrono::seconds>(now)
					  : std::chrono::floor<std::chrono::seconds>(now);
	if (send_frame >= sch_info.frames_per_second) {
		send_frame = 0;
	}

	// Calculate offset from second start
	auto offset =
		std::chrono::milliseconds(send_frame * sch_info.frame_duration_ms +
								  slot * TIMESLOT_DURATION.count());

	std::this_thread::sleep_until(second + offset);
}
