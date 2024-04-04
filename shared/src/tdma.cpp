/**
 * @file src/tdma.cpp
 * @brief TDMA wrapper for RF communications.
 */
#include "tdma.hpp"

#include <bits/chrono.h>
#include <chrono>
#include <cstdint>
#include <string>
#include <map>
#include <thread>

#include <driver/drf7020d20.hpp>

static constexpr uint32_t TIMESLOT_DURATION_MS = 20;
static constexpr auto TIMESLOT_DURATION = std::chrono::milliseconds(TIMESLOT_DURATION_MS);

static constexpr uint32_t A_FRAME_DUR = TIMESLOT_DURATION_MS * 4;
static constexpr uint32_t B_FRAME_DUR = TIMESLOT_DURATION_MS * 8;
static constexpr uint32_t C_FRAME_DUR = TIMESLOT_DURATION_MS * 16;

static constexpr uint32_t A_FRAMES_PER_SEC = 1000 / A_FRAME_DUR;
static constexpr uint32_t B_FRAMES_PER_SEC = 1000 / B_FRAME_DUR;
static constexpr uint32_t C_FRAMES_PER_SEC = 1000 / C_FRAME_DUR;

static const std::map<tdma::scheme, tdma::scheme_info> SCHEME_MAP = {
	{ tdma::AIR_A, { .frame_duration_ms = A_FRAME_DUR, .frames_per_second = A_FRAMES_PER_SEC } },
	{ tdma::AIR_B, { .frame_duration_ms = B_FRAME_DUR, .frames_per_second = B_FRAMES_PER_SEC } },
	{ tdma::AIR_C, { .frame_duration_ms = C_FRAME_DUR, .frames_per_second = C_FRAMES_PER_SEC } }
};

tdma::tdma(const std::shared_ptr<drf7020d20> &rf_dev_in, uint32_t timeslot, scheme div) : rf_dev(rf_dev_in), slot(timeslot), sch_info(SCHEME_MAP.at(div)) {
	rf_dev->rejecter_on();
}

bool tdma::tx_sync(const std::string &msg) const {
	if (msg.length() > 15) {
		return false;
	}

	sleep_until_next_slot(tx_offset_ms);
	return rf_dev->transmit(msg.data(), 15);
}

std::string tdma::rx_sync(uint32_t max_frames) const {
	for (uint32_t i = 0; i < max_frames; i++) {
		sleep_until_next_slot(rx_offset_ms);
		std::string res = rf_dev->receive(TIMESLOT_DURATION);

		if (!res.empty()) {
			return res;
		}
	}

	return "";
}

void tdma::sleep_until_next_slot(int32_t offset_ms) const {
	auto timestamp = std::chrono::system_clock::now();
	auto timestamp_adj = timestamp - std::chrono::milliseconds(offset_ms);
	auto ms_part = timestamp_adj.time_since_epoch().count() / 1000000 % 1000;

	// Current position
	uint32_t cur_frame = ms_part / sch_info.frame_duration_ms;
	uint32_t cur_slot = (ms_part % sch_info.frame_duration_ms) / TIMESLOT_DURATION_MS;

	// Same frame if timeslot not passed, otherwise next frame
	uint32_t send_frame = (cur_slot >= slot)
		? cur_frame + 1 : cur_frame;

	// If above allowed frames, go to next second
	auto second = (send_frame >= sch_info.frames_per_second)
		? std::chrono::ceil<std::chrono::seconds>(timestamp_adj)
		: std::chrono::floor<std::chrono::seconds>(timestamp_adj);
	if (send_frame >= sch_info.frames_per_second) {
		send_frame = 0;
	}

	// Calculate offset from second start
	auto ms_desired = std::chrono::milliseconds((int32_t)(send_frame * sch_info.frame_duration_ms + slot * TIMESLOT_DURATION_MS) + offset_ms);
	
	std::this_thread::sleep_until(second + ms_desired);
}
