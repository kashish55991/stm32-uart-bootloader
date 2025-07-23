/*
 * debug.h
 *
 *  Created on: Jul 20, 2025
 *      Author: kshah
 */


#ifndef __DEBUG_H
#define __DEBUG_H

#include "usart.h"   // For huart2
#include <stdio.h>   // For vsnprintf
#include <string.h>

#define DEBUG_BUFFER_SIZE 128

// Set to 1 to enable debug printing
#define DEBUG_ENABLED 1

#if DEBUG_ENABLED
#define DBG_PRINTF(fmt, ...)                    \
    do {                                        \
        char dbg_buf[DEBUG_BUFFER_SIZE];        \
        snprintf(dbg_buf, sizeof(dbg_buf), fmt, ##__VA_ARGS__); \
        HAL_UART_Transmit(&huart2, (uint8_t*)dbg_buf, strlen(dbg_buf), HAL_MAX_DELAY); \
    } while (0)
#else
#define DBG_PRINTF(fmt, ...)  ((void)0)
#endif

#endif /* __DEBUG_H */
