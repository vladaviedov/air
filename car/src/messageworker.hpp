#pragma once

#include <memory>
#include <stdlib.h>
#include <string>

#include <shared/tdma.hpp>

class message_worker {
public:
	message_worker(const std::shared_ptr<tdma> &tdma_handler_in);
	void await_checkin();

	std::string format_checkin();
	std::string format_request(uint8_t desired_pos);
	std::string format_acknowledge(std::string command);

private:
	std::shared_ptr<tdma> tdma_handler;
	std::shared_ptr<std::string> car_id;
	uint8_t current_pos;
};
