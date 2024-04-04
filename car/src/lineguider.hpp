#pragma once

#include <stdlib.h>

#include <driver/lightsens.hpp>
#include <driver/motors.hpp>
#include <driver/servo.hpp>

class lineguider {
public:
	enum direction {
		RIGHT,
		LEFT,
		STRAIGHT
	};

	lineguider(std::shared_ptr<motor> motor_in,
		std::shared_ptr<servo> s_in,
		std::shared_ptr<light_sens> sens_r_in,
		std::shared_ptr<light_sens> sens_l_in);
	void read_hits();
	void guide();
	void get_turning_direction();

private:
	std::shared_ptr<motor> m;
	std::shared_ptr<servo> s;
	std::shared_ptr<light_sens> sens_r;
	std::shared_ptr<light_sens> sens_l;

	std::vector<std::chrono::milliseconds> hit_diffs;
	std::chrono::_V2::system_clock::time_point hit_time_l;
	std::chrono::_V2::system_clock::time_point hit_time_r;
	direction drive_dir;
	uint8_t degrees;
};
