/**
 * @file src/rc552.cpp
 * @brief Drivers for RC552 RFID reader
 */

#include "rc552.hpp"

#include <chrono>
#include <cstdint>
#include <cstring>

#include "defines.hpp"

static constexpr uint8_t commandReg = 0x01 << 1;
// static constexpr uint8_t commIEnReg = 0x02 << 1;
static constexpr uint8_t comIrqReg = 0x04 << 1;
static constexpr uint8_t divIqrReg = 0x05 << 1;
static constexpr uint8_t errorReg = 0x06 << 1;
// static constexpr uint8_t status1Reg = 0x07;
static constexpr uint8_t Status2Reg =
	0x08 << 1; // receiver and transmitter status bits
static constexpr uint8_t FIFODataReg = 0x09 << 1;
static constexpr uint8_t FIFOLevelReg = 0x0A << 1;
static constexpr uint8_t controlReg = 0x0C << 1;
static constexpr uint8_t TxModeReg = 0x12 << 1;
static constexpr uint8_t RxModeReg = 0x13 << 1;
static constexpr uint8_t CRCResultRegH = 0x21 << 1;
static constexpr uint8_t CRCResultRegL = 0x22 << 1;
static constexpr uint8_t ModWidthReg = 0x24 << 1;
static constexpr uint8_t bitFramingReg = 0x0D << 1;
// bit position of the first bit-collision detected on the RF interface
static constexpr uint8_t collReg = 0x0E << 1;

// Commands to write to command register
static constexpr uint8_t idle = 0x0;
// static constexpr uint8_t mem = 0b0001;
// static constexpr uint8_t genRandomId = 0b0010;
static constexpr uint8_t calcCRC = 0b0011;
// static constexpr uint8_t transmit = 0b0100;
// static constexpr uint8_t noCmd = 0b0111;
// static constexpr uint8_t receive = 0b1000;
static constexpr uint8_t transcieve = 0b1100;
static constexpr uint8_t PCD_MFAuthent =
	0x0E; // performs the MIFARE standard authentication as a reader

// MIFARE commands

// REQuest command, Type A. Invites PICCs in state IDLE to go to READY and
// prepare for anticollision or selection. 7 bit frame.
static constexpr uint8_t PICC_CMD_REQA = 0x26;
static constexpr uint8_t PICC_CMD_MF_READ = 0x30;
static constexpr uint8_t PICC_CMD_HLTA =
	0x50; // HaLT command, Type A. Instructs an ACTIVE PICC to go to state HALT.
static constexpr uint8_t PICC_CMD_MF_AUTH_KEY_A =
	0x60; // Perform authentication with Key A

static constexpr int STATUS_OK = 0;
static constexpr int STATUS_TIMEOUT = 1;
static constexpr int STATUS_ERROR = 2;

rc552::rc552(const gpiod::chip &chip, uint32_t inter_pin, std::string adapter)
	: uid(4, {0}, 0),
	  interrupt(chip.get_line(inter_pin)),
	  spid(spi(0, 8, 4000000U, adapter.data())) {
	interrupt.request({
		.consumer = GPIO_CONSUMER,
		.request_type = gpiod::line_request::EVENT_RISING_EDGE,
		.flags = 0,
	});

	uint8_t res = spid.read_byte(errorReg);
	if (res != 0x80) {
		throw std::runtime_error("RC552 spi read failure");
	}
}

