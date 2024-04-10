/**
 * @file src/messageworker.hpp
 * @brief Message handler for car
 */
#pragma once

#include <cstdlib>
#include <memory>
#include <string>
#include <functional>

#include <shared/tdma.hpp>

class message_worker {
public:
	enum command {
		SBY,
		GRQ,
	};
	/**
	 * @brief constructor for car message worker
	 * @param[in] tdma_handler_in
	 */
	message_worker(const std::shared_ptr<tdma> &tdma_handler_in);

	/**
	 * @brief send and receive check in from control
	 * @return control id
	 */
	std::string await_checkin();

	/**
	 * @brief send request callback
	*/
	message_worker::command send_request(uint8_t desired_pos);

	/**
	 * @brief send clear
	*/
	void send_clear();

	/**
	 * @brief create check in message
	 * @return check in message
	 */
	std::string format_checkin();

	/**
	 * @param[in] desired_pos position car is requesting to go
	 * @brief creates request for car with id, current position, and desired
	 * position
	 * @return request message
	 */
	std::string format_request(uint8_t desired_pos);

private:
	std::shared_ptr<tdma> tdma_handler;
	std::shared_ptr<std::string> car_id;
	uint8_t current_pos;
};
