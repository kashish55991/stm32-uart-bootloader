/*
 * jump_utils.c
 *
 *  Created on: Jul 15, 2025
 *      Author: kshah
 */

#include "jump_utils.h"

#define STAGE2_ADDRESS  0x08002000

void jump_to_stage2(void)
{
    typedef void (*pFunction)(void);
    uint32_t jump_address = *(__IO uint32_t*)(STAGE2_ADDRESS + 4);
    pFunction jump_to_app = (pFunction)jump_address;

    // Set main stack pointer
    __set_MSP(*(__IO uint32_t*)STAGE2_ADDRESS);

    // Jump
    jump_to_app();
}

