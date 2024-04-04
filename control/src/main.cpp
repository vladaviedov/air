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
	auto rf_test = std::make_shared<drf7020d20>(gpio_pins, RASPI_12, RASPI_11, RASPI_7, 0);

	rf_test->enable();
	rf_test->configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	auto ex0 = [&](){
		tdma tdma_slot0(rf_test, 0, tdma::AIR_A);
		tdma_slot0.rx_set_offset(-5);
		tdma_slot0.tx_set_offset(-70);

		while (true) {
			std::cout << tdma_slot0.rx_sync(1000000) << '\n';
			tdma_slot0.tx_sync("KC1TNB/SL0");
		}
	};
	auto ex1 = [&](){
		tdma tdma_slot2(rf_test, 2, tdma::AIR_A);
		tdma_slot2.rx_set_offset(-5);
		tdma_slot2.tx_set_offset(-70);

		while (true) {
			std::cout << tdma_slot2.rx_sync(1000000) << '\n';
			tdma_slot2.tx_sync("KC1TNB/SL2");
		}
	};

	auto th0 = std::thread(ex0);
	auto th1 = std::thread(ex1);

	th0.join();
	th1.join();

	return 0;
}
