/**
 * @file   xmodem.c
 * @brief  Implementation of xmodem protocol to receive data from the serial port.
 *
 * This file contains the implementation of the xmodem protocol. The xmodem protocol is
 * a simple error-correcting protocol for transferring data over asynchronous serial
 * lines. This implementation is based on the xmodem-128 variant of the protocol which
 * uses 128 byte packets.
 *
 * @author Kashish Shah
 * @date   Jul 16, 2025
 */


/******************************************************************************
 *                               INCLUDES
 *****************************************************************************/
#include "xmodem.h"
#include "usart.h"
#include "flash_utils.h"
#include "string.h"
#include <stdbool.h>



/******************************************************************************
 *                               FUNCTIONS
 *****************************************************************************/

static uint16_t xModemCrc16Calculate(uint8_t *data, uint16_t len);
static xModemStatus xModemHandleError();
static xModemStatus xModemHandlePacket();


/******************************************************************************
 *                               VARIABLES
 *****************************************************************************/

sXModemPacket_t xPacket;


/******************************************************************************
 *                               FUNCTIONS
 *****************************************************************************/

/**
 * @brief Initializes the xModem transfer with the specified base address and maximum size.
 * 
 * This function sets up the xModem packet structure with initial values and
 * prepares it for data transfer.
 * 
 * @param base_addr The base address in flash memory where data will be written.
 * @param max_size The maximum size of data that can be written to flash memory.
 */
void xModemInit(uint32_t base_addr, uint32_t max_size){
    // Set the initial spam character for xModem protocol
    xPacket.spam_char = 'C';
    
    // Initialize error count to zero
    xPacket.errorCount = 0;
    
    // Initialize the packet header to zero
    xPacket.header = 0;
    
    // Initialize the data packet size to zero
    xPacket.size = 0;
    
    // Set the status of xModem packet to OK
    xPacket.status = XMODEM_OK;
    
    // Initialize the xModem packet number to 1
    xPacket.xmodem_packet_number = 1;
    
    // Set the flash address where data will be written
    xPacket.xmodem_flash_address = base_addr;
    
    // Set the maximum size of the xModem transfer
    xPacket.xmodem_flash_size = max_size;
    
    // Mark the first packet as not yet received
    xPacket.x_first_packet = false;
}

/**
 * @brief Receive the header of the xModem packet from the serial port.
 *
 * This function waits for 100ms to receive the header of the xModem packet.
 * If the header is not received within the specified time, it will return false.
 * If the header is received, it will store it in the xPacket structure and return true.
 *
 * @return true if the header is received, false otherwise.
 */
bool xModemGetHeader(){
	if (HAL_UART_Receive(&huart2, &(xPacket.header), 1, 100) != HAL_OK)
	{
		// If the first packet has not been received yet, send a spam character
		if (!xPacket.x_first_packet) {
			HAL_UART_Transmit(&huart2, &(xPacket.spam_char), 1, HAL_MAX_DELAY);
		} else {
			// Otherwise, handle the error
			xPacket.status = xModemHandleError();
		}
		return false;
	}
	return true;
}

/**
 * @brief Receive the xModem packet from the serial port and write it to the flash.
 *
 * This function will receive the xModem packet from the serial port and write it to the
 * flash memory. It will continue to receive packets until an error occurs or the end
 * of the transfer is reached.
 */
