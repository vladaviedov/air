#include <iostream>

#include <gpiod.hpp>

#include <driver/device.hpp>
#include <driver/drf7020d20.hpp>
#include <driver/pinmap.hpp>
#include <shared/messages.hpp>
#include <shared/tdma.hpp>

int main() {
	/* auto rf_test = std::make_shared<drf7020d20>(gpio_pins, RASPI_40, RASPI_37, RASPI_38, 0); */
	auto rf_test = std::make_shared<drf7020d20>(gpio_pins, RASPI_12, RASPI_16, RASPI_18, 0);

	rf_test->enable();
	rf_test->configure(
		433900, drf7020d20::DR9600, 9, drf7020d20::DR9600, drf7020d20::NONE);

	tdma tdma_slot(rf_test, 0, tdma::AIR_A);
	tdma_slot.rx_set_offset(-5);
	tdma_slot.tx_set_offset(-70);

	while (true) {
		tdma_slot.tx_sync(*get_id());
		std::cout << tdma_slot.rx_sync(15) << std::endl;
	}

	return 0;
}
