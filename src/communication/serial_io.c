#include "main.h"

uint8_t _serialDataIn[100];

void USART0_IRQHandler(void)
{
    static uint16_t index = 0;

    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RBNE) == SET)
    {
        _serialDataIn[index++] = (uint8_t)usart_data_receive(USART0);
        index %= 100;
        _serialDataIn[index] = '\0';
    }
    
    if (usart_flag_get(USART0, USART_FLAG_PERR) == SET)
    {
        usart_flag_clear(USART0, USART_FLAG_PERR);
    }

    if (usart_interrupt_flag_get(USART0, USART_INT_FLAG_RT) == SET)
    {
        usart_interrupt_flag_clear(USART0, USART_INT_FLAG_RT);
        _BUG;
        osEventFlagsSet(pump.eventFlagsSerial, 0x0001);
        index = 0;
    }
    
    usart_flag_clear(USART0, USART_FLAG_NERR);
    usart_flag_clear(USART0, USART_FLAG_FERR);
    usart_flag_clear(USART0, USART_FLAG_ORERR);
}

void SerialSendByteHex(uint8_t data)
{
    usart_data_transmit(USART0, (uint32_t)data);
    while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
    {
        osThreadYield();
    }
}

void SerialSendHex(uint8_t *data, uint16_t len)
{
    for (uint8_t i = 0; i < len; i++)
    {
        usart_data_transmit(USART0, (uint8_t)*(data + i));
        while (usart_flag_get(USART0, USART_FLAG_TBE) == RESET)
        {
            osThreadYield();
        }
    }
}

void service_serialPort(void *pvParameters)
{
    (void)pvParameters;
    uint8_t temp[10];
    
    pump.eventFlagsSerial = osEventFlagsNew(NULL);
    
    while (1)
    {
        osEventFlagsWait(pump.eventFlagsSerial, 0x0001, osFlagsWaitAny, osWaitForever);
        osEventFlagsClear(pump.eventFlagsSerial, 0x0001);
        
        if (_serialDataIn[0] == 0x7F)
        {
            SerialSendByteHex(0x79);
        }
        else if ((_serialDataIn[0] == 0) && (_serialDataIn[1] == 0xFF))
        {
            temp[0] = 0x79;                                    // ack
            temp[1] = 0x03;                                    // brand
            temp[2] = _deviceType;                             // type
            temp[3] = (uint8_t)((uint16_t)_firmware >> 8);     // version high
            temp[4] = (uint8_t)_firmware;                      // version low
            temp[5] = pump.ecuLock == RESET ? 2 : 0;           // status lock ecu
            temp[6] = temp[1] ^ temp[2] ^ temp[3] ^ temp[4] ^ temp[5]; // checksum
            SerialSendHex(temp, 7);
        }
        else if ((_serialDataIn[0] == 0x81) && (_serialDataIn[1] == 0x7E)) // Jump to bootloader
        {
            fmc_unlock();
            fmc_word_reprogram(__addressStartProgram, 0);
            fmc_lock();
            SerialSendByteHex(0x79);
            osDelay(10);
            __disable_irq();
            NVIC_SystemReset();
            while (1);
        }
        else if ((_serialDataIn[0] == 0x83) && (_serialDataIn[1] == 0x7C)) // Save token
        {
            SerialSendByteHex(0x79);
            save_token();
        }
        
        memset(_serialDataIn, 0, 100);
    }
}

