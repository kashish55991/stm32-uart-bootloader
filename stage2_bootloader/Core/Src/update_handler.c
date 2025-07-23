/**
 * @file update_handler.c
 * @brief This file contains the implementation of update handling logic for the bootloader.
 * 
 * This module is responsible for managing the update process, including communication
 * with the main application and handling update flags.
 * 
 * @author Kashish Shah
 * @date October 2023
 */



/******************************************************************************
*                               INCLUDES
*****************************************************************************/
#include "update_handler.h"
#include "xmodem.h"
#include "usart.h"
#include "rtc.h"
#include "main.h"

/******************************************************************************
*                               DEFINES
*****************************************************************************/
#define RTC_UPDATE_FLAG   0xDEADBEEF
#define RTC_UPDATE_REG    RTC_BKP_DR0

#define APP_BASE_ADDR  0x08008000U
#define APP_MAX_SIZE   (0x0803FFFFU - APP_BASE_ADDR + 1)  // Adjust based on your flash layout

/******************************************************************************
*                               FUNCTIONS
*****************************************************************************/

/**
 * @brief Main entry point for the bootloader logic.
 * 
 * This function is the main entry point for the bootloader logic. It is responsible
 * for checking the update flag, receiving the update data using XMODEM, and jumping
 * to the main application.
 */
void execute_bootloader_sequence(void)
{
    // Print a success message to the serial terminal
    HAL_UART_Transmit(&huart2, (uint8_t *)"\r\n[BOOT] Stage 2 Ready...\r\n", 27, HAL_MAX_DELAY);

    // Check if the update flag is set
    if (HAL_RTCEx_BKUPRead(&hrtc, RTC_UPDATE_REG) == RTC_UPDATE_FLAG)
    {
        // Print a message to the serial terminal indicating that an update is available
        HAL_UART_Transmit(&huart2, (uint8_t *)"[BOOT] Update flag detected.\r\n", 30, HAL_MAX_DELAY);

        // Clear the update flag
        HAL_RTCEx_BKUPWrite(&hrtc, RTC_UPDATE_REG, 0);

        // Print a message to the serial terminal indicating that the bootloader is waiting for XMODEM data
        HAL_UART_Transmit(&huart2, (uint8_t *)"[BOOT] Waiting for XMODEM...\r\n", 30, HAL_MAX_DELAY);

        // Initialize the XMODEM receiver
        xModemInit(APP_BASE_ADDR, APP_MAX_SIZE);

        // Receive the XMODEM data
        xModemReceive();  // Full blocking receive routine
    }
    else
    {
        // Print a message to the serial terminal indicating that there is no update available
        HAL_UART_Transmit(&huart2, (uint8_t *)"[BOOT] No flag. Jumping to app...\r\n", 35, HAL_MAX_DELAY);
    }

    // Wait for 100ms before jumping to the main application
    HAL_Delay(100);

    // Jump to the main application
    jump_to_main_app();
}

/**
 * @brief Jump to the main application.
 * 
 * This function jumps to the main application by setting the main stack pointer
 * (MSP) and the vector table offset register (VTOR) to the values stored in the
 * beginning of the main application's memory region.
 */
void jump_to_main_app(void)
{
    // Check if the stack pointer is within the valid range
    uint32_t app_stack = *(volatile uint32_t*)APP_BASE_ADDR;
    if (app_stack < 0x20000000 || app_stack > 0x20040000)
    {
        // Print an error message if the stack pointer is invalid
        HAL_UART_Transmit(&huart2, (uint8_t *)"Invalid stack pointer\r\n", 24, HAL_MAX_DELAY);
        // Enter an infinite loop (debugging purpose)
        while (1);
    }

    // Set the vector table offset register (VTOR)
    SCB->VTOR = APP_BASE_ADDR;

    // Set the main stack pointer (MSP)
    __set_MSP(app_stack);

    // Jump to the reset handler
    ((void (*)(void))(*(volatile uint32_t*)(APP_BASE_ADDR + 4)))();
}
