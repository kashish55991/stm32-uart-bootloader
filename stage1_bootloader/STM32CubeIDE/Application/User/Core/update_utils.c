/*
 * update_utils.c
 *
 *  Created on: Jul 15, 2025
 *      Author: kshah
 */


#include "update_utils.h"

// --- Addresses and sizes ---
#define FLASH_KEY1              0x45670123U
#define FLASH_KEY2              0xCDEF89ABU

#define FLASH_BASE_ADDRESS      0x08000000U
#define FLASH_PAGE_SIZE         0x800U       // 2 KB

#define STAGE2_ADDRESS          0x08002000U
#define STAGE2_SIZE             (16 * 1024U)

#define STAGE2_TEMP_ADDRESS     0x0803A000U  // Temporary internal Flash storage

#define RTC_UPDATE_FLAG_REG     LL_RTC_BKP_DR0
#define RTC_UPDATE_FLAG_VALUE   0xDEADBEEF

#define FLASH_CR_START          (1U << 16)   // Not defined in STM32L4 headers

// --- RTC Backup Flag API ---

void set_update_flag(void)
{
    LL_PWR_EnableBkUpAccess();
    LL_RTC_BAK_SetRegister(RTC, RTC_UPDATE_FLAG_REG, RTC_UPDATE_FLAG_VALUE);
}

bool check_update_flag(void)
{
    LL_PWR_EnableBkUpAccess();
    return (LL_RTC_BAK_GetRegister(RTC, RTC_UPDATE_FLAG_REG) == RTC_UPDATE_FLAG_VALUE);
}

void clear_update_flag(void)
{
    LL_PWR_EnableBkUpAccess();
    LL_RTC_BAK_SetRegister(RTC, RTC_UPDATE_FLAG_REG, 0);
}

// --- Flash Low-Level Utilities ---

static void flash_unlock(void)
{
    if ((FLASH->CR & FLASH_CR_LOCK) != 0)
    {
        FLASH->KEYR = FLASH_KEY1;
        FLASH->KEYR = FLASH_KEY2;
    }
}

static void flash_lock(void)
{
    FLASH->CR |= FLASH_CR_LOCK;
}

// Erase stage2 bootloader region (16 KB)
void erase_stage2_flash(void)
{
    flash_unlock();

    for (uint32_t addr = STAGE2_ADDRESS; addr < (STAGE2_ADDRESS + STAGE2_SIZE); addr += FLASH_PAGE_SIZE)
    {
        while (FLASH->SR & FLASH_SR_BSY);

        // Clear error flags
        FLASH->SR |= FLASH_SR_WRPERR | FLASH_SR_PGAERR | FLASH_SR_SIZERR |
                     FLASH_SR_PGSERR | FLASH_SR_MISERR | FLASH_SR_FASTERR |
                     FLASH_SR_RDERR | FLASH_SR_OPTVERR;

        FLASH->CR &= ~(FLASH_CR_PNB | FLASH_CR_PER);
        FLASH->CR |= FLASH_CR_PER;

        uint32_t page_number = (addr - FLASH_BASE_ADDRESS) / FLASH_PAGE_SIZE;
        FLASH->CR |= (page_number << FLASH_CR_PNB_Pos);

        FLASH->CR |= FLASH_CR_START;
        while (FLASH->SR & FLASH_SR_BSY);

        FLASH->CR &= ~FLASH_CR_PER;
    }

    flash_lock();
}

// Write a 64-bit double word to Flash
static void flash_write64(uint32_t addr, uint64_t data)
{
    flash_unlock();
    FLASH->CR |= FLASH_CR_PG;

    *(volatile uint32_t*)addr       = (uint32_t)(data & 0xFFFFFFFFU);
    *(volatile uint32_t*)(addr + 4) = (uint32_t)(data >> 32U);

    while (FLASH->SR & FLASH_SR_BSY);
    FLASH->CR &= ~FLASH_CR_PG;
    flash_lock();
}

// Copy new stage2 image from internal temp Flash region to stage2 region
void copy_stage2_from_temp_flash(void)
{
    for (uint32_t i = 0; i < STAGE2_SIZE; i += 8)
    {
        uint64_t data = *((uint64_t*)(STAGE2_TEMP_ADDRESS + i));
        flash_write64(STAGE2_ADDRESS + i, data);
    }
}