void xModemReceive()
{
    while (xPacket.status == XMODEM_OK)
    {

    	if (!xModemGetHeader()){
    		continue;
    	}

        switch (xPacket.header)
        {
            case XMODEM_SOH:
            case XMODEM_STX:
            	// Set the status of xModem packet to OK
            	xPacket.status = XMODEM_OK;

            	// Set the size of the xModem packet
            	if (xPacket.header == XMODEM_SOH) {
            		xPacket.size = XMODEM_PACKET_128_SIZE;
            	} else if (xPacket.header == XMODEM_STX) {
            		xPacket.size = XMODEM_PACKET_1024_SIZE;
            	}

                if (xPacket.size == 0) {
                	// Set the status of xModem packet to error if the size is 0
                	xPacket.status = XMODEM_ERROR;
                }


                if (HAL_UART_Receive(&huart2, xPacket.pkt_num, 2, HAL_MAX_DELAY) != HAL_OK ||
                    HAL_UART_Receive(&huart2, xPacket.data, xPacket.size, HAL_MAX_DELAY) != HAL_OK ||
                    HAL_UART_Receive(&huart2, xPacket.crc, 2, HAL_MAX_DELAY) != HAL_OK){
                	// Set the status of xModem packet to error if the packet is not received correctly
                	xPacket.status = XMODEM_ERROR_UART;
                }
                uint16_t crc_recv = ((uint16_t)xPacket.crc[0] << 8) | xPacket.crc[1];
                uint16_t crc_calc = xModemCrc16Calculate(xPacket.data, xPacket.size);

                if (xPacket.pkt_num[0] != xPacket.xmodem_packet_number ||
					((xPacket.pkt_num[0] + xPacket.pkt_num[1]) != 0xFF))
                	// Set the status of xModem packet to error if the packet number is not correct
                	xPacket.status = XMODEM_ERROR_NUMBER;
                if (crc_recv != crc_calc)
                	// Set the status of xModem packet to error if the CRC is not correct
                	xPacket.status = XMODEM_ERROR_CRC;

                if (!xPacket.x_first_packet)
                {
                    if (!flash_erase_region(xPacket.xmodem_flash_address, xPacket.xmodem_flash_size))
                    	// Set the status of xModem packet to error if the flash erase failed
                    	xPacket.status = XMODEM_ERROR_FLASH;
                    xPacket.x_first_packet = true;
                }

                if (!flash_write(xPacket.xmodem_flash_address, xPacket.data, xPacket.size))
                	// Set the status of xModem packet to error if the flash write failed
                	xPacket.status = XMODEM_ERROR_FLASH;

                xPacket.xmodem_packet_number++;
                xPacket.xmodem_flash_address += xPacket.size;

                if (xPacket.status == XMODEM_OK)
                {
                    uint8_t ack = XMODEM_ACK;
                    HAL_UART_Transmit(&huart2, &ack, 1, HAL_MAX_DELAY);
                }
                else if (xPacket.status == XMODEM_ERROR_FLASH)
				{
					// Set the error count to the maximum if the flash error occurs
					xPacket.errorCount = XMODEM_MAX_ERRORS;
					xPacket.status = xModemHandleError();
				}
                else
                {
					// Handle the error if the error count is greater than 0
					xPacket.status = xModemHandleError();
				}
                break;

            case XMODEM_EOT:
                {
                    uint8_t ack = XMODEM_ACK;
                    HAL_UART_Transmit(&huart2, &ack, 1, HAL_MAX_DELAY);
                    return;  // Finished
                }

            case XMODEM_CAN:
                xPacket.status = XMODEM_ERROR;
                return;  // Exit on cancel

            default:
                if (xPacket.header != 0xFF)  // Ignore noise
                	// Set the status of xModem packet to error if the header is not valid
                	xPacket.status = XMODEM_ERROR;
                return;
        }
        if (xPacket.status != XMODEM_OK) {
            return;  // Exit on failure
        }
    }
}

/**
 * @brief Calculate the 16-bit CRC of a given data block.
 *
 * Calculates the 16-bit CRC of the given data block using the CRC-16/CCITT
 * algorithm.
 *
 * @param[in] data Pointer to the data block.
 * @param[in] len Length of the data block.
 * @return The 16-bit CRC.
 */
static uint16_t xModemCrc16Calculate(uint8_t *data, uint16_t len)
{
    uint16_t crc = 0;
    while (len--)
    {
        // XOR the current byte with the current CRC
        crc ^= ((uint16_t)*data++) << 8;
        // Process each bit of the current byte
        for (uint8_t i = 0; i < 8; ++i)
            // If the current bit is set, XOR it with 0x1021
            crc = (crc & 0x8000) ? (crc << 1) ^ 0x1021 : (crc << 1);
    }
    return crc;
}

/**
 * @brief Handle errors in the xModem transfer.
 *
 * This function handles errors that occur during the xModem transfer. It
 * increments the error count and checks if the maximum number of errors has
 * been reached. If the maximum number of errors has been reached, it sends a
 * cancel packet to the sender and returns an error. Otherwise, it sends a
 * NAK packet to the sender and returns an OK status.
 *
 * @return The status of the error handling. If the maximum number of errors
 * has been reached, the status is XMODEM_ERROR. Otherwise, the status is
 * XMODEM_OK.
 */
static xModemStatus xModemHandleError()
{
    xPacket.errorCount++;
    if ( xPacket.errorCount >= XMODEM_MAX_ERRORS)
    {
        // Send a cancel packet to the sender if the maximum number of errors
        // has been reached
        uint8_t can = XMODEM_CAN;
        HAL_UART_Transmit(&huart2, &can, 1, HAL_MAX_DELAY);
        HAL_UART_Transmit(&huart2, &can, 1, HAL_MAX_DELAY);
        return XMODEM_ERROR;
    }
    else
    {
        // Send a NAK packet to the sender if the maximum number of errors has
        // not been reached
        uint8_t nak = XMODEM_NAK;
        HAL_UART_Transmit(&huart2, &nak, 1, HAL_MAX_DELAY);
        return XMODEM_OK;
    }
}
