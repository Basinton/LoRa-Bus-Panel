/* Includes ------------------------------------------------------------------*/
#include "rs485.h"

#include "Arduino.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include <SoftwareSerial.h>

#include "crc16.h"
#include "board.h"

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
TaskHandle_t rs485TaskHandle = NULL;

int RS485_RX_length                     = 0;
char RS485_RX_buffer[RS485_RX_BUF_SIZE] = {0};

RS485_MODE RS485_mode = RS485_RECEIVE;
SoftwareSerial RS485_Serial(RS485_RXD_PIN, RS485_TXD_PIN);

/* Functions -----------------------------------------------------------------*/
uint8_t CRC16_check(char *buffer, int length)
{
    uint16_t calculated_crc = CRC16(buffer, length - 2);

    // Extract the 2-byte CRC from the end of the buffer
    uint16_t received_crc = buffer[length - 1] << 8 | (buffer[length - 2]);

    // Compare the calculated CRC with the received CRC
    if (calculated_crc == received_crc)
    {
        return 1;
    }

    // CRC check failed
    Serial.printf("CRC16: Calculated 0x%04X --- Recieved 0x%04X\n", calculated_crc, received_crc);

    return 0;
}

void rs485_setmode(RS485_MODE mode)
{
    if (mode == RS485_TRANSMIT)
    {
        digitalWrite(RS485_EN, HIGH);
    }
    else
    {
        while (!RS485_Serial.availableForWrite())
            ;

        digitalWrite(RS485_EN, LOW);
    }
}

void rs485_task(void)
{
    RS485_RX_length = RS485_Serial.readBytes(RS485_RX_buffer, RS485_RX_BUF_SIZE);
    for (int responseIndex = 0; responseIndex < RS485_RX_length / 4; ++responseIndex)
    {
        int startIndex = responseIndex * 4;

        char *currentResponse = RS485_RX_buffer + startIndex;

        // Serial.print("rs485: \t ");
        // for (int i = 0; i < 4; i++)
        // {
        //     Serial.print(currentResponse[i], HEX);
        //     Serial.print(" ");
        // }
        // Serial.println();

        if (CRC16_check(currentResponse, 4))
        {
            switch (currentResponse[0])
            {
                case STATION_NOTIFY_ACCEPT_TO_BOARD:
                    if (boardState == REQUEST_TO_STATION)
                    {
                        isStationAccept = 1;
                    }

                    break;

                case STATION_NOTIFY_BUS_ACCEPT_TO_BOARD:
                    if (boardState == REQUEST_TO_BUS)
                    {
                        isBusAccept = 1;
                    }
                    else
                    {
                        isReAckBusAccept = 1;
                    }

                    break;

                case STATION_NOTIFY_BUS_PASS_TO_BOARD:
                    if (boardState == BUS_ACCEPT)
                    {
                        isBusPass = 1;
                    }
                    else
                    {
                        isReAckBusPass = 1;
                    }

                    break;

                case STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD:
                    if (boardState != WAITING)
                    {
                        isBusCancel = 1;
                    }
                    else
                    {
                        isReAckBusCancel = 1;
                    }
                    break;

                case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
                    if (boardState == BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION)
                    {
                        isPassengerCancelAck = 1;
                    }

                default:
                    break;
            }
        }
    }
}

void rs485_init()
{
    RS485_Serial.begin(RS485_BAUDRATE);

    pinMode(RS485_EN, OUTPUT);
    rs485_setmode(RS485_RECEIVE);

    // xTaskCreate(rs485_task, "Board Task", 4096, NULL, 2, &rs485TaskHandle);

    Serial.println("rs485: \t [init]");
}