#include "main.h"

void moduls_powr_init()
{
    rcu_periph_clock_enable(RCU_AF);
    rcu_periph_clock_enable(RCU_GPIOA);
    rcu_periph_clock_enable(RCU_GPIOB);
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_ADC0);
    rcu_periph_clock_enable(RCU_DMA0);
    rcu_periph_clock_enable(RCU_USART0);

    rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV12);
    gpio_pin_remap_config(GPIO_SWJ_SWDPENABLE_REMAP, ENABLE);
}

void module_init(void)
{
    nvic_priority_group_set(NVIC_PRIGROUP_PRE4_SUB0);
    moduls_powr_init();
    init_gpio();
    init_adc();
    init_PwmFet();
    init_uart_bluetooth();
    init_LED();
    init_injector();
    init_uart0();
}

void vAssertCalled(void)
{
    volatile unsigned long looping = 0;
    volatile uint32_t debug_lr = 0;
    volatile uint32_t debug_pc = 0;

    __asm volatile("mov %0, lr" : "=r"(debug_lr));
    __asm volatile("mov %0, pc" : "=r"(debug_pc));

    __disable_irq();  // disable interrupts instead of taskENTER_CRITICAL
    {
        while (looping == 0LU)
        {
            __NOP(); 
        }
    }
    __enable_irq();  // enable interrupts instead of taskEXIT_CRITICAL
}
