#include "main.h"

FlagStatus FLASH_Erase_Range_0x08030000_to_0x08040000(void)
{
    fmc_state_enum status = FMC_READY;

    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

        for (uint32_t address = MEM_START_ADDR;
            address < MEM_START_ADDR + (DATA_SIZE_SLOT * DATA_SLOT_MAX);
            address += PAGE_SIZE)
    {
        status = fmc_page_erase(address);
        if (status != FMC_READY)
        {
            fmc_lock();
            return RESET;
        }
    }

    fmc_lock();
    return (status == FMC_READY) ? SET : RESET;
}

FlagStatus FLASH_Erase_Page(uint32_t address)
{
    fmc_state_enum flash_status = FMC_READY;

    // Unlock the flash memory
    fmc_unlock();

    // Clear all flags before starting
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    // Erase the current page
    flash_status = fmc_page_erase(address);

    // Check if erase operation was successful
    if (flash_status != FMC_READY)
    {
        // Clear flags and lock flash before returning error
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
        fmc_lock();
        return RESET; // Erase failed
    }

    // Clear the end flag after each page erase
    fmc_flag_clear(FMC_FLAG_BANK0_END);

    // Clear all flags after completion
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);

    // Lock the flash memory
    fmc_lock();

    return SET; // Erase successful
}

uint32_t find_last_index(void)
{
    uint32_t address = 0;
    uint32_t maxAddress = 0;
    uint32_t last_value = 0;
    uint32_t indexCheck = 0;
    
    for (address = MEM_START_ADDR; 
         address < (MEM_START_ADDR + DATA_SIZE_SLOT * DATA_SLOT_MAX); 
         address += DATA_SIZE_SLOT)
    {
        indexCheck = *(uint32_t *)address;

        if ((indexCheck != 0xFFFFFFFF && indexCheck >= last_value))
        {
            last_value = indexCheck;
            maxAddress = address;
        }
    }
    return maxAddress;
}

void mem_read(mem_save_t *dest)
{
    uint32_t last_index = find_last_index();
    if (*(const uint32_t *)last_index == 0xFFFFFFFF ||
        *(const uint32_t *)last_index == 0)
    {
        memset(dest, 0, sizeof(mem_save_t));
        for (uint32_t i = 0; i < sizeof(dest->tabletune); i++)
            dest->tabletune[i] = 50;
        return;
    }

    memcpy(dest, (const void *)last_index, sizeof(mem_save_t));
}

void mem_write(mem_save_t *source)
{
    uint32_t last_index, length;

    if (source->index == 0xFFFFFFFF)
    {
        source->index = 0;
    }

    last_index = MEM_START_ADDR + ((source->index * DATA_SIZE_SLOT) % (DATA_SIZE_SLOT * DATA_SLOT_MAX));
   
    source->index++;
    
    if ((last_index % PAGE_SIZE) == 0)
    {
        FLASH_Erase_Page(last_index);
    }

    length = sizeof(mem_save_t);
    fmc_unlock();

    for (uint32_t i = 0; i < length; i += 4)
    {
        uint32_t buff = *(uint32_t *)(((uint8_t *)source + i));

        fmc_word_program(last_index + i, buff);

        osThreadYield();
        fmc_flag_clear(FMC_FLAG_BANK0_END);
        fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
        fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    }

    fmc_lock();
}
