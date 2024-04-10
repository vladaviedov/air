/**
 * @file src/messageworker.hpp
 * @brief handler for control messages
 */
#pragma once

#include <cstdint>
#include <cstdlib>
#include <functional>
#include <optional>
#include <string>
#include <tuple>

#include <shared/tdma.hpp>

class message_worker {
public:
	/**
	 * @brief constructor for control message worker
	 * @param[in] tdma_handler_in
	 */
	message_worker(const std::shared_ptr<tdma> &tdma_handler_in, std::atomic<bool> &active_flag_in);

	/**
	 * @brief awaits request form car synchronously
	 * @return true if check in is sent successfully
	 */
	std::tuple<uint8_t, uint8_t, std::string> await_request_sync();

	/**
	 * @brief awaits request form car
	 * @return true if check in is sent successfully
	 * @param[in] callback
	 */
	void await_request(
		std::function<void(uint8_t, uint8_t, std::string, message_worker)>
			callback);
	/**
	 * @brief receives request from car
	 * @return pair of current and desired position of car
	 */
	std::optional<std::tuple<uint8_t, uint8_t, std::string>> get_request();

	/**
	 * @brief receives clear message from car and ends conversation synchronously
	 * @return true if clear is sent successfully
	 */
	bool await_clear_sync();

	/**
	 * @brief receives clear message from car and ends conversation
	 * @return true if clear is sent successfully
	 * @param[in] callback
	 */
	void await_clear(std::function<void(bool, message_worker)> callback);

	/**
	 * @brief sends check in message
	 */
	void send_checkin();

	/**
	 * @brief sends unsupported message
	 */
	void send_unsupported();

	/**
	 * @brief sends standby message
	 */
	void send_standby();

	/**
	 * @brief sends go as requested
	 */
	void send_go_requested();

	/**
	 * @brief get timeslot of car
	 * @return timeslot
	 */
	inline uint32_t get_timeslot() {
		return tdma_handler->get_timeslot();
	}

private:
	/**
	 * @param command response to car's request
	 * @brief creates command message
	 */
	void send_command(const std::string &command);

	std::atomic<bool> &active_flag;
	std::shared_ptr<tdma> tdma_handler;
	std::shared_ptr<std::string> control_id;
};
