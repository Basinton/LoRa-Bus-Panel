#include <Arduino.h>

#include "led7.h"
#include "bled_595.h"
#include "button_165.h"
#include "board.h"
#include "rs485.h"
#include "led.h"

void setup()
{
    Serial.begin(115200);

    pinMode(5, OUTPUT);

    led7_init();
    bled_init();
    b165_init();

    rs485_init();
    board_init();

    led_init();
}

uint32_t startCycle = 0;
void loop()
{
    while (millis() - startCycle < 50)
        ;
    startCycle = millis();

    rs485_task();
}