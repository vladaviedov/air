/**
 * @file include/rc552.cpp
 * @brief Drivers for RC552 RFID reader
 */

#include "rc552.hpp"

#include <chrono>
#include <cstdint>

#include "defines.hpp"

static constexpr uint8_t commandReg = 0x01;
static constexpr uint8_t comIrqReg = 0x04;
static constexpr uint8_t divIqrReg = 0x05;
static constexpr uint8_t errorReg = 0x06;
static constexpr uint8_t status1Reg = 0x07;
static constexpr uint8_t FIFODataReg = 0x09;
static constexpr uint8_t FIFOLevelReg = 0x0A;
static constexpr uint8_t controlReg = 0x0C;
static constexpr uint8_t CRCResultRegH = 0x21;
static constexpr uint8_t CRCResultRegL = 0x22;
static constexpr uint8_t bitFramingReg = 0x0D;

// Commands to write to command register
static constexpr uint8_t idle = 0x0;
static constexpr uint8_t mem = 0b0001;
static constexpr uint8_t genRandomId = 0b0010;
static constexpr uint8_t calcCRC = 0b0011;
static constexpr uint8_t transmit = 0b0100;
static constexpr uint8_t noCmd = 0b0111;
static constexpr uint8_t receive = 0b1000;
static constexpr uint8_t transcieve = 0b1100;
static constexpr uint8_t mfAuthent = 0b1110;

// MIFARE commands
static constexpr uint8_t PICC_CMD_MF_READ = 0x30;

rc552::rc552(
	const gpiod::chip &chip, uint32_t inter_pin, uint8_t dev_addr, int adapter)
	: interrupt(chip.get_line(inter_pin)),
	  i2cd(i2c(dev_addr, adapter)) {
	interrupt.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::EVENT_RISING_EDGE,
		.flags = 0,
	});
}

rc552::~rc552() {
	active = false;
	interrupt_thread->join();
	interrupt.release();
}

void rc552::on_interrupt(std::function<void()> callback) {
	// Thread function
	auto executor = [&]() {
		while (active) {
			if (interrupt.event_wait(std::chrono::milliseconds(100))) {
				interrupt.event_read();
				callback();
			}
		}
	};

	interrupt_thread = std::make_unique<std::thread>(executor);
}

int rc552::read_block(uint8_t blockAddr, uint8_t *data, uint8_t bufferSize) {
	// Sanity check
	if (data == nullptr || bufferSize < 18) {
		return 4;
	}

	// Build command buffer
	data[0] = PICC_CMD_MF_READ;
	data[1] = blockAddr;
	// calc 16 bit crc
	int result = calculateCRC(data, 2, &data[2]);
	if (result != 0) {
		return result;
	}

	// Transmit the buffer and receive the response, validate CRC_A.
	return transcieveData(data, 4, data, &bufferSize, nullptr, 0, true);
}

int rc552::transcieveData(
	uint8_t *sendData,  ///< Pointer to the data to transfer to the FIFO.
	uint8_t sendLen,    ///< Number of bytes to transfer to the FIFO.
	uint8_t *backData,  ///< nullptr or pointer to buffer if data should be read
						///< back after executing the command.
	uint8_t *backLen,   ///< In: Max number of bytes to write to *backData. Out:
						///< The number of bytes returned.
	uint8_t *validBits, ///< In/Out: The number of valid bits in the last byte.
						///< 0 for 8 valid bits. Default nullptr.
	uint8_t rxAlign,    ///< In: Defines the bit position in backData[0] for the
						///< first bit received. Default 0.
	bool checkCRC ///< In: True => The last two bytes of the response is assumed
				  ///< to be a CRC_A that must be validated.
) {
	uint8_t waitIRq = 0x30; // RxIRq and IdleIRq
	return communicateWithCard(transcieve, waitIRq, sendData, sendLen, backData,
		backLen, validBits, rxAlign, checkCRC);
}

