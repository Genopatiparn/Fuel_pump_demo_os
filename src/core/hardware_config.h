#ifndef __HARDWARE_CONFIG_H
#define __HARDWARE_CONFIG_H

#include "gd32f30x.h"

//GPIO pin defines
#define LED_PORT                GPIOA
#define LED_PIN                 GPIO_PIN_15


#define PUMP_PWM_PORT           GPIOB
#define PUMP_PWM_PIN            GPIO_PIN_5

#define PUMP_STATUS_PORT        GPIOC
#define PUMP_STATUS_PIN         GPIO_PIN_8

#define INJ_INPUT_PORT          GPIOC
#define INJ_INPUT_PIN           GPIO_PIN_6

#define CURRENT_PORT            GPIOC
#define CURRENT_PIN             GPIO_PIN_0

#define UART_TUNE_PORT          GPIOA
#define UART_TUNE_TX            GPIO_PIN_9
#define UART_TUNE_RX            GPIO_PIN_10

#define UART_BT_PORT            GPIOB
#define UART_BT_TX              GPIO_PIN_10
#define UART_BT_RX              GPIO_PIN_11

#define SWD_PORT                GPIOA
#define SWDIO_PIN               GPIO_PIN_13
#define SWCLK_PIN               GPIO_PIN_14

//defines
#define uarttune                USART0
#define Nop()                   asm("NOP")

#endif
