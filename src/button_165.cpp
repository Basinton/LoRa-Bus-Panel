#include "button_165.h"
#include "board.h"
#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

TaskHandle_t b165TaskHandle = NULL;

uint8_t b165_val;

uint32_t key_code[BUTTON_N]         = {0};
uint32_t press_count[BUTTON_N]      = {0};
uint32_t key_code_timeout[BUTTON_N] = {0};

uint8_t b165_read(int index)
{
    return (b165_val >> index) & 1;
}

uint8_t b165_reg()
{
    return b165_val;
}

void b165_task(void *pvParameters)
{
    while (1)
    {
        digitalWrite(b_pload, LOW);
        digitalWrite(b_pload, HIGH);
        digitalWrite(b_clock, HIGH);

        b165_val = shiftIn(B_DATA, b_clock, MSBFIRST);

        uint8_t _i = 0;
        for (_i = 0; _i < BUTTON_N; _i++)
        {
            if (b165_read(_i) == LOW)
            {
                key_code[_i]++;
            }
            else
            {
                if (key_code[_i] > 5)
                {
                    press_count[_i]++;
                    key_code_timeout[_i] = 40;

                    Serial.printf("but: \t %d times\n", press_count[_i]);
                }

                key_code[_i] = 0;
            }

            if (key_code_timeout[_i] > 0)
            {
                key_code_timeout[_i]--;
            }
            else
            {
                press_count[_i] = 0;
            }
        }

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void b165_init(void)
{
    b165_val = 0b00000000;

    pinMode(B_DATA, INPUT);
    pinMode(b_clock, OUTPUT);
    pinMode(b_pload, OUTPUT);

    xTaskCreate(b165_task, "b165 Task", 4096, NULL, configMAX_PRIORITIES, &b165TaskHandle);
    Serial.println("b165: \t [init]");
}