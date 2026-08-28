#ifndef __MAIN_H
#define __MAIN_H

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdbool.h>
#include "gd32f30x.h"
#include <cmsis_os2.h>

#include "core/hardware_config.h"
#include "core/global.h"

#define Nop() asm("NOP")

#define VERSION 110
#define _firmware VERSION 
#define __addressStartProgram 0x08002000
#define __projectStart 0x22385678

#define PAGE_SIZE ((uint32_t)0x800)
#define PAGE_NUM ((uint32_t)0x60)
#define FLASH_BASE_END ((uint32_t)FLASH_BASE + ((uint32_t)PAGE_NUM * (uint32_t)PAGE_SIZE))

#define MEM_START_ADDR 0x08030000
#define MEM_NUMBER_PAGE ((uint32_t)20)

#define DATA_SIZE_SLOT ((uint32_t)0x100)
#define DATA_SLOT_MAX ((uint32_t)0x100)

#define GetUID_Word2 *((volatile uint32_t *)0x1FFFF7F0)
#define GetUID_Word1 *((volatile uint32_t *)0x1FFFF7EC)
#define GetUID_Word0 *((volatile uint32_t *)0x1FFFF7E8)

#define productID "C38525BB"
#define _VERSION_BUILD 20

#define _hardware 100
#define _deviceType 0x02

#define _model "MODEL010"
#define _s(x) #x
#define str(x) _s(x)

#define __StringVersion "ecuShopFirmware_demo:"str(_firmware)"\n ecu_model:ecuShopFirmware_demo\n"

#endif
