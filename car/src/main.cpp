#include <cstdio>
#include <iostream>
#include <gpiod.hpp>
#include <driver/pwm.hpp>
#include <driver/pinmap.hpp>

int main() {
	gpiod::chip chip("gpiochip0");
	pwm_worker pwm_test(chip, RASPI_3);
	pwm_test.set_duty(50);
	std::getchar();
	return 0;
}
