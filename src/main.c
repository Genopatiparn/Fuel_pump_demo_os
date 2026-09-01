#include "main.h"

const char *__text = __StringVersion;
pump_t pump;
BlueTooth_module BT;

osThreadAttr_t thread_attr = {
    .name = "MainTask",
    .stack_size = 1024,
    .priority = osPriorityNormal,
};

void init_0(void)
{
    pump.xrpm = &pump.mem_save.setup[0];
    pump.yduty = &pump.mem_save.setup[12];
    pump.mode = &pump.mem_save.setup[18];
    pump.enginetype = &pump.mem_save.setup[19];
    pump.cylinder = &pump.mem_save.setup[20];
    pump.brand = &pump.mem_save.setup[21];
    pump.password = &pump.mem_save.setup[22];

    pump.Mac0 = (char *)&pump.mem_save.setup[23];
    pump.Mac1 = pump.Mac0 + 1;
    pump.Mac2 = (char *)&pump.mem_save.setup[24];
    pump.Mac3 = pump.Mac2 + 1;

    pump.Band0 = (char *)&pump.mem_save.setup[25];
    pump.Band1 = pump.Band0 + 1;
    pump.Band2 = (char *)&pump.mem_save.setup[26];
    pump.Band3 = pump.Band2 + 1;

    pump.o2type = (unsigned char *)&pump.mem_save.setup[27];
    pump.temptype = pump.o2type + 1;

    BT.mobilename = (char *)&pump.mem_save.setup[28];

    BT.buffname[0] = '\0';
    BT.buffname[1] = '\0';
    BT.buffname[2] = '\0';
    BT.buffname[3] = '\0';

    pump.xrpm[0] = 1200;
    pump.xrpm[1] = 1982;
    pump.xrpm[2] = 2764;
    pump.xrpm[3] = 3545;
    pump.xrpm[4] = 4327;
    pump.xrpm[5] = 5109;
    pump.xrpm[6] = 5891;
    pump.xrpm[7] = 6673;
    pump.xrpm[8] = 7455;
    pump.xrpm[9] = 8236;
    pump.xrpm[10] = 9018;
    pump.xrpm[11] = 9800;

    pump.yduty[0] = 90;
    pump.yduty[1] = 70;
    pump.yduty[2] = 50;
    pump.yduty[3] = 30;
    pump.yduty[4] = 10;
    pump.yduty[5] = 2;

    mem_read(&pump.mem_save);

    pump._Ftestoutput = RESET;
    BT._Fadduser = RESET;
    BT._Fsetpower = RESET;      // Don't send AT+POWR anymore
    BT._Fsetconni = RESET;      // Don't send AT+CONN anymore
    BT._FATrespond = RESET;
    BT._FATmode = RESET;        // Exit AT mode immediately
    BT._Fgetname = SET;         // Pretend we got name already
    BT._Fgetmac = SET;          // Pretend we got MAC already
}

int main()
{
    pump.ecuLock = RESET;

    module_init();

    osKernelInitialize();
    osThreadNew(vMainTask, NULL, &thread_attr);
    osThreadNew(task_bluetooth, NULL, &thread_attr);
    osThreadNew(service_serialPort, NULL, &thread_attr);

    if (osKernelGetState() == osKernelReady)
    {
        osKernelStart();
    }

    return 0;
}

uint16_t _write(uint16_t file, char *data, uint16_t len)
{
    for (uint16_t i = 0; i < len; i++)
    {
        usart_data_transmit(uarttune, (uint8_t)data[i]);
        while (RESET == usart_flag_get(uarttune, USART_FLAG_TBE))
        {
            osThreadYield();
            Nop();
        }
    }
    return len;
}

