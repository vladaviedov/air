#include <cstdio>
#include <iostream>
#include <gpiod.hpp>
#include <driver/pinmap.hpp>
#include <driver/drf7020d20.hpp>

int main() {
	gpiod::chip chip("gpiochip0");
	drf7020d20 rf_test(chip, RASPI_40, RASPI_37, RASPI_38, 0);
	rf_test.enable();
	rf_test.configure(433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);
	return 0;
}
