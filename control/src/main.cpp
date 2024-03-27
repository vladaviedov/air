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

int main() {
	drf7020d20 rf_test(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0);
	rf_test.enable();
	rf_test.configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	msg_t ack_msg = {
		.caller_id = "1",
		.receiver_id = "1",
		.body = "ACK",
	};

	return 0;
}
