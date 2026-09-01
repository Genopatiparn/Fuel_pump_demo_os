#include <FreeRTOS.h>
#include <task.h>
#include <FreeRTOSConfig.h>

void vAssertCalled(void)
{
	volatile unsigned long looping = 0;
	volatile uint32_t debug_lr = 0;
	volatile uint32_t debug_pc = 0;

	// Capture where assertion was called from
	__asm volatile("mov %0, lr" : "=r"(debug_lr));
	__asm volatile("mov %0, pc" : "=r"(debug_pc));

	taskENTER_CRITICAL();
	{
		/* Use the debugger to set ul to a non-zero value in order to step out
		 *      of this function to determine why it was called. */
		while (looping == 0LU)
		{
			portNOP();
		}
	}
	taskEXIT_CRITICAL();
}
