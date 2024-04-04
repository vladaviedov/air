#include "lineguider.hpp"

#include <chrono>
#include <driver/device.hpp>
#include <driver/pinmap.hpp>

#define SERVO_MID 84
#define SERVO_RIGHT 125
#define SERVO_LEFT 40

#define TURN_INCREMENT 5 /*degrees*/
#define TURNING_OFFSET 5 /*milis*/
#define DIFF_OFFSET 100  /*milis*/

lineguider::lineguider(std::shared_ptr<motor> m_in,
	std::shared_ptr<servo> s_in,
	std::shared_ptr<light_sens> sens_l_in,
	std::shared_ptr<light_sens> sens_r_in)
	: m(m_in),
	  s(s_in),
	  sens_l(sens_l_in),
	  sens_r(sens_r_in) {
	degrees = SERVO_MID;
	drive_dir = STRAIGHT;
}

void lineguider::read_hits() {
	if (sens_l->read()) {
		hit_time_l = std::chrono::system_clock::now();
		degrees = SERVO_RIGHT;
	}
	if (sens_r->read()) {
		hit_time_r = std::chrono::system_clock::now();
		degrees = SERVO_LEFT;
	}

	auto diff = std::chrono::duration_cast<std::chrono::milliseconds>(
		hit_time_r - hit_time_l); // closer to infinity the better
	hit_diffs.push_back(diff);
}

void lineguider::guide() {
	std::size_t size = hit_diffs.size();
	int latest_diff = hit_diffs[size - 1].count();
	int previous_diff = hit_diffs[size - 1].count();
	if (latest_diff > previous_diff - DIFF_OFFSET &&
		latest_diff < previous_diff + DIFF_OFFSET) {
		// keep turning direction
	} else if (latest_diff < previous_diff - DIFF_OFFSET) {
		if (drive_dir == LEFT) {
			degrees += TURN_INCREMENT; // go to right
		} else if (drive_dir == RIGHT) {
			degrees -= TURN_INCREMENT; // go to left
		}
		// what to do for straight case?
	}
}

void lineguider::get_turning_direction() {
	if (degrees < SERVO_LEFT) {
		degrees = SERVO_LEFT;
	}

	if (degrees > SERVO_RIGHT) {
		degrees = SERVO_RIGHT;
	}

	if (degrees > SERVO_MID + TURNING_OFFSET) {
		drive_dir = RIGHT;
	} else if (degrees < SERVO_MID - TURNING_OFFSET) {
		drive_dir = LEFT;
	} else {
		drive_dir = STRAIGHT;
	}
}
