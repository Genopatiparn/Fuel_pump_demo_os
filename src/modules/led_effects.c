#include "main.h"

void updateLed(void)
{
    static uint16_t brightness = 0;
    static int8_t direction = 1; // 1 for up, -1 for down
    static uint8_t speed_divider = 0;

    speed_divider++;
    if (speed_divider < 10)
        return; // Slow down the dimming effect
    speed_divider = 0;

    // Update brightness based on direction
    if (direction == 1)
    {
        brightness += 1; // Step size for dimming up
        if (brightness >= 150)
        { // Max brightness
            brightness = 150;
            direction = -1; // Change to dimming down
        }
    }
    else
    {
        if (brightness <= 1)
        {
            brightness = 0;
            direction = 1; // Change to dimming up
        }
        else
        {
            brightness -= 1; // Step size for dimming down
        }
    }

    updatePwmLed(brightness > 100 ? 101 : brightness);
}
