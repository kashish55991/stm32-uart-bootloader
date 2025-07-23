/**
 * @file       flash_utils.c
 * @brief      Utilities for accessing and erasing the non-volatile memory.
 * @details    This module provides functions for erasing the application region
 *             of the flash memory. The erase region function is used by the
 *             XMODEM receive function to erase the target region before
 *             reprogramming the flash.
 *
 * @author     kshah
 * @date       Jul 20, 2025
 */

/******************************************************************************
 *                               INCLUDES
 *****************************************************************************/
#include "flash_utils.h"
#include "stm32l4xx_hal.h"
#include <string.h>


/******************************************************************************
 *                               FUNCTIONS
 *****************************************************************************/

/**
 * @brief  Erases a specified region of the flash memory.
 * @param  address: Start address of the flash region to be erased.
 * @param  size: Size of the flash region to be erased in bytes.
 * @retval true if the erase operation was successful, false otherwise.
 */
bool flash_erase_region(uint32_t address, uint32_t size)
{
    // Unlock the flash memory for write access
    HAL_FLASH_Unlock();

    // Calculate the first page to erase
    uint32_t first_page = (address - 0x08000000U) / FLASH_PAGE_SIZE;
    // Calculate the number of pages to erase, rounding up if needed
    uint32_t num_pages  = (size + FLASH_PAGE_SIZE - 1) / FLASH_PAGE_SIZE;

    // Initialize the erase structure
    FLASH_EraseInitTypeDef erase_init = {0};
    uint32_t page_error = 0;

    // Set the erase parameters
    erase_init.TypeErase = FLASH_TYPEERASE_PAGES;
    erase_init.Page      = first_page;
    erase_init.NbPages   = num_pages;
    erase_init.Banks     = FLASH_BANK_1;

    // Perform the erase operation
    if (HAL_FLASHEx_Erase(&erase_init, &page_error) != HAL_OK)
    {
        // Optionally: handle error
        HAL_FLASH_Lock();
        return false;
    }

    // Lock the flash memory to prevent accidental writes
    HAL_FLASH_Lock();

    return true;
}

/**
 * @brief  Writes data to a specified region of the flash memory.
 * @param  address: Start address of the flash region to be written to.
 * @param  data: Data to be written to the flash memory.
 * @param  length: Length of the data to be written in bytes.
 * @retval true if the write operation was successful, false otherwise.
 */
bool flash_write(uint32_t address, const uint8_t *data, uint32_t length)
{
    // Unlock the flash memory for write access
    HAL_FLASH_Unlock();

    // Iterate over the data to be written in steps of 8 bytes (for double-word write)
    for (uint32_t i = 0; i < length; i += 8)
    {
        // Create a 64-bit word from the data to be written
        uint64_t word = 0xFFFFFFFFFFFFFFFF;
        memcpy(&word, data + i, (length - i) >= 8 ? 8 : (length - i));

        // Write the word to the flash memory, checking for errors
        if (HAL_FLASH_Program(FLASH_TYPEPROGRAM_DOUBLEWORD, address + i, word) != HAL_OK)
        {
            // If an error occurs, lock the flash memory and return false
            HAL_FLASH_Lock();
            return false;
        }
    }

    // Lock the flash memory to prevent accidental writes
    HAL_FLASH_Lock();

    // Return true if the write operation was successful
    return true;
}
