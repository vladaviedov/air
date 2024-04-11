/**
 * @file src/profile.cpp
 * @brief Car calibration profile.
 */
#include "profile.hpp"

#include <fstream>
#include <optional>
#include <sstream>
#include <stdexcept>
#include <string>

// File parsing: servo profile
static constexpr std::string CHECK_SERVO = "[servo]";
static constexpr std::string CHECK_SERVO_MAX_LEFT = "left";
static constexpr std::string CHECK_SERVO_MAX_RIGHT = "right";
static constexpr std::string CHECK_SERVO_CENTER = "center";

// File parsing: tdma profile
static constexpr std::string CHECK_TDMA = "[tdma]";
static constexpr std::string CHECK_TDMA_TX_OFFSET_MS = "tx";
static constexpr std::string CHECK_TDMA_RX_OFFSET_MS = "rx";

// File parsing: us profile
static constexpr std::string CHECK_US = "[us]";
static constexpr std::string CHECK_US_THRESHOLD = "threshold";

// File parsing: turning profile
static constexpr std::string CHECK_TURN = "[turn]";
static constexpr std::string CHECK_TURN_RIGHT_MS = "right";
static constexpr std::string CHECK_TURN_RIGHT_DELAY_MS = "right_delay";
static constexpr std::string CHECK_TURN_LEFT_MS = "left";
static constexpr std::string CHECK_TURN_LEFT_DELAY_MS = "left_delay";

template<typename T>
static void load_field(
	std::ifstream &file, const std::string &check, T &destination);

void profile::load(const std::string &filename) {
	std::ifstream file(filename);
	if (file.fail()) {
		throw std::runtime_error("Cannot use provided file");
	}

	std::optional<servo> servo_load = std::nullopt;
	std::optional<tdma> tdma_load = std::nullopt;
	std::optional<us> us_load = std::nullopt;
	std::optional<turn> turn_load = std::nullopt;

	std::string line;
	while (std::getline(file, line)) {
		if (line == CHECK_SERVO) {
			servo_load = std::make_optional<servo>();

			load_field(file, CHECK_SERVO_MAX_LEFT, servo_load->max_left);
			load_field(file, CHECK_SERVO_MAX_RIGHT, servo_load->max_right);
			load_field(file, CHECK_SERVO_CENTER, servo_load->center);
		}
		if (line == CHECK_TDMA) {
			tdma_load = std::make_optional<tdma>();

			load_field(file, CHECK_TDMA_TX_OFFSET_MS, tdma_load->tx_offset_ms);
			load_field(file, CHECK_TDMA_RX_OFFSET_MS, tdma_load->rx_offset_ms);
		}
		if (line == CHECK_US) {
			us_load = std::make_optional<us>();

			load_field(file, CHECK_US_THRESHOLD, us_load->threshold);
		}
		if (line == CHECK_TURN) {
			turn_load = std::make_optional<turn>();

			load_field(file, CHECK_TURN_RIGHT_MS, turn_load->right_ms);
			load_field(
				file, CHECK_TURN_RIGHT_DELAY_MS, turn_load->right_delay_ms);
			load_field(file, CHECK_TURN_LEFT_MS, turn_load->left_ms);
			load_field(
				file, CHECK_TURN_LEFT_DELAY_MS, turn_load->left_delay_ms);
		}
	}

	servo_profile = servo_load;
	tdma_profile = tdma_load;
	us_profile = us_load;
	turn_profile = turn_load;
}

void profile::save(const std::string &filename) const {
	std::ofstream file(filename);
	if (file.fail()) {
		throw std::runtime_error("Cannot use provided file");
	}

	if (servo_profile.has_value()) {
		file << CHECK_SERVO << '\n';
		file << CHECK_SERVO_MAX_LEFT << ' ' << servo_profile->max_left << '\n';
		file << CHECK_SERVO_MAX_RIGHT << ' ' << servo_profile->max_right
			 << '\n';
		file << CHECK_SERVO_CENTER << ' ' << servo_profile->center << '\n';
	}
	if (tdma_profile.has_value()) {
		file << CHECK_TDMA << '\n';
		file << CHECK_TDMA_TX_OFFSET_MS << ' ' << tdma_profile->rx_offset_ms
			 << '\n';
		file << CHECK_TDMA_RX_OFFSET_MS << ' ' << tdma_profile->tx_offset_ms
			 << '\n';
	}
	if (us_profile.has_value()) {
		file << CHECK_US << '\n';
		file << CHECK_US_THRESHOLD << ' ' << us_profile->threshold << '\n';
	}
	if (turn_profile.has_value()) {
		file << CHECK_TURN << '\n';
		file << CHECK_TURN_RIGHT_MS << ' ' << turn_profile->right_ms << '\n';
		file << CHECK_TURN_RIGHT_DELAY_MS << ' ' << turn_profile->left_delay_ms
			 << '\n';
		file << CHECK_TURN_LEFT_MS << ' ' << turn_profile->left_ms << '\n';
		file << CHECK_TURN_LEFT_DELAY_MS << ' ' << turn_profile->left_delay_ms
			 << '\n';
	}
}

/**
 * @brief Try to load a field from line.
 *
 * @tparam T Destination type.
 * @param[in] line - Input line.
 * @param[in] check - Verification condition.
 * @param[out] destination - Write location.
 */
template<typename T>
void load_field(std::ifstream &file, const std::string &check, T &destination) {
	// Load line
	std::string line;
	std::getline(file, line);

	// Get check value
	std::istringstream tokens(line);
	std::string file_check;
	tokens >> file_check;

	if (file_check == check) {
		tokens >> destination;
	} else {
		throw std::runtime_error("Corrupted profile");
	}
}
