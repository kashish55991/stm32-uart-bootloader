/*
 * update_utils.h
 *
 *  Created on: Jul 15, 2025
 *      Author: kshah
 */

#ifndef __UPDATE_UTILS_H
#define __UPDATE_UTILS_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32l4xx.h"
#include "stm32l4xx_ll_rtc.h"
#include "stm32l4xx_ll_pwr.h"

// Update flag handling
void set_update_flag(void);
bool check_update_flag(void);
void clear_update_flag(void);

// Flash update operations
void erase_stage2_flash(void);
void copy_stage2_from_temp_flash(void);

#endif
