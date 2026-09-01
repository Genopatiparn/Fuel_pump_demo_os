#include "main.h"

void init_uart_bluetooth(void)
{
    rcu_periph_clock_enable(RCU_USART2);

    // TX = PB10, RX = PB11
    gpio_init(UART_BT_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, UART_BT_TX);
    gpio_init(UART_BT_PORT, GPIO_MODE_IN_FLOATING, GPIO_OSPEED_50MHZ, UART_BT_RX);

    // setup DMA for TX
    dma_parameter_struct dma_data_parameter = {
        .periph_addr = ((uint32_t)&USART_DATA(USART2)),
        .periph_inc = DMA_PERIPH_INCREASE_DISABLE,
        .memory_inc = DMA_MEMORY_INCREASE_ENABLE,
        .periph_width = DMA_PERIPHERAL_WIDTH_8BIT,
        .memory_width = DMA_MEMORY_WIDTH_8BIT,
        .direction = DMA_MEMORY_TO_PERIPHERAL,
        .number = 13,
        .priority = DMA_PRIORITY_LOW,
    };
    dma_init(DMA0, DMA_CH1, &dma_data_parameter);

    dma_circulation_disable(DMA0, DMA_CH1);
    dma_memory_to_memory_disable(DMA0, DMA_CH1);
    dma_channel_enable(DMA0, DMA_CH1);
    dma_interrupt_enable(DMA0, DMA_CH1, DMA_INT_FTF);
    nvic_irq_enable(DMA0_Channel1_IRQn, 13, 0);

    // USART config
    usart_deinit(USART2);
    usart_baudrate_set(USART2, 115200);
    usart_receive_config(USART2, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART2, USART_TRANSMIT_ENABLE);
    usart_parity_config(USART2, USART_PM_NONE);
    usart_word_length_set(USART2, USART_WL_8BIT);
    usart_stop_bit_set(USART2, USART_STB_1BIT);

    nvic_irq_enable(USART2_IRQn, 14, 0);
    usart_data_receive(USART2);

    usart_interrupt_enable(USART2, USART_INT_RBNE);

    // enable DMA for TX
    usart_dma_transmit_config(USART2, USART_DENT_ENABLE);

    usart_enable(USART2);
}

void init_uart0(void)
{
    // TX = PA9, RX = PA10
    gpio_init(UART_TUNE_PORT, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, UART_TUNE_TX);
    gpio_init(UART_TUNE_PORT, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, UART_TUNE_RX);

    usart_deinit(USART0);
    usart_baudrate_set(USART0, 115200);
    usart_receive_config(USART0, USART_RECEIVE_ENABLE);
    usart_transmit_config(USART0, USART_TRANSMIT_ENABLE);
    usart_parity_config(USART0, USART_PM_EVEN);
    usart_word_length_set(USART0, USART_WL_9BIT);
    usart_stop_bit_set(USART0, USART_STB_1BIT);

    usart_receiver_timeout_threshold_config(USART0, 1000);
    usart_receiver_timeout_enable(USART0);

    nvic_irq_enable(USART0_IRQn, 11, 0);
    usart_data_receive(USART0);

    usart_interrupt_enable(USART0, USART_INT_RBNE);
    usart_interrupt_enable(USART0, USART_INT_RT);

    usart_enable(USART0);
}
