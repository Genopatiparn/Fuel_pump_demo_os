#include "main.h"

void calGeneral(void)
{
    uint32_t tickCap = osKernelGetTickCount() - pump.lastTickRpm;

    if (pump.newRpm == SET || tickCap > 1500)
    {
        pump.lastTickRpm = osKernelGetTickCount();

        if (tickCap > 1100)
        {
            tickCap = 1300000;  // ถือว่าเครื่องดับ
        }
        else if (tickCap > 60)
        {
            tickCap *= 1000;  // แปลง ms เป็น us
        }
        else
        {
            // ใช้ค่า period จาก timer capture
            tickCap = (pump.timeLastCapRpm + pump.timeCapRpm) & 0x0000FFFF;
        }
        
        pump.timeLastCapRpm = ~pump.timeCapRpm;

        // คำนวณ RPM และ Duty
        if (tickCap > 0 && tickCap < 1200000)
        {
            // RPM = 120,000,000 / period (in microseconds)
            pump.counterRpm = (int16_t)(120000000 / tickCap);
            
            // Duty = (pulse_width * 1000) / period
            pump.countDuty = (uint32_t)(pump.counterInj * 1000) / tickCap;
        }
        else
        {
            // เครื่องดับหรือ invalid
            pump.counterRpm = 0;
            pump.countDuty = 0;
        }

        // Low-pass filter สำหรับ RPM 
        pump.rpm += (float)(pump.counterRpm - pump.rpm) * 0.1f;
        
        // Filter pulse width
        pump.timeInjIn = pump.counterInj * 0.1f;

        // Filter duty cycle
        pump.tempCalDuty += (float)(pump.countDuty - pump.tempCalDuty) * 0.5f;
        pump.duty = (float)pump.tempCalDuty * 0.1f;

        // ถ้า timeout มาก = เครื่องดับ
        if (tickCap > 1200000)
        {
            pump.timeInjIn = 0;
            pump.rpm = 0;
            pump.tempCalDuty = 0;
            pump.duty = 0;
        }

        pump.newRpm = RESET;
    }
}

void TIMER7_Channel_IRQHandler(void)
{
    // CH0 interrupt: rising edge detected (start of pulse)
    if (timer_interrupt_flag_get(TIMER7, TIMER_INT_FLAG_CH0) != RESET)
    {
        timer_interrupt_flag_clear(TIMER7, TIMER_INT_FLAG_CH0);
        
        // setup timeout on CH2 (1000us = 1ms after rising edge)
        timer_interrupt_flag_clear(TIMER7, TIMER_INT_FLAG_CH2);
        timer_channel_output_pulse_value_config(TIMER7, TIMER_CH_2, 
            timer_counter_read(TIMER7) + 0x3E8);  // +1000us
        timer_interrupt_enable(TIMER7, TIMER_INT_CH2);
    }

    // CH1 interrupt: falling edge detected (end of pulse)
    if (timer_interrupt_flag_get(TIMER7, TIMER_INT_FLAG_CH1) != RESET)
    {
        timer_interrupt_flag_clear(TIMER7, TIMER_INT_FLAG_CH1);
    }

    // CH2 interrupt: timeout - pulse ended, now calculate
    if (timer_interrupt_flag_get(TIMER7, TIMER_INT_FLAG_CH2) != RESET)
    {
        // check if still HIGH (pulse still active)
        if (gpio_input_bit_get(INJ_INPUT_PORT, INJ_INPUT_PIN) == SET)
        {
            // capture the period (time between pulses)
            pump.timeCapRpm = timer_channel_capture_value_register_read(TIMER7, TIMER_CH_0);
            
            // capture pulse width
            pump.counterInj = ~timer_channel_capture_value_register_read(TIMER7, TIMER_CH_1) + pump.timeCapRpm;
            
            // signal that new data is ready
            pump.newRpm = SET;
        }

        timer_interrupt_disable(TIMER7, TIMER_INT_CH2);
        timer_interrupt_flag_clear(TIMER7, TIMER_INT_FLAG_CH2);
    }
}