rc552::~rc552() {
	active = false;
	// interrupt_thread->join();
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

int rc552::MIFARE_Read(uint8_t blockAddr, uint8_t *data, uint8_t *bufferSize) {
	// Sanity check
	if (data == nullptr || *bufferSize < 18) {
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
	return transcieveData(data, 4, data, bufferSize, nullptr, 0, true);
}

int rc552::transcieveData(uint8_t *sendData,
	uint8_t sendLen,
	uint8_t *backData,
	uint8_t *backLen,
	uint8_t *validBits = nullptr,
	uint8_t rxAlign = 0,
	bool checkCRC = false) {
	uint8_t waitIRq = 0x30; // RxIRq and IdleIRq
	return PCD_CommunicateWithPICC(transcieve, waitIRq, sendData, sendLen,
		backData, backLen, validBits, rxAlign, checkCRC);
}

int rc552::PCD_CommunicateWithPICC(uint8_t command,
	uint8_t waitIRq,
	uint8_t *sendData,
	uint8_t sendLen,
	uint8_t *backData,
	uint8_t *backLen,
	uint8_t *validBits,
	uint8_t rxAlign,
	bool checkCRC

) {
	// Prepare values for BitFramingReg
	uint8_t txLastBits;
	if (validBits != nullptr) {
		txLastBits = *validBits;
	} else {
		txLastBits = 0;
	}
	uint8_t bitFraming =
		(rxAlign << 4) + txLastBits; // RxAlign = BitFramingReg[6..4].
									 // TxLastBits = BitFramingReg[2..0]

	spid.write_byte(commandReg, idle); // Stop any active command.
	uint8_t clearIrq = 0x7F;
	// Clear all seven interrupt request bits
	spid.write_byte(comIrqReg, clearIrq);
	uint8_t flushFifo = 0x80;
	// FlushBuffer = 1, FIFO initialization
	spid.write_byte(FIFOLevelReg, flushFifo);
	spid.write(FIFODataReg, sendData, sendLen); // Write sendData to the FIFO
	spid.write_byte(bitFramingReg, bitFraming); // Bit adjustments
	spid.write_byte(commandReg, command);       // Execute the command
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

	// ComIrqReg[7..0] bits are: Set1 TxIRq RxIRq IdleIRq
	// HiAlertIRq LoAlertIRq ErrIRq TimerIRq
	uint8_t irqReg = spid.read_byte(comIrqReg);

	// One of the interrupts that signal success has been set.
	if ((irqReg & waitIRq) > 0) {
		completed = true;
	}

	if ((irqReg & 0x01) > 0) { // Timer interrupt - nothing received in 25ms
		return 1;
	}

	// 36ms and nothing happened. Communication with the MFRC522 might be down.
	if (!completed) {
		return 2;
	}

	// Stop now if any errors except collisions were detected.
	// ErrorReg[7..0] bits are: WrErr TempErr reserved BufferOvfl
	uint8_t errorRegValue = spid.read_byte(errorReg);
	// CollErr CRCErr ParityErr ProtocolErr
	if ((errorRegValue & 0x13) > 0) { // BufferOvfl ParityErr ProtocolErr
		return 3;
	}

	uint8_t _validBits = 0;

	// If the caller wants data back, get it from the MFRC522.
	if (backData != nullptr && backLen != nullptr) {
		uint8_t fifoLevel =
			spid.read_byte(FIFOLevelReg); // Number of bytes in the FIFO
		if (fifoLevel > *backLen) {
			return 5; // no room
		}
		*backLen = fifoLevel; // Number of bytes returned
		spid.read(
			FIFODataReg, backData, fifoLevel); // Get received data from FIFO
		// RxLastBits[2:0] indicates the number of valid bits
		_validBits = uint8_t(controlReg) & 0x07;
		// in the last received byte. If this value is 000b,
		// the whole byte is valid.
		if (validBits != nullptr) {
			*validBits = _validBits;
		}
	}

	// Tell about collisions
	if ((errorRegValue & 0x08) > 0) { // CollErr
		return 6;
	}

	// Perform CRC_A validation if requested.
	if (backData != nullptr && backLen != nullptr && checkCRC) {
		int res = validateCRCA(backData, backLen, &_validBits);
		if (res != 0) {
			return res;
		}
	}

	return 0;
}

int rc552::calculateCRC(uint8_t *data, uint8_t length, uint8_t *result) {
	uint8_t clear_inters = 0x04;
	uint8_t flush_buffer = 0x80;
	// Stop any active command.
	spid.write_byte(commandReg, idle);
	// Clear the CRCIRq interrupt request bit
	spid.write_byte(divIqrReg, clear_inters);
	// FlushBuffer = 1, FIFO initialization
	spid.write_byte(FIFOLevelReg, flush_buffer);
	spid.write(FIFODataReg, data, length); // Write data to the FIFO
	spid.write_byte(commandReg, calcCRC);  // Start the calculation

	// Wait for the CRC calculation to complete. Check for the register to
	// indicate that the CRC calculation is complete in a loop. If the
	// calculation is not indicated as complete in ~90ms, then time out
	// the operation.

	// 89ms passed and nothing happened. Communication with the MFRC522 might be
	// down.
	std::this_thread::sleep_for(std::chrono::milliseconds(89));

	// DivIrqReg[7..0] bits are: Set2 reserved reserved MfinActIRq reserved
	// CRCIRq reserved reserved
	uint8_t divIrq = spid.read_byte(divIqrReg);
	if ((divIrq & 0x04) > 0) { // CRCIRq bit set - calculation done
		// Stop calculating CRC for new content in the FIFO.
		spid.write_byte(commandReg, idle);
		// Transfer the result from the registers to the result buffer
		result[0] = spid.read_byte(CRCResultRegL);
		result[1] = spid.read_byte(CRCResultRegH);
		return 0;
	}

	return 7;
}

/**
 * Sets the bits given in mask in register reg.
 */
void rc552::setRegisterBitMask(uint8_t reg, uint8_t mask) {
	uint8_t tmp;
	tmp = spid.read_byte(reg);
	uint8_t bitMask = (tmp | mask);
	spid.write_byte(reg, bitMask); // set bit mask
}

int rc552::validateCRCA(
	uint8_t *backData, const uint8_t *backLen, const uint8_t *_validBits) {
	// In this case a MIFARE Classic NAK is not OK.
	if (*backLen == 1 && *_validBits == 4) {
		return 8;
	}
	// We need at least the CRC_A value and all 8 bits of the last byte must
	// be received.
	if (*backLen < 2 || *_validBits != 0) {
		return 9;
	}
	// Verify CRC_A - do our own calculation and store the control in
	// controlBuffer.
	uint8_t controlBuffer[2];
	int status = calculateCRC(&backData[0], *backLen - 2, &controlBuffer[0]);
	if (status != 0) {
		return status;
	}
	if ((backData[*backLen - 2] != controlBuffer[0]) ||
		(backData[*backLen - 1] != controlBuffer[1])) {
		return 10;
	}
	return 0;
}

/**
 * Returns true if a PICC responds to PICC_CMD_REQA.
 * Only "new" cards in state IDLE are invited. Sleeping cards in state HALT are
 * ignored.
 *
 * @return bool
 */
bool rc552::PICC_IsNewCardPresent() {
	uint8_t bufferATQA[2];
	uint8_t bufferSize = sizeof(bufferATQA);

	// Reset baud rates
	PCD_WriteRegister(TxModeReg, 0x00);
	PCD_WriteRegister(RxModeReg, 0x00);
	// Reset ModWidthReg
	PCD_WriteRegister(ModWidthReg, 0x26);

	int result = PICC_RequestA(bufferATQA, &bufferSize);
	return result == 0 || result == 1;
} // End PICC_IsNewCardPresent()

int rc552::PICC_RequestA(
	uint8_t
		*bufferATQA,    ///< The buffer to store the ATQA (Answer to request) in
	uint8_t *bufferSize ///< Buffer size, at least two bytes. Also number of
						///< bytes returned if STATUS_OK.
) {
	return PICC_REQA_or_WUPA(PICC_CMD_REQA, bufferATQA, bufferSize);
} // End PICC_RequestA()

int rc552::PICC_REQA_or_WUPA(
	uint8_t command, ///< The command to send - PICC_CMD_REQA or PICC_CMD_WUPA
	uint8_t
		*bufferATQA,    ///< The buffer to store the ATQA (Answer to request) in
	uint8_t *bufferSize ///< Buffer size, at least two bytes. Also number of
						///< bytes returned if STATUS_OK.
) {
	uint8_t validBits;
	int status;

	if (bufferATQA == nullptr ||
		*bufferSize < 2) { // The ATQA response is 2 bytes long.
		return 4;
	}

	// ValuesAfterColl=1 => Bits received after collision
	// are cleared.
	PCD_ClearRegisterBitMask(collReg, 0x80);
	validBits =
		7; // For REQA and WUPA we need the short frame format - transmit only 7
		   // bits of the last (and only) byte. TxLastBits = BitFramingReg[2..0]
	status = transcieveData(
		&command, 1, bufferATQA, bufferSize, &validBits, 0, false);
	if (status != 0) {
		return status;
	}
	if (*bufferSize != 2 || validBits != 0) { // ATQA must be exactly 16 bits.
		return 1;
	}
	return 0;
} // End PICC_REQA_or_WUPA()

/**
 * Clears the bits given in mask from register reg.
 */
void rc552::PCD_ClearRegisterBitMask(
	uint8_t reg, ///< The register to update. One of the PCD_Register enums.
	uint8_t mask ///< The bits to clear.
) {
	uint8_t tmp;
	spid.read(reg, &tmp, 1);
	uint8_t clear = tmp & (~mask);
	spid.write_byte(reg, clear); // clear bit mask
} // End PCD_ClearRegisterBitMask()

/**
 * Simple wrapper around PICC_Select.
 * Returns true if a UID could be read.
 * Remember to call PICC_IsNewCardPresent(), PICC_RequestA() or PICC_WakeupA()
 * first. The read UID is available in the class variable uid.
 *
 * @return bool
 */
bool rc552::PICC_ReadCardSerial() {
	int result = PICC_Select();
	return result == 0;
} // End

/**
 * Instructs a PICC in state ACTIVE(*) to go to state HALT.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise.
 */
int rc552::PICC_HaltA() {
	int result;
	uint8_t buffer[4];

	// Build command buffer
	buffer[0] = PICC_CMD_HLTA;
	buffer[1] = 0;
	// Calculate CRC_A
	result = calculateCRC(buffer, 2, &buffer[2]);
	if (result != STATUS_OK) {
		return result;
	}

	// Send the command.
	// The standard says:
	//		If the PICC responds with any modulation during a period of 1 ms
	// after the end of the frame containing the 		HLTA command, this
	// response shall be interpreted as 'not acknowledge'.
	// We interpret that this way: Only STATUS_TIMEOUT is a success.
	result = transcieveData(buffer, sizeof(buffer), nullptr, nullptr);
	if (result == STATUS_TIMEOUT) {
		return STATUS_OK;
	}
	if (result == STATUS_OK) { // That is ironically NOT ok in this case ;-)
		return STATUS_ERROR;
	}
	return result;
} // End PICC_HaltA()

/**
 * Used to exit the PCD from its authenticated state.
 * Remember to call this function after communicating with an authenticated PICC
 * - otherwise no new communications can start.
 */
void rc552::PCD_StopCrypto1() {
	// Clear MFCrypto1On bit
	// Status2Reg[7..0] bits are: TempSensClear I2CForceHS reserved reserved
	// MFCrypto1On ModemState[2:0]
	PCD_ClearRegisterBitMask(Status2Reg, 0x08);
} // End PCD_StopCrypto1()

/**
 * Dumps memory contents of a MIFARE Classic PICC.
 * On success the PICC is halted after dumping the data.
 */
void rc552::PICC_DumpMifareClassicToSerial() {
	uint8_t no_of_sectors = 16;
	uint8_t key[6];
	// All keys are set to FFFFFFFFFFFFh at chip delivery from the factory.
	for (uint8_t i = 0; i < 6; i++) {
		key[i] = 0xFF;
	}

	// Dump sectors, highest address first.
	printf("Sector Block   0  1  2  3   4  5  6  7   8  9 10 11  "
		   "12 13 14 15  AccessBits");
	for (int i = no_of_sectors - 1; i >= 0; i--) {
		PICC_DumpMifareClassicSectorToSerial(key, i);
	}
	PICC_HaltA(); // Halt the PICC before stopping the encrypted session.
	PCD_StopCrypto1();
} // End PICC_DumpMifareClassicToSerial()

/**
 * Dumps memory contents of a sector of a MIFARE Classic PICC.
 * Uses PCD_Authenticate(), MIFARE_Read() and PCD_StopCrypto1.
 * Always uses PICC_CMD_MF_AUTH_KEY_A because only Key A can always read the
 * sector trailer access bits.
 */
void rc552::PICC_DumpMifareClassicSectorToSerial(
	uint8_t *key,  ///< Key A for the sector.
	uint8_t sector ///< The sector to dump, 0..39.
) {
	uint8_t firstBlock; // Address of lowest address to dump actually last block
						// dumped)
	uint8_t no_of_blocks; // Number of blocks in sector
	bool isSectorTrailer; // Set to true while handling the "last" (ie highest
						  // address) in the sector.

	// Determine position and size of sector.
	if (sector < 32) { // Sectors 0..31 has 4 blocks each
		no_of_blocks = 4;
		firstBlock = sector * no_of_blocks;
	} else if (sector < 40) { // Sectors 32-39 has 16 blocks each
		no_of_blocks = 16;
		firstBlock = 128 + (sector - 32) * no_of_blocks;
	} else { // Illegal input, no MIFARE Classic PICC has more than 40 sectors.
		return;
	}

	// Dump blocks, highest address first.
	uint8_t buffer[18];
	uint8_t blockAddr;
	isSectorTrailer = true;

	for (int blockOffset = no_of_blocks - 1; blockOffset >= 0; blockOffset--) {
		blockAddr = firstBlock + blockOffset;

		MIFARE_dump_sector(blockAddr, isSectorTrailer, firstBlock, key, buffer,
			no_of_blocks, blockOffset);
	}

} // End PICC_DumpMifareClassicSectorToSerial()

void rc552::parseSectorTrailerData(bool *isSectorTrailer,
	bool *invertedError,
	const uint8_t *buffer,
	uint8_t *group) {
	if (*isSectorTrailer) {
		uint8_t acc1 = buffer[7] >> 4;
		uint8_t acc2 = buffer[8] & 0xF;
		uint8_t acc3 = buffer[8] >> 4;
		uint8_t acc1Inv = buffer[6] & 0xF;
		uint8_t acc2Inv = buffer[6] >> 4;
		uint8_t acc3Inv = buffer[7] & 0xF;
		*invertedError = (acc1 != (~acc1Inv & 0xF)) ||
						 (acc2 != (~acc2Inv & 0xF)) ||
						 (acc3 != (~acc3Inv & 0xF));
		group[0] = ((acc1 & 1) << 2) | ((acc2 & 1) << 1) | ((acc3Inv & 1) << 0);
		group[1] = ((acc1 & 2) << 1) | ((acc2 & 2) << 0) | ((acc3Inv & 2) >> 1);
		group[2] = ((acc1 & 4) << 0) | ((acc2 & 4) >> 1) | ((acc3Inv & 4) >> 2);
		group[3] = ((acc1 & 8) >> 1) | ((acc2 & 8) >> 2) | ((acc3Inv & 8) >> 3);
		*isSectorTrailer = false;
	}
}

void rc552::MIFARE_dump_sector(uint8_t blockAddr,
	bool isSectorTrailer,
	uint8_t firstBlock,
	uint8_t *key,
	uint8_t *buffer,
	uint8_t no_of_blocks,
	uint8_t blockOffset) {
	bool invertedError = false; // Avoid "unused variable" warning.

	// The access bits are stored in a peculiar fashion.
	// There are four groups:
	//		g[3]	Access bits for the sector trailer, block 3 (for sectors
	// 0-31) or block 15 (for sectors 32-39) 		g[2]	Access bits for
	// block 2 (for sectors 0-31) or blocks 10-14 (for sectors 32-39) 		g[1]
	// Access bits for block 1 (for sectors 0-31) or blocks 5-9 (for sectors
	// 32-39) 		g[0]	Access bits for block 0 (for sectors 0-31) or blocks
	// 0-4 (for sectors 32-39)
	// Each group has access bits [C1 C2 C3]. In this code C1 is MSB and C3 is
	// LSB. The four CX bits are stored together in a nible cx and an inverted
	// nible cx_.
	uint8_t gAcc[4]; // Access bits for each of the four groups.
	uint8_t group;   // 0-3 - active group for access bits

	printf("%d  ", blockAddr);

	// Establish encrypted communications before reading the first block
	if (isSectorTrailer) {
		int status = PCD_Authenticate(PICC_CMD_MF_AUTH_KEY_A, firstBlock, key);
		if (status != STATUS_OK) {
			printf("PCD_Authenticate() failed: ");
			printf("%d", status);
			return;
		}
	}

	// Read block
	uint8_t byteCount = sizeof(buffer);
	int status = MIFARE_Read(blockAddr, buffer, &byteCount);
	if (status != STATUS_OK) {
		throw std::runtime_error("MIFARE READ FAIL");
	}

	// Dump data
	for (uint8_t index = 0; index < 16; index++) {
		printf("%X ", buffer[index]);
	}

	// Parse sector trailer data
	parseSectorTrailerData(&isSectorTrailer, &invertedError, buffer, gAcc);

	bool firstInGroup; // True for the first block dumped in the group

	// Which access group is this block in?
	if (no_of_blocks == 4) {
		group = blockOffset;
		firstInGroup = true;
	} else {
		group = blockOffset / 5;
		firstInGroup = (group == 3) || (group != (blockOffset + 1) / 5);
	}

	if (firstInGroup) {
		// Print access bits
		printf(" [ ");
		printf("%d", (gAcc[group] >> 2) & 1);
		printf(" ");
		printf("%d", (gAcc[group] >> 1) & 1);
		printf(" ");
		printf("%d", (gAcc[group] >> 0) & 1);
		printf(" ] ");
		if (invertedError) {
			printf(" Inverted access bits did not match! ");
		}
	}

	if (group != 3 &&
		(gAcc[group] == 1 ||
			gAcc[group] == 6)) { // Not a sector trailer, a value block
		int32_t value = (int32_t(buffer[3]) << 24) |
						(int32_t(buffer[2]) << 16) | (int32_t(buffer[1]) << 8) |
						int32_t(buffer[0]);
		printf(" Value=0x");
		printf("%X", value);
		printf(" Adr=0x");
		printf("%X", buffer[12]);
	}
	printf("\n");
}

/**
 * Executes the MFRC522 MFAuthent command.
 * This command manages MIFARE authentication to enable a secure communication
 * to any MIFARE Mini, MIFARE 1K and MIFARE 4K card. The authentication is
 * described in the MFRC522 datasheet section 10.3.1.9 and
 * http://www.nxp.com/documents/data_sheet/MF1S503x.pdf section 10.1. For use
 * with MIFARE Classic PICCs. The PICC must be selected - ie in state ACTIVE(*)
 * - before calling this function. Remember to call PCD_StopCrypto1() after
 * communicating with the authenticated PICC - otherwise no new communications
 * can start.
 *
 * All keys are set to FFFFFFFFFFFFh at chip delivery.
 *
 * @return STATUS_OK on success, STATUS_??? otherwise. Probably STATUS_TIMEOUT
 * if you supply the wrong key.
 */
int rc552::PCD_Authenticate(
	uint8_t command,   ///< PICC_CMD_MF_AUTH_KEY_A or PICC_CMD_MF_AUTH_KEY_B
	uint8_t blockAddr, ///< The block number. See numbering in the comments in
					   ///< the .h file.
	const uint8_t *key ///< Pointer to the Crypteo1 key to use (6 bytes)
) {
	uint8_t waitIRq = 0x10; // IdleIRq

	// Build command buffer
	uint8_t sendData[12];
	sendData[0] = command;
	sendData[1] = blockAddr;
	for (uint8_t i = 0; i < MF_KEY_SIZE; i++) { // 6 key bytes
		sendData[2 + i] = key[i];
	}
	// Use the last uid bytes as specified in
	// http://cache.nxp.com/documents/application_note/AN10927.pdf section 3.2.5
	// "MIFARE Classic Authentication". The only missed case is the MF1Sxxxx
	// shortcut activation, but it requires cascade tag (CT) byte, that is not
	// part of uid.
	for (uint8_t i = 0; i < 4; i++) { // The last 4 bytes of the UID
		sendData[8 + i] = uid.uidByte[i + uid.size - 4];
	}

	// Start the authentication.
	return PCD_CommunicateWithPICC(PCD_MFAuthent, waitIRq, &sendData[0],
		sizeof(sendData), nullptr, nullptr, nullptr, 0, false);
} // End PCD_Authenticate()
