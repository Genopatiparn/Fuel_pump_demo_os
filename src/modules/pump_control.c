#include "main.h"

void updatePwmAdc(uint16_t duty)
{
    // duty: 0-1000 (0-100%)
    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_1, duty * 10);
    timer_channel_output_pulse_value_config(TIMER2, TIMER_CH_0, duty * 2);
}

void updatePwmLed(uint16_t duty)
{
  if (duty > 100)
    duty = 100;

  timer_channel_output_pulse_value_config(TIMER1, TIMER_CH_0, duty);
}

void updateCurrentMeasurement(void)
{
  uint16_t adcValue = adc_regular_data_read(ADC0);
  float current = (float)adcValue * 0.52f;
  float filteredCurrent = (float)pump.realCurrent +
                          (0.3f * (current - (float)pump.realCurrent));

  pump.realCurrent = (filteredCurrent > 0.0f) ? (uint16_t)filteredCurrent : 0;
}

FlagStatus checkFuelPumpOn(void)
{
    static uint16_t countcheck = 0;

    if ((gpio_input_bit_get(PUMP_STATUS_PORT, PUMP_STATUS_PIN) == RESET) && countcheck < 150)
    {
        countcheck++;
    }
    else
    {
        if (countcheck > 0)
            countcheck--;
    }
    
    if (countcheck > 100)
        return SET;
    else
        return RESET;
}

void controlPump(uint16_t power)
{
  uint16_t duty = 0;

  // Simple proportional control for R1k load testing
  if (power > 0) {
    duty = power * 2;  // power 0-500 → duty 0-1000
    if (duty > 1000) duty = 1000;
    if (duty < 100) duty = 100;  // Minimum 10%
  } else {
    duty = 0;
  }
  updatePwmAdc(duty);
}

  /* PID CONTROLLER (for real pump with current feedback)
  uint16_t duty = 400;
  float kp = 7.0f;
  float kd = 1.0f;
  float ki = 5.0f;
  static float lastError = 0;
  static float integral = 0;
  float error;
  float target = power;
  float current;
  uint16_t adcValue = adc_regular_data_read(ADC0);
  
  if (pump.controlDelay < 600)
  {
    pump.controlDelay++;
    if (adcValue < 3800)
      target = 2000;
  }

  current = ((float)adcValue * 0.52f);
  pump.realCurrent += 0.3 * (current - pump.realCurrent);
  error = target - current;
  integral += error;
  float derivative = error - lastError;
  lastError = error;
  duty = (uint16_t)(kp * error + ki * integral + kd * derivative);
  duty = (duty > 1000) ? 1000 : duty;
  
  if (integral > 500)
    integral = 500;
  if (integral < -500)
    integral = -500;

  updatePwmAdc(duty);
  */ 