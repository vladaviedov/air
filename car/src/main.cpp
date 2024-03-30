#include <chrono>
#include <cstdio>
#include <iostream>
#include <string>
#include <thread>

#include <gpiod.hpp>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/pinmap.hpp>
#include <shared/messages.hpp>
#include <shared/tdma.hpp>

int main() {
	auto rf_test = std::make_shared<drf7020d20>(gpio_pins, RASPI_40, RASPI_37, RASPI_38, 0);

	rf_test->enable();
	rf_test->configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	return 0;
}
