#include "main.h"

void init_gpio()
{
    // LED output (PA15)
    gpio_init(LED_PORT, GPIO_MODE_OUT_OD, GPIO_OSPEED_2MHZ, LED_PIN);
    gpio_bit_write(LED_PORT, LED_PIN, SET);
    
    // Fuel pump enable input (PC8) - active low
    gpio_init(GPIOC, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_2MHZ, GPIO_PIN_8);
}
