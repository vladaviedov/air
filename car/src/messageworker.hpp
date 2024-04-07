/**
 * @file src/messageworker.hpp
 * @brief Message handler for car
*/
#pragma once

#include <memory>
#include <cstdlib>
#include <string>

#include <shared/tdma.hpp>

class message_worker {
public:
	message_worker(const std::shared_ptr<tdma> &tdma_handler_in);
	/**
	 * @brief send and receive check in from control
	*/
	void await_checkin();

	/**
	 * @brief create check in message
	*/
	std::string format_checkin();
	/**
	 * @param desired_pos position car is requesting to go
	 * @brief creates request for car with id, current position, and desired position
	*/
	std::string format_request(uint8_t desired_pos);
	/**
	 * @param command given by control
	 * @brief acknowledges command given by control
	*/
	std::string format_acknowledge(std::string command);

private:
	std::shared_ptr<tdma> tdma_handler;
	std::shared_ptr<std::string> car_id;
	uint8_t current_pos;
};
