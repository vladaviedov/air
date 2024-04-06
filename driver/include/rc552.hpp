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

#include "spi.hpp"

#define MIFARE_Key 6
// A Mifare Crypto1 key is 6 bytes.
#define MF_KEY_SIZE 6

class rc552 {
public:
	// A struct used for passing the UID of a PICC.
	typedef struct {
		uint8_t size; // Number of bytes in the UID. 4, 7 or 10.
		uint8_t uidByte[10];
		uint8_t sak; // The SAK (Select acknowledge) byte returned from the PICC
					 // after successful selection.
	} Uid;

	// MIFARE Crypto1 key
	uint8_t keyByte[MF_KEY_SIZE];

	// Member variables
	Uid uid; // Used by PICC_ReadCardSerial().

	/**
	 * @brief Initializer
	 *
	 * @param[in] chip - libgpiod GPIO chip object.
	 * @param[in] inter_pin - Interrupt libgpiod pin number.
	 * @param[in] dev_addr - I2C device address.
	 * @param[in] adapter - I2C ioctl adapter number.
	 */
	rc552(const gpiod::chip &chip, uint32_t inter_pin, std::string adapter);

	~rc552();

	/**
	 * @brief Call callback on interrupt.
	 *
	 * @param[in] callback - Function to call on interrupt
	 */
	void on_interrupt(std::function<void()> callback);

	/**
	 * Reads 16 bytes (+ 2 bytes CRC_A) from the active PICC.
	 *
	 * For MIFARE Classic the sector containing the block must be authenticated
	 * before calling this function.
	 *
	 * For MIFARE Ultralight only addresses 00h to 0Fh are decoded.
	 * The MF0ICU1 returns a NAK for higher addresses.
	 * The MF0ICU1 responds to the READ command by sending 16 bytes starting
	 * from the page address defined by the command argument. For example; if
	 * blockAddr is 03h then pages 03h, 04h, 05h, 06h are returned. A roll-back
	 * is implemented: If blockAddr is 0Eh, then the contents of pages 0Eh, 0Fh,
	 * 00h and 01h are returned.
	 *
	 * The buffer must be at least 18 bytes because a CRC_A is also returned.
	 * Checks the CRC_A before returning STATUS_OK.
	 *
	 * @return STATUS_OK on success, STATUS_??? otherwise.
	 */
	int MIFARE_Read(uint8_t blockAddr, uint8_t *data, uint8_t *bufferSize);

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

