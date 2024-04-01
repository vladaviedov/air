/**
 * @file include/rc552.hpp
 * @brief Drivers for RC552 RFID reader
 */
#pragma once

#include <atomic>
#include <cstdint>
#include <functional>
#include <memory>
#include <thread>

#include <gpiod.hpp>

#include "i2c.hpp"

class rc552 {
public:
	/**
	 * @brief Initializer
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] inter_pin - Interrupt libgpiod pin number.
	 * @param[in] dev_addr - I2C device address.
	 * @param[in] adapter - I2C ioctl adapter number.
	 */
	rc552(const gpiod::chip &chip,
		uint32_t inter_pin,
		uint8_t dev_addr,
		int adapter);

	~rc552();

	/**
	 * @brief Call callback on interrupt.
	 *
	 * @param[in] callback - Function to call on interrupt
	 */
	void on_interrupt(std::function<void()> callback);

	/**
	 * @brief Read RFID information
	 *
	 * @param[in] data - Buffer to store read information
	 * @return Number of bytes read
	 */
	int read_block(uint8_t blockAddr, uint8_t *data, uint8_t bufferSize);

	// Pointer to the data to transfer to the FIFO for CRC calculation.
	// The number of bytes to transfer.
	// Pointer to result buffer. Result is written to result[0..1], low byte
	// first.
	int calculateCRC(uint8_t *data, uint8_t length, uint8_t *result);

	int transcieveData(
		uint8_t *sendData,  ///< Pointer to the data to transfer to the FIFO.
		uint8_t sendLen,    ///< Number of bytes to transfer to the FIFO.
		uint8_t *backData,  ///< nullptr or pointer to buffer if data should be
							///< read back after executing the command.
		uint8_t *backLen,   ///< In: Max number of bytes to write to *backData.
							///< Out: The number of bytes returned.
		uint8_t *validBits, ///< In/Out: The number of valid bits in the last
							///< byte. 0 for 8 valid bits. Default nullptr.
		uint8_t rxAlign,    ///< In: Defines the bit position in backData[0] for
							///< the first bit received. Default 0.
		bool checkCRC ///< In: True => The last two bytes of the response is
					  ///< assumed to be a CRC_A that must be validated.
	);

	int communicateWithCard(
		uint8_t
			command, ///< The command to execute. One of the PCD_Command enums.
		uint8_t waitIRq,    ///< The bits in the ComIrqReg register that signals
							///< successful completion of the command.
		uint8_t *sendData,  ///< Pointer to the data to transfer to the FIFO.
		uint8_t sendLen,    ///< Number of bytes to transfer to the FIFO.
		uint8_t *backData,  ///< nullptr or pointer to buffer if data should be
							///< read back after executing the command.
		uint8_t *backLen,   ///< In: Max number of bytes to write to *backData.
							///< Out: The number of bytes returned.
		uint8_t *validBits, ///< In/Out: The number of valid bits in the last
							///< byte. 0 for 8 valid bits.
		uint8_t rxAlign,    ///< In: Defines the bit position in backData[0] for
							///< the first bit received. Default 0.
		bool checkCRC ///< In: True => The last two bytes of the response is
					  ///< assumed to be a CRC_A that must be validated.
	);
	/**
	 * Sets the bits given in mask in register reg.
	 */
	void setRegisterBitMask(
		uint8_t reg, ///< The register to update. One of the PCD_Register enums.
		uint8_t mask ///< The bits to set.
	);

private:
	gpiod::line interrupt;
	std::unique_ptr<std::thread> interrupt_thread;
	std::atomic_bool active = true;
	i2c i2cd;
};
