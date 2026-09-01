#include "main.h"

void initTxBuffers(void)
{
    BT.bufferReadIndex = 0;
    BT.bufferWriteIndex = 0;
    BT.buffersUsed = 0;
    BT.dmaBusy = RESET;

    for (uint8_t i = 0; i < 5; i++)
    {
        BT.bufferSizes[i] = 0;
        memset(BT.txBuffer[i], 0, 100);
    }
}

FlagStatus addToTxBuffer(char *data, unsigned char size)
{
    // check if buffer full
    if (BT.buffersUsed >= 5)
    {
        return RESET;
    }

    // check size
    if (size > 90 || size == 0)
    {
        return RESET;
    }

    // copy data to write buffer
    memcpy(&BT.txBuffer[BT.bufferWriteIndex], data, size);
    BT.bufferSizes[BT.bufferWriteIndex] = size;

    // update write index
    BT.bufferWriteIndex = (BT.bufferWriteIndex + 1) % 5;
    BT.buffersUsed++;

    // start transmission if DMA not busy
    if (BT.dmaBusy == RESET)
    {
        serviceDmaInterrupt();
    }

    return SET;
}

void serviceDmaInterrupt(void)
{
    // check if there are buffers to send
    if (BT.buffersUsed == 0)
    {
        BT.dmaBusy = RESET;
        return;
    }

    // check if previous transmission complete
    if (dma_flag_get(DMA0, DMA_CH1, DMA_FLAG_FTF) == RESET && BT.dmaBusy == SET)
    {
        // move to next buffer
        BT.bufferReadIndex = (BT.bufferReadIndex + 1) % 5;
        BT.buffersUsed--;

        if (BT.buffersUsed == 0)
        {
            BT.dmaBusy = RESET;
            return;
        }
    }

    // start transmission
    if (BT.buffersUsed > 0)
    {
        BT.dmaBusy = SET;

        dma_channel_disable(DMA0, DMA_CH1);
        dma_memory_address_config(DMA0, DMA_CH1, (uint32_t)&BT.txBuffer[BT.bufferReadIndex]);
        dma_transfer_number_config(DMA0, DMA_CH1, BT.bufferSizes[BT.bufferReadIndex]);
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
