#include "Arduino.h"
#include "led7.h"

uint8_t led7_matrix[10] = {
    0b01000001,
    0b11011101,
    0b10100001,
    0b10001001,
    0b00011101,
    0b00001011,
    0b00000011,
    0b11011001,
    0b00000001,
    0b00001001};
uint8_t led7_val[NO_OF_LED7] = {led7_matrix[0]};
bool process_flag = true;

TaskHandle_t led7TaskHandle = NULL;

void shiftOut_pro(uint8_t dataPin, uint8_t clockPin, uint8_t bitOrder, uint8_t val, uint8_t latch)
{
    uint8_t i;

    for (i = 0; i < 8; i++)
    {
        if (bitOrder == LSBFIRST)
            digitalWrite(dataPin, !!(val & (1 << i)));
        else
            digitalWrite(dataPin, !!(val & (1 << (7 - i))));

        digitalWrite(latch, LOW);
        digitalWrite(latch, HIGH);
        digitalWrite(clockPin, HIGH);
        digitalWrite(clockPin, LOW);
    }
}

void led7_write(int index, int value)
{
    led7_val[index] = led7_matrix[value % 10];
    process_flag = true;
    return;
}

void led7_write_pro(BUTTON_ID busstonID, uint8_t guestNumber)
{
    if (guestNumber < 10)
    {
        if (buttonID == BUTTON_0)
        {
            led7_write(buttonID * 2 + 1, 0);
            led7_write(buttonID * 2, guestNumber);
        }
        else
        {
            led7_write(buttonID - 1, 0);
            led7_write(buttonID - 2, guestNumber);
        }
    }
    else
    {
        if (buttonID == BUTTON_0)
        {
            led7_write(buttonID * 2 + 1, guestNumber / 10);
            led7_write(buttonID * 2, guestNumber % 10);
        }
        else
        {
            led7_write(buttonID - 1, guestNumber / 10);
            led7_write(buttonID - 2, guestNumber % 10);
        }
    }
}

void led7_task(void *pvParameters)
{
    while (1)
    {
        if (process_flag == true)
        {
            for (int i = 0; i < NO_OF_LED7; i++)
            {
                shiftOut_pro(led7_data, led7_clock, LSBFIRST, led7_val[NO_OF_LED7 - i - 1], led7_latch);
            }
            digitalWrite(led7_latch, LOW);
            digitalWrite(led7_latch, HIGH);
            process_flag = false;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
};

void led7_init(void)
{
    pinMode(led7_en, OUTPUT);
    pinMode(led7_latch, OUTPUT);
    pinMode(led7_clock, OUTPUT);
    pinMode(led7_data, OUTPUT);
    digitalWrite(led7_en, LOW);

    xTaskCreate(led7_task, "led7 Task", 2048, NULL, 2, &led7TaskHandle);
    Serial.println("led7: \t [init]");
}