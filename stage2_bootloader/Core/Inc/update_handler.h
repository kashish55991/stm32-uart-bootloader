/**
 * @file    update_handler.h
 * @brief   Header file for update_handler.c
 *
 *         This module contains the functions needed to handle the
 *         bootloader logic after the device is powered on or reset.
 *
 * @author  Kashish Shah
 * @date    July 20, 2025
 */

#ifndef _UPDATE_HANDLER_H_
#define _UPDATE_HANDLER_H_

/******************************************************************************
*                               FUNCTIONS
*****************************************************************************/

/**
 * @brief Checks if the main application is present and runs the bootloader
 *        logic: erases the sector, programs the sector and jumps to the
 *        application.
 */
void execute_bootloader_sequence(void);

/**
 * @brief Jumps to the main application by setting the main stack pointer (MSP)
 *        and the vector table offset register (VTOR) to the values stored in
 *        the beginning of the main application's memory region.
 */
void jump_to_main_app(void);

#endif /* _UPDATE_HANDLER_H_ */
