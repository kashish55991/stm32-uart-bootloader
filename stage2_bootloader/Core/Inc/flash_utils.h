/**
 * @file    flash_utils.h
 * @author  Kashish Shah
 * @brief   Utility functions for flash memory operations
 * @date    July 20, 2025
 *
 * This file contains utility functions for flash memory operations such as
 * erasing and programming flash pages. It also contains the definition of
 * the flash page size and the struct for flash page information.
 */



#ifndef _CORE_FLASH_UTILS_H_
#define _CORE_FLASH_UTILS_H_


/******************************************************************************
 *                               INCLUDES
 *****************************************************************************/
#include "stm32l4xx_hal.h"
#include <stdbool.h>

/******************************************************************************
 *                               FUNCTIONS
 *****************************************************************************/

/**
 * @brief  Erase a given region in flash memory.
 * @param  address  Starting address of the region to be erased.
 * @param  size     Size of the region to be erased.
 * @return true if the erase operation was successful, false if not.
 */
bool flash_erase_region(uint32_t address, uint32_t size);

/**
 * @brief  Write data to a given region in flash memory.
 * @param  address  Starting address of the region to be written to.
 * @param  data     Data to be written.
 * @param  length   Length of the data to be written.
 * @return true if the write operation was successful, false if not.
 */
bool flash_write(uint32_t address, const uint8_t *data, uint32_t length);

#endif /* _CORE_FLASH_UTILS_H_ */