int rc552::communicateWithCard(
	uint8_t command, ///< The command to execute. One of the PCD_Command enums.
	uint8_t waitIRq, ///< The bits in the ComIrqReg register that signals
					 ///< successful completion of the command.
	uint8_t *sendData,  ///< Pointer to the data to transfer to the FIFO.
	uint8_t sendLen,    ///< Number of bytes to transfer to the FIFO.
	uint8_t *backData,  ///< nullptr or pointer to buffer if data should be read
						///< back after executing the command.
	uint8_t *backLen,   ///< In: Max number of bytes to write to *backData. Out:
						///< The number of bytes returned.
	uint8_t *validBits, ///< In/Out: The number of valid bits in the last byte.
						///< 0 for 8 valid bits.
	uint8_t rxAlign,    ///< In: Defines the bit position in backData[0] for the
						///< first bit received. Default 0.
	bool checkCRC ///< In: True => The last two bytes of the response is assumed
				  ///< to be a CRC_A that must be validated.
) {
	// Prepare values for BitFramingReg
	uint8_t txLastBits = validBits ? *validBits : 0;
	uint8_t bitFraming =
		(rxAlign << 4) + txLastBits; // RxAlign = BitFramingReg[6..4].
									 // TxLastBits = BitFramingReg[2..0]

	i2cd.write(commandReg, &idle, 1); // Stop any active command.
	uint8_t clearIrq = 0x7F;
	// Clear all seven interrupt request bits
	i2cd.write(comIrqReg, &clearIrq, 1);
	uint8_t flushFifo = 0x80;
	// FlushBuffer = 1, FIFO initialization
	i2cd.write(FIFOLevelReg, &flushFifo, 1);
	i2cd.write(FIFODataReg, sendData, sendLen); // Write sendData to the FIFO
	i2cd.write(bitFramingReg, &bitFraming, 1);  // Bit adjustments
	i2cd.write(commandReg, &command, 1);        // Execute the command
	if (command == transcieve) {
		// StartSend=1, transmission of data starts
		setRegisterBitMask(bitFramingReg, 0x80);
	}

	// In PCD_Init() we set the TAuto flag in TModeReg. This means the timer
	// automatically starts when the PCD stops transmitting.
	//
	// Wait here for the command to complete. The bits specified in the
	// `waitIRq` parameter define what bits constitute a completed command.
	// When they are set in the ComIrqReg register, then the command is
	// considered complete. If the command is not indicated as complete in
	// ~36ms, then consider the command as timed out.
	bool completed = false;
	std::this_thread::sleep_for(std::chrono::milliseconds(36));

	uint8_t n = i2cd.read_byte(
		comIrqReg); // ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq
					// HiAlertIRq LoAlertIRq ErrIRq TimerIRq
	if (n &
		waitIRq) { // One of the interrupts that signal success has been set.
		completed = true;
	}
	if (n & 0x01) { // Timer interrupt - nothing received in 25ms
		return 1;
	}

	// 36ms and nothing happened. Communication with the MFRC522 might be down.
	if (!completed) {
		return 1;
	}

	// Stop now if any errors except collisions were detected.
	// ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl
	uint8_t errorRegValue = i2cd.read_byte(errorReg);
	// CollErr CRCErr ParityErr ProtocolErr
	if (errorRegValue & 0x13) { // BufferOvfl ParityErr ProtocolErr
		return 1;
	}

	uint8_t _validBits = 0;

	// If the caller wants data back, get it from the MFRC522.
	if (backData && backLen) {
		uint8_t n = i2cd.read_byte(FIFOLevelReg); // Number of bytes in the FIFO
		if (n > *backLen) {
			return 2; // no room
		}
		*backLen = n;                            // Number of bytes returned
		*backData = i2cd.read_byte(FIFODataReg); // Get received data from FIFO
		// RxLastBits[2:0] indicates the number of valid bits
		_validBits = uint8_t(controlReg) & 0x07;
		// in the last received byte. If this value is 000b,
		// the whole byte is valid.
		if (validBits) {
			*validBits = _validBits;
		}
	}

	// Tell about collisions
	if (errorRegValue & 0x08) { // CollErr
		return 3;
	}

	// Perform CRC_A validation if requested.
	if (backData && backLen && checkCRC) {
		// In this case a MIFARE Classic NAK is not OK.
		if (*backLen == 1 && _validBits == 4) {
			return 3;
		}
		// We need at least the CRC_A value and all 8 bits of the last byte must
		// be received.
		if (*backLen < 2 || _validBits != 0) {
			return 4;
		}
		// Verify CRC_A - do our own calculation and store the control in
		// controlBuffer.
		uint8_t controlBuffer[2];
		int status =
			calculateCRC(&backData[0], *backLen - 2, &controlBuffer[0]);
		if (status != 0) {
			return status;
		}
		if ((backData[*backLen - 2] != controlBuffer[0]) ||
			(backData[*backLen - 1] != controlBuffer[1])) {
			return 4;
		}
	}

	return 0;
} // End PCD_CommunicateWithPICC()

int rc552::calculateCRC(uint8_t *data, ///< In: Pointer to the data to transfer
									   ///< to the FIFO for CRC calculation.
	uint8_t length,                    ///< In: The number of bytes to transfer.
	uint8_t *result ///< Out: Pointer to result buffer. Result is written to
					///< result[0..1], low byte first.
) {
	uint8_t clear_inters = 0x04;
	uint8_t flush_buffer = 0x80;
	i2cd.write(commandReg, &idle, 1); // Stop any active command.
									  // Clear the CRCIRq interrupt request bit
	i2cd.write(divIqrReg, &clear_inters, 1);
	// FlushBuffer = 1, FIFO initialization
	i2cd.write(FIFOLevelReg, &flush_buffer, 1);
	i2cd.write(FIFODataReg, data, length); // Write data to the FIFO
	i2cd.write(commandReg, &calcCRC, 1);   // Start the calculation

	// Wait for the CRC calculation to complete. Check for the register to
	// indicate that the CRC calculation is complete in a loop. If the
	// calculation is not indicated as complete in ~90ms, then time out
	// the operation.

	// 89ms passed and nothing happened. Communication with the MFRC522 might be
	// down.
	std::this_thread::sleep_for(std::chrono::milliseconds(89));

	// TODO: Set a timeout.
	// DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq reserved
	// CRCIRq reserved reserved
	uint8_t n = i2cd.read_byte(divIqrReg);
	if (n & 0x04) { // CRCIRq bit set - calculation done
					// Stop calculating CRC for new content in the FIFO.
		i2cd.write(commandReg, &idle, 1);
		// Transfer the result from the registers to the result buffer
		result[0] = i2cd.read_byte(CRCResultRegL);
		result[1] = i2cd.read_byte(CRCResultRegH);
		return 0;
	}

	return 1;
}

/**
 * Sets the bits given in mask in register reg.
 */
void rc552::setRegisterBitMask(
	uint8_t reg, ///< The register to update. One of the PCD_Register enums.
	uint8_t mask ///< The bits to set.
) {
	uint8_t tmp;
	tmp = i2cd.read_byte(reg);
	i2cd.write(reg, (uint8_t *)(tmp | mask), 1); // set bit mask
}
