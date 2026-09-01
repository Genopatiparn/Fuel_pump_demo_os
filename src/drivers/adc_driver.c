#include "main.h"

void init_adc()
{
    // setup ADC pin (PC0 = ADC0_IN10)
    gpio_init(CURRENT_PORT, GPIO_MODE_AIN, GPIO_OSPEED_2MHZ, CURRENT_PIN);

    // ADC mode config
    adc_mode_config(ADC_MODE_FREE);
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, DISABLE);
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 1);
    adc_resolution_config(ADC0, ADC_RESOLUTION_12B);

    // external trigger from TIMER2
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_T2_TRGO);
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);

    // oversampling for noise reduction
    adc_oversample_mode_config(ADC0, ADC_OVERSAMPLING_ALL_CONVERT, 
                               ADC_OVERSAMPLING_SHIFT_1B, 
                               ADC_OVERSAMPLING_RATIO_MUL2);
    adc_oversample_mode_enable(ADC0);
    
    // channel 10 = PC0
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_10, ADC_SAMPLETIME_71POINT5);
}
