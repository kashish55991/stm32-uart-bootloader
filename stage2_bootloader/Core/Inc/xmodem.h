/**
 * @file   xmodem.h
 * @brief  Header file for xmodem implementation.
 *
 * This file contains the definitions, macros and function prototypes for the xmodem implementation.
 *
 * @author Kashish Shah
 * @date   Jul 20, 2025
 */

#ifndef _XMODEM_H_
#define _XMODEM_H_

/******************************************************************************
 *                               INCLUDES
 *****************************************************************************/

#include <stdint.h>

/******************************************************************************
 *                               DEFINES
 *****************************************************************************/

#define XMODEM_PACKET_NUMBER_SIZE     2   /*!< Size of packet number in bytes. */
#define XMODEM_PACKET_CRC_SIZE        2   /*!< Size of packet CRC in bytes. */
#define XMODEM_PACKET_128_SIZE        128 /*!< Size of packet in bytes for 128 byte packets. */
#define XMODEM_PACKET_1024_SIZE       1024 /*!< Size of packet in bytes for 1024 byte packets. */
#define XMODEM_PACKET_CRC_HIGH_INDEX  0   /*!< Index of high byte of packet CRC. */
#define XMODEM_PACKET_CRC_LOW_INDEX   1   /*!< Index of low byte of packet CRC. */
#define XMODEM_MAX_ERRORS             10  /*!< Maximum number of errors allowed before the transfer is terminated. */


/******************************************************************************
 *                               ENUMS
 *****************************************************************************/

 typedef enum {
    XMODEM_OK,             /*!< Successful operation */
    XMODEM_ERROR,          /*!< General error */
    XMODEM_ERROR_FLASH,    /*!< Flash memory error */
    XMODEM_ERROR_CRC,      /*!< CRC check failed */
    XMODEM_ERROR_NUMBER,   /*!< Incorrect packet number */
    XMODEM_ERROR_UART      /*!< UART communication error */
} xModemStatus;

typedef enum {
	XMODEM_SOH = 0x01, /*!< Start of 128 byte packet */
	XMODEM_STX = 0x02, /*!< Start of 1024 byte packet */
	XMODEM_EOT = 0x04, /*!< End of transmission */
	XMODEM_ACK = 0x06, /*!< Acknowledge */
	XMODEM_NAK = 0x15, /*!< Negative acknowledge */
	XMODEM_CAN = 0x18, /*!< Cancel */
	XMODEM_CRC = 0x43  /*!< CRC check */
} xModemHeader;


/******************************************************************************
 *                               STRUCTURES
 *****************************************************************************/


/**
 * @brief Structure containing the data and status of an xModem packet.
 *
 * @details This structure contains the status of the xModem packet, whether it is the first packet,
 * the packet number, the flash address and size, the spam character, the error count, the header,
 * the size of the packet, the packet number, the data, and the CRC.
 */
typedef struct sXModemPacket{
	xModemStatus status; /*!< Status of the xModem packet. */
	bool x_first_packet; /*!< Flag indicating if it is the first packet. */
	uint8_t xmodem_packet_number; /*!< Packet number. */
	uint32_t xmodem_flash_address; /*!< Address in flash memory where data will be written. */
	uint32_t xmodem_flash_size; /*!< Maximum size of data that can be written to flash memory. */
	uint8_t spam_char; /*!< Character used to spam the serial port if an error occurs. */
	uint8_t errorCount; /*!< Number of errors that have occurred. */
	xModemHeader header; /*!< Header of the xModem packet. */
	uint16_t size; /*!< Size of the packet in bytes. */
	uint8_t pkt_num[XMODEM_PACKET_NUMBER_SIZE]; /*!< Packet number in bytes. */
	uint8_t data[XMODEM_PACKET_1024_SIZE]; /*!< Data in bytes. */
	uint8_t crc[XMODEM_PACKET_CRC_SIZE]; /*!< CRC of the packet in bytes. */
} sXModemPacket_t;


/******************************************************************************
 *                               FUNCTIONS
 *****************************************************************************/

/**
 * @brief Initializes the xModem transfer with the specified base address and maximum size.
 *
 * Sets up the xModem packet structure with initial values and prepares it for data transfer.
 *
 * @param base_addr The base address in flash memory where data will be written.
 * @param max_size The maximum size of data that can be written to flash memory.
 */
void xModemInit(uint32_t base_addr, uint32_t max_size);

/**
 * @brief Receives data using the xModem protocol.
 *
 * This function handles the reception of data packets following the xModem protocol.
 * It processes incoming headers and manages the status of the transfer, handling errors and
 * acknowledgments as necessary.
 */
void xModemReceive();

#endif //_XMODEM_H_

