#include <chrono>
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
<<<<<<< HEAD
	auto rf_test = std::make_shared<drf7020d20>(
		gpio_pins, RASPI_40, RASPI_37, RASPI_38, 0);
=======
	auto rf_test = std::make_shared<drf7020d20>(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0);
>>>>>>> upstream/timeslots

	rf_test->enable();
	rf_test->configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

<<<<<<< HEAD
	msg_t ack_msg = {
		.caller_id = "1",
		.receiver_id = "1",
		.body = "ACK",
	};
=======
	tdma tdma_test(rf_test, 0, tdma::AIR_A);

	while (true) {
		std::cout << tdma_test.rx_sync(1000) << '\n';
		std::cout << std::chrono::system_clock::now().time_since_epoch().count() / 1000000 << '\n';
	}
>>>>>>> upstream/timeslots

	return 0;
}
