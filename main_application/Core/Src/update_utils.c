/*
 * update_utils.c
 *
 *  Created on: Jul 20, 2025
 *      Author: kshah
 */


#include "stm32l4xx_hal.h"
#include "update_utils.h"

#define RTC_UPDATE_FLAG_VALUE   0xDEADBEEF
#define RTC_UPDATE_BKP_REG      RTC_BKP_DR0

void set_update_flag(void)
{
    // Enable access to the backup domain
    __HAL_RCC_PWR_CLK_ENABLE();
    HAL_PWR_EnableBkUpAccess();

    // Enable RTC peripheral if not already enabled
    __HAL_RCC_RTC_ENABLE();

    // Store update flag into Backup Register 0
    HAL_RTCEx_BKUPWrite(&hrtc, RTC_UPDATE_BKP_REG, RTC_UPDATE_FLAG_VALUE);
}

