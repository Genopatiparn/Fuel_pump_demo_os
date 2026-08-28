#include <FreeRTOS.h>
#include <task.h>
#include <FreeRTOSConfig.h>
#include <string.h>
#include <stdarg.h>
#include <stdio.h>

void vApplicationIdleHook(void)
{
    //idle hook
}

void vApplicationTickHook(void)
{
    //tick hook
}

void vApplicationMallocFailedHook(void)
{
    volatile unsigned long looping = 0;
    taskDISABLE_INTERRUPTS();
    while (looping == 0LU)
    {
        portNOP();
    }
}

void vApplicationStackOverflowHook(TaskHandle_t xTask, char *pcTaskName)
{
    (void)xTask;
    (void)pcTaskName;
    volatile unsigned long looping = 0;
    taskDISABLE_INTERRUPTS();
    while (looping == 0LU)
    {
        portNOP();
    }
}