	int PCD_CommunicateWithPICC(
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

	bool PICC_IsNewCardPresent();

	void PCD_ClearRegisterBitMask(
		uint8_t reg, ///< The register to update. One of the PCD_Register enums.
		uint8_t mask ///< The bits to clear.
	);

	int PICC_REQA_or_WUPA(
		uint8_t
			command, ///< The command to send - PICC_CMD_REQA or PICC_CMD_WUPA
		uint8_t *
			bufferATQA, ///< The buffer to store the ATQA (Answer to request) in
		uint8_t *bufferSize ///< Buffer size, at least two bytes. Also number of
							///< bytes returned if STATUS_OK.
	);

	int PICC_RequestA(
		uint8_t *
			bufferATQA, ///< The buffer to store the ATQA (Answer to request) in
		uint8_t *bufferSize ///< Buffer size, at least two bytes. Also number of
							///< bytes returned if STATUS_OK.
	);

	void PCD_WriteRegister(
		uint8_t
			reg, ///< The register to write to. One of the PCD_Register enums.
		uint8_t value ///< The value to write.
	);

	/**
	 * Simple wrapper around PICC_Select.
	 * Returns true if a UID could be read.
	 * Remember to call PICC_IsNewCardPresent(), PICC_RequestA() or
	 * PICC_WakeupA() first. The read UID is available in the class variable
	 * uid.
	 *
	 * @return bool
	 */
	bool PICC_ReadCardSerial();

	/**
	 * Transmits SELECT/ANTICOLLISION commands to select a single PICC.
	 * Before calling this function the PICCs must be placed in the READY(*)
	 * state by calling PICC_RequestA() or PICC_WakeupA(). On success:
	 * 		- The chosen PICC is in state ACTIVE(*) and all other PICCs have
	 * returned to state IDLE/HALT. (Figure 7 of the ISO/IEC 14443-3 draft.)
	 * 		- The UID size and value of the chosen PICC is returned in *uid
	 * along with the SAK.
	 *
	 * A PICC UID consists of 4, 7 or 10 bytes.
	 * Only 4 bytes can be specified in a SELECT command, so for the longer UIDs
	 * two or three iterations are used: UID size	Number of UID bytes
	 * Cascade levels		Example of PICC
	 * 		========	===================		==============
	 * =============== single				 4						1
	 * MIFARE Classic double				 7						2
	 * MIFARE Ultralight triple				10						3
	 * Not currently in use?
	 *
	 * @return STATUS_OK on success, STATUS_??? otherwise.
	 */
	int PICC_Select(uint8_t validBits = 0);

	/**
	 * Instructs a PICC in state ACTIVE(*) to go to state HALT.
	 *
	 * @return STATUS_OK on success, STATUS_??? otherwise.
	 */
	int PICC_HaltA();

	/**
	 * Used to exit the PCD from its authenticated state.
	 * Remember to call this function after communicating with an authenticated
	 * PICC - otherwise no new communications can start.
	 */
	void PCD_StopCrypto1();

	/**
	 * Dumps memory contents of a MIFARE Classic PICC.
	 * On success the PICC is halted after dumping the data.
	 */
	void PICC_DumpMifareClassicToSerial();
	/**
	 * Dumps memory contents of a sector of a MIFARE Classic PICC.
	 * Uses PCD_Authenticate(), MIFARE_Read() and PCD_StopCrypto1.
	 * Always uses PICC_CMD_MF_AUTH_KEY_A because only Key A can always read the
	 * sector trailer access bits.
	 */
	void PICC_DumpMifareClassicSectorToSerial(
		uint8_t *key,  ///< Key A for the sector.
		uint8_t sector ///< The sector to dump, 0..39.
	);

	/**
	 * Executes the MFRC522 MFAuthent command.
	 * This command manages MIFARE authentication to enable a secure
	 * communication to any MIFARE Mini, MIFARE 1K and MIFARE 4K card. The
	 * authentication is described in the MFRC522 datasheet section 10.3.1.9 and
	 * http://www.nxp.com/documents/data_sheet/MF1S503x.pdf section 10.1. For
	 * use with MIFARE Classic PICCs. The PICC must be selected - ie in state
	 * ACTIVE(*) - before calling this function. Remember to call
	 * PCD_StopCrypto1() after communicating with the authenticated PICC -
	 * otherwise no new communications can start.
	 *
	 * All keys are set to FFFFFFFFFFFFh at chip delivery.
	 *
	 * @return STATUS_OK on success, STATUS_??? otherwise. Probably
	 * STATUS_TIMEOUT if you supply the wrong key.
	 */
	int PCD_Authenticate(
		uint8_t command,   ///< PICC_CMD_MF_AUTH_KEY_A or PICC_CMD_MF_AUTH_KEY_B
		uint8_t blockAddr, ///< The block number. See numbering in the comments
						   ///< in the .h file.
		uint8_t *key       ///< Pointer to the Crypteo1 key to use (6 bytes)
	);

private:
	gpiod::line interrupt;
	std::unique_ptr<std::thread> interrupt_thread;
	std::atomic_bool active = true;
	spi spid;

	/**
	 * @brief Validate CRC A
	 *
	 * @param backData - Pointer to data from rc552
	 * @param backLen - Length of data
	 *
	 * @return error status
	 */
	int validateCRCA(
		uint8_t *backData, const uint8_t *backLen, const uint8_t *_validBits);
};
