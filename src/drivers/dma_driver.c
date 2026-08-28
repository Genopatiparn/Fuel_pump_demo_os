#include "main.h"

void initTxBuffers(void)
{
    BT5.bufferReadIndex = 0;
    BT5.bufferWriteIndex = 0;
    BT5.buffersUsed = 0;
    BT5.dmaBusy = RESET;

    for (uint8_t i = 0; i < 5; i++)
    {
        BT5.bufferSizes[i] = 0;
        memset(BT5.txBuffer[i], 0, 100);
    }
}

FlagStatus addToTxBuffer(char *data, unsigned char size)
{
    // check if buffer full
    if (BT5.buffersUsed >= 5)
    {
        return RESET;
    }

    // check size
    if (size > 90 || size == 0)
    {
        return RESET;
    }

    // copy data to write buffer
    memcpy(&BT5.txBuffer[BT5.bufferWriteIndex], data, size);
    BT5.bufferSizes[BT5.bufferWriteIndex] = size;

    // update write index
    BT5.bufferWriteIndex = (BT5.bufferWriteIndex + 1) % 5;
    BT5.buffersUsed++;

    // start transmission if DMA not busy
    if (BT5.dmaBusy == RESET)
    {
        serviceDmaInterrupt();
    }

    return SET;
}

void serviceDmaInterrupt(void)
{
    // check if there are buffers to send
    if (BT5.buffersUsed == 0)
    {
        BT5.dmaBusy = RESET;
        return;
    }

    // check if previous transmission complete
    if (dma_flag_get(DMA0, DMA_CH1, DMA_FLAG_FTF) == RESET && BT5.dmaBusy == SET)
    {
        // move to next buffer
        BT5.bufferReadIndex = (BT5.bufferReadIndex + 1) % 5;
        BT5.buffersUsed--;

        if (BT5.buffersUsed == 0)
        {
            BT5.dmaBusy = RESET;
            return;
        }
    }

    // start transmission
    if (BT5.buffersUsed > 0)
    {
        BT5.dmaBusy = SET;

        dma_channel_disable(DMA0, DMA_CH1);
        dma_memory_address_config(DMA0, DMA_CH1, (uint32_t)&BT5.txBuffer[BT5.bufferReadIndex]);
        dma_transfer_number_config(DMA0, DMA_CH1, BT5.bufferSizes[BT5.bufferReadIndex]);
        dma_flag_clear(DMA0, DMA_CH1, DMA_FLAG_FTF);
        dma_channel_enable(DMA0, DMA_CH1);
    }
}

void DMA0_Channel1_IRQHandler(void)
{
    if (dma_flag_get(DMA0, DMA_CH1, DMA_INT_FLAG_FTF) == SET)
    {
        dma_flag_clear(DMA0, DMA_CH1, DMA_INT_FLAG_FTF);
        serviceDmaInterrupt();
    }
}
