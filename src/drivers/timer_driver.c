#include "main.h"

void init_PwmFet(void)
{
    rcu_periph_clock_enable(RCU_TIMER2);

    gpio_pin_remap_config(GPIO_TIMER2_PARTIAL_REMAP, ENABLE);
    gpio_init(PUMP_PWM_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_2MHZ, PUMP_PWM_PIN);
    
    timer_deinit(TIMER2);

    // 120MHz / 16 = 7.5MHz, 7.5MHz / 10000 = 750Hz PWM
    timer_parameter_struct timer_parameter = {
        .prescaler = 15,
        .clockdivision = TIMER_CKDIV_DIV1,
        .alignedmode = TIMER_COUNTER_EDGE,
        .counterdirection = TIMER_COUNTER_UP,
        .period = 10000,
        .repetitioncounter = 0
    };
    timer_init(TIMER2, &timer_parameter);
    
    timer_oc_parameter_struct timer_ocintpara = {
        .outputstate = TIMER_CCX_ENABLE,
        .outputnstate = TIMER_CCXN_DISABLE,
        .ocpolarity = TIMER_OC_POLARITY_LOW,
        .ocnpolarity = TIMER_OCN_POLARITY_HIGH,
        .ocidlestate = TIMER_OC_IDLE_STATE_LOW,
        .ocnidlestate = TIMER_OCN_IDLE_STATE_LOW,
    };

    // CH1: PWM output to pump
    timer_channel_output_config(TIMER2, TIMER_CH_1, &timer_ocintpara);
    timer_channel_output_mode_config(TIMER2, TIMER_CH_1, TIMER_OC_MODE_PWM0);
    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, 0);
    timer_channel_output_shadow_config(TIMER2, TIMER_CH_1, TIMER_OC_SHADOW_ENABLE);
    
    // CH0: trigger for ADC
    timer_channel_output_config(TIMER2, TIMER_CH_0, &timer_ocintpara);
    timer_channel_output_mode_config(TIMER2, TIMER_CH_0, TIMER_OC_MODE_TIMING);
    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_0, 0);
    timer_channel_output_shadow_config(TIMER2, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);
    
    // Configure to trigger ADC
    timer_master_output_trigger_source_select(TIMER2, TIMER_TRI_OUT_SRC_CH0);
    
    timer_auto_reload_shadow_enable(TIMER2);
    timer_automatic_output_enable(TIMER2);
    timer_enable(TIMER2);
}

void init_LED(void)
{
    rcu_periph_clock_enable(RCU_TIMER1);

    gpio_pin_remap_config(GPIO_TIMER1_PARTIAL_REMAP0, ENABLE);
    gpio_init(LED_PORT, GPIO_MODE_AF_OD, GPIO_OSPEED_2MHZ, LED_PIN);

    timer_deinit(TIMER1);

    // 120 MHz / 4001 / 100 = approximately 300 Hz PWM.
    timer_parameter_struct timer_parameter = {
        .prescaler = 4000,
        .clockdivision = TIMER_CKDIV_DIV4,
        .alignedmode = TIMER_COUNTER_EDGE,
        .counterdirection = TIMER_COUNTER_UP,
        .period = 100,
        .repetitioncounter = 0
    };
    timer_init(TIMER1, &timer_parameter);

    timer_oc_parameter_struct timer_ocintpara = {
        .outputstate = TIMER_CCX_ENABLE,
        .outputnstate = TIMER_CCXN_DISABLE,
        .ocpolarity = TIMER_OC_POLARITY_HIGH,
        .ocnpolarity = TIMER_OC_POLARITY_HIGH,
        .ocidlestate = TIMER_OC_IDLE_STATE_LOW,
        .ocnidlestate = TIMER_OC_IDLE_STATE_LOW,
    };
    timer_channel_output_config(TIMER1, TIMER_CH_0, &timer_ocintpara);
    timer_channel_output_mode_config(TIMER1, TIMER_CH_0, TIMER_OC_MODE_PWM1);
    timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, 0);
    timer_channel_output_shadow_config(TIMER1, TIMER_CH_0, TIMER_OC_SHADOW_ENABLE);
    timer_auto_reload_shadow_enable(TIMER1);
    timer_automatic_output_enable(TIMER1);
    timer_enable(TIMER1);
}

void init_injector(void)
{
    rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_TIMER7);
    
    // setup injector input pin
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_6);

    timer_deinit(TIMER7);
    
    // timer config: 120MHz / 120 = 1MHz (1us per tick)
    timer_parameter_struct timer_parameter = {
        .prescaler = 119,
        .clockdivision = TIMER_CKDIV_DIV1,
        .alignedmode = TIMER_COUNTER_EDGE,
        .counterdirection = TIMER_COUNTER_UP,
        .period = 65535,
        .repetitioncounter = 0
    };
    timer_init(TIMER7, &timer_parameter);
    
    // CH0: capture rising edge (start of pulse)
    timer_ic_parameter_struct cappara = {
        .icpolarity = TIMER_IC_POLARITY_RISING,
        .icselection = TIMER_IC_SELECTION_DIRECTTI,
        .icprescaler = TIMER_IC_PSC_DIV1,
        .icfilter = 0x0A,
    };
    timer_input_capture_config(TIMER7, TIMER_CH_0, &cappara);
    
    // CH1: capture falling edge (end of pulse)
    cappara.icpolarity = TIMER_IC_POLARITY_FALLING;
    cappara.icselection = TIMER_IC_SELECTION_INDIRECTTI;
    timer_input_capture_config(TIMER7, TIMER_CH_1, &cappara);

    // CH2: timeout detection
    timer_channel_output_mode_config(TIMER7, TIMER_CH_2, TIMER_OC_MODE_TIMING);
    
    // enable interrupt
    timer_interrupt_enable(TIMER7, TIMER_INT_CH0);
    nvic_irq_enable(TIMER7_Channel_IRQn, 15, 2);
    
    timer_enable(TIMER7);
}