void save_token(void)
{
    union
    {
        uint8_t dummy[32];
        uint32_t dummy32[8];
    } d;

    uint8_t id[12];
    uint32_t temp = 0;
    
    if (pump.ecuLock == RESET)
        return;

    memcpy(id, (uint32_t *)(0x1FFFF7E8), 12);
    
    d.dummy[0] = (uint8_t)osKernelGetTickCount();
    d.dummy[1] = 12 ^ d.dummy[0];
    d.dummy[2] = id[0] ^ d.dummy[0];
    d.dummy[3] = id[1] ^ d.dummy[0];
    d.dummy[4] = id[2] ^ d.dummy[0];
    d.dummy[5] = id[3] ^ d.dummy[0];
    d.dummy[6] = id[4] ^ d.dummy[0];
    d.dummy[7] = id[5] ^ d.dummy[0];
    d.dummy[8] = id[6] ^ d.dummy[0];
    d.dummy[9] = id[7] ^ d.dummy[0];
    d.dummy[10] = id[8] ^ d.dummy[0];
    d.dummy[11] = id[9] ^ d.dummy[0];
    d.dummy[12] = id[10] ^ d.dummy[0];
    d.dummy[13] = id[11] ^ d.dummy[0];
    d.dummy[14] = 0x12 ^ d.dummy[0];
    d.dummy[15] = 0x34 ^ d.dummy[0];

    for (uint8_t i = 0; i < 16; i++)
    {
        d.dummy[(i + 16)] = d.dummy[i] ^ 0x55;
    }

    fmc_unlock();
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    fmc_bank0_ready_wait(FMC_TIMEOUT_COUNT);
    fmc_page_erase(__addressStartProgram - 2048);
    fmc_bank0_ready_wait(FMC_TIMEOUT_COUNT);
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    fmc_lock();
    
    temp = osKernelGetTickCount() ^ 0xF4D68534;
    fmc_unlock();
    
    for (uint16_t i = 2048; i > 32; i -= 4)
    {
        temp += *((uint32_t *)(__addressStartProgram + 0x123 + i));
        fmc_word_program(__addressStartProgram - i, temp);
    }
    
    fmc_word_reprogram(__addressStartProgram - 32, d.dummy32[0]);
    fmc_word_reprogram(__addressStartProgram - 28, d.dummy32[1]);
    fmc_word_reprogram(__addressStartProgram - 24, d.dummy32[2]);
    fmc_word_reprogram(__addressStartProgram - 20, d.dummy32[3]);
    fmc_word_reprogram(__addressStartProgram - 16, d.dummy32[4]);
    fmc_word_reprogram(__addressStartProgram - 12, d.dummy32[5]);
    fmc_word_reprogram(__addressStartProgram - 8, d.dummy32[6]);
    fmc_word_reprogram(__addressStartProgram - 4, d.dummy32[7]);
    fmc_lock();
    
    pump.ecuLock = RESET;
}

FlagStatus read_token(void)
{
    uint8_t dummy[32];
    uint8_t id[12];

    memcpy(id, (uint32_t *)(0x1FFFF7E8), 12);
    memcpy(dummy, (uint32_t *)(__addressStartProgram - 32), 32);

    for (uint8_t i = 0; i < 16; i++)
    {
        if (dummy[i] != (dummy[(i + 16)] ^ 0x55))
        {
            return SET; // Token invalid
        }
    }
    
    for (uint8_t i = 0; i < 12; i++)
    {
        if (id[i] != (dummy[(i + 2)] ^ dummy[0]))
            return SET; // Token mismatch
    }

    return RESET; // Token valid
}

FlagStatus checkIdCpu(void)
{
    uint32_t address, _data0, _data1, _data2;

    address = FLASH_BASE_END - 0x800 + 4;

    _data2 = (uint32_t)((*(__IO uint32_t *)(uint32_t)address));
    address += 4;
    _data1 = (uint32_t)((*(__IO uint32_t *)(uint32_t)address));
    address += 4;
    _data0 = (uint32_t)((*(__IO uint32_t *)(uint32_t)address));
    address += 4;

    _data0 ^= 0x12345678;
    _data1 ^= 0x12345678;
    _data2 ^= 0x12345678;

    if (_data0 != GetUID_Word0)
        return RESET;
    if (_data1 != GetUID_Word1)
        return RESET;
    if (_data2 != GetUID_Word2)
        return RESET;

    return SET;
}

void FLASH_Program_Word(unsigned long address, unsigned long dest)
{
    fmc_unlock();
    fmc_word_program(address, dest);
    fmc_flag_clear(FMC_FLAG_BANK0_END);
    fmc_flag_clear(FMC_FLAG_BANK0_WPERR);
    fmc_flag_clear(FMC_FLAG_BANK0_PGERR);
    fmc_lock();
}

void save_cpuid(void)
{
    uint32_t address;
    uint32_t dummy = 0x33457F0D;

    address = FLASH_BASE_END - 0x800;
    FLASH_Program_Word(address, dummy);
    address += 4;
    dummy += 0x12345678;
    FLASH_Program_Word(address, (GetUID_Word2 ^ dummy));
    address += 4;
    dummy += 0x12345678;
    FLASH_Program_Word(address, (GetUID_Word1 ^ dummy));
    address += 4;
    dummy += 0x12345678;
    FLASH_Program_Word(address, (GetUID_Word0 ^ dummy));

    address += 4;
    dummy += 0x11111111;
    FLASH_Program_Word(address, (GetUID_Word2 ^ dummy));
    address += 4;
    dummy += 0x22222222;
    FLASH_Program_Word(address, (GetUID_Word1 ^ dummy));
    address += 4;
    dummy += 0x33333333;
    FLASH_Program_Word(address, (GetUID_Word0 ^ dummy));
    address += 4;
    dummy += 0x44444444;
    FLASH_Program_Word(address, (GetUID_Word2 ^ dummy));
    address += 4;
    dummy += 0x55555555;
    FLASH_Program_Word(address, (GetUID_Word1 ^ dummy));
    address += 4;
    dummy += 0x66666666;
    FLASH_Program_Word(address, (GetUID_Word0 ^ dummy));
}
