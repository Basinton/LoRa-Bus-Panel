#include "led.h"
#include <Arduino.h>

#include "bled_595.h"

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
void led_on(BUTTON_ID buttonID)
{
    bled_write(buttonID, 1);
}

void led_off(BUTTON_ID buttonID)
{
    bled_write(buttonID, 0);
}

void all_led_off()
{
    for (int led = 0; led < 4; led++)
    {
        bled_write(led, 0);
    }
}

void led_init()
{
    pinMode(PIN_ALARM, OUTPUT);
    all_led_off();

    Serial.println("led: \t [init]");
}
