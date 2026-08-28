#include "main.h"

void vMainTask(void *pvParameters)
{
    (void)pvParameters;

    adc_enable(ADC0);
    osDelay(5);
    adc_calibration_enable(ADC0);

    init_0();
    
    initTxBuffers();
    updatePwmLed(80);
    
    for (;;)
    {
        osDelay(1);

        pump.powerCal = calInterPolate2D();
        pump.currentControl = pump.powerCal * 5;

        if (checkFuelPumpOn() == SET)
        {
            controlPump(pump.currentControl);
        }
        else
        {
            pump.currentControl = 0;
            pump.powerCal = 0;
            updatePwmAdc(0);
            pump.controlDelay = 0;
        }

        updateCurrentMeasurement();
        pump.powerTune = pump.powerCal * 100;
        calGeneral();
        updateLed();
    }
}
