#include "Arduino.h"
#include "bled_595.h"

uint8_t bled_val;
bool bled_process_flag = false;

TaskHandle_t bledTaskHandle = NULL;

void bled_write(int index, int value)
{
    uint8_t mask        = (1 << index);
    uint8_t indexed_bit = value << index;
    bled_val &= ~mask;
    bled_val |= indexed_bit;
    bled_process_flag = true;
}

void bled_task(void *pvParameters)
{
    while (1)
    {
        if (bled_process_flag == true)
        {
            shiftOut(bled_data, bled_clock, MSBFIRST, bled_val);
            digitalWrite(bled_latch, LOW);
            digitalWrite(bled_latch, HIGH);
            bled_process_flag = false;
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void bled_init(void)
{
    bled_val          = 0b00000000;
    bled_process_flag = true;
    pinMode(bled_latch, OUTPUT);
    pinMode(bled_clock, OUTPUT);
    pinMode(bled_data, OUTPUT);

    xTaskCreate(bled_task, "bled Task", 1024, NULL, 2, &bledTaskHandle);
    Serial.println("bled: \t [init]");
}