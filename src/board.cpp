/* Includes ------------------------------------------------------------------*/
#include "board.h"
#include "button_165.h"
#include "crc16.h"
#include "rs485.h"
#include "led7.h"
#include "bled_595.h"
#include "led.h"

#include <Arduino.h>

/* Define --------------------------------------------------------------------*/

/* Variables -----------------------------------------------------------------*/
TaskHandle_t boardTaskHandle = NULL;

SYSTEM_STATE boardState = INIT;
uint8_t guestNumber     = 0;

uint8_t responseToStation[128] = {0};
uint16_t responseToStationCRC  = 0;

int boardTimeout = 0;

uint8_t isStationAccept = 0;
uint8_t isBusAccept     = 0;
uint8_t isBusPass       = 0;

uint8_t isReAckBusAccept       = 0;
uint8_t isReAckBusPass         = 0;
uint8_t isReAckBusCancel       = 0;
uint8_t isReAckPassengerCancel = 0;

uint8_t isBusCancel          = 0;
uint8_t isPassengerCancelAck = 0;

/* Functions -----------------------------------------------------------------*/
void boardAckToStation(SYSTEM_STATE state);
void cancelProcess(void);

void board_fsm_reset_state(SYSTEM_STATE state)
{
    switch (state)
    {
        case WAITING:
            led7_write(0, 0);
            led7_write(1, 0);

            isStationAccept = 0;
            isBusAccept     = 0;
            isBusPass       = 0;

            isReAckBusAccept       = 0;
            isReAckBusPass         = 0;
            isReAckBusCancel       = 0;
            isReAckPassengerCancel = 0;

            isBusCancel          = 0;
            isPassengerCancelAck = 0;

            Serial.println("board: \t [fsm] waiting");
            break;

        case REQUEST_TO_STATION:
            boardAckToStation(REQUEST_TO_STATION);
            boardTimeout = 40;

            Serial.println("board: \t [fsm] request to station");
            break;

        case REQUEST_TO_BUS:
            Serial.println("board: \t [fsm] station accept and request to bus");
            break;

        case BUS_ACCEPT:
            boardAckToStation(BUS_ACCEPT);

            Serial.println("board: \t [fsm] bus accepted");
            break;

        case BUS_PASS:
            guestNumber = 0;
            boardAckToStation(BUS_PASS);

            Serial.printf("board: \t [fsm] bus pass\n");
            break;

        case DRIVER_CANCEL:
            guestNumber = 0;
            boardAckToStation(DRIVER_CANCEL);

            Serial.printf("board: \t [fsm] bus cancel\n");
            break;

        case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
            boardAckToStation(PASSENGER_CANCEL);
            boardTimeout = 40;

            Serial.println("board: \t [fsm] notify passenger cancel");
            break;

        case PASSENGER_CANCEL:
            guestNumber = 0;
            Serial.println("board: \t [fsm] passenger cancel");

            break;

        default:
            break;
    }
}

void board_fsm(void)
{
    switch (boardState)
    {
        case INIT:
            led7_write(0, 0);
            led7_write(1, 0);

            Serial.println("board: \t [fsm] init");

            board_fsm_reset_state(WAITING);
            boardState = WAITING;
            break;

        case WAITING:
            if (guestNumber > 0)
            {
                board_fsm_reset_state(REQUEST_TO_STATION);
                boardState = REQUEST_TO_STATION;
            }
            break;

        case REQUEST_TO_STATION:
            if (isStationAccept)
            {
                isStationAccept = 0;

                board_fsm_reset_state(REQUEST_TO_BUS);
                boardState = REQUEST_TO_BUS;
            }
            else if (--boardTimeout == 0)
            {
                board_fsm_reset_state(REQUEST_TO_STATION);
                boardState = REQUEST_TO_STATION;
            }
            break;

        case REQUEST_TO_BUS:
            if (isBusAccept)
            {
                isBusAccept = 0;

                board_fsm_reset_state(BUS_ACCEPT);
                boardState = BUS_ACCEPT;
            }
            break;

        case BUS_ACCEPT:
            if (isBusPass)
            {
                isBusPass = 0;

                board_fsm_reset_state(BUS_PASS);
                boardState = BUS_PASS;
            }

            if (press_count[0] == 2)
            {
                board_fsm_reset_state(BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);
                boardState = BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION;
            }
            break;

        case BUS_PASS:
            board_fsm_reset_state(WAITING);
            boardState = WAITING;
            break;

        case DRIVER_CANCEL:
            board_fsm_reset_state(WAITING);
            boardState = WAITING;
            break;

        case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
            if (isPassengerCancelAck)
            {
                isPassengerCancelAck = 0;

                board_fsm_reset_state(PASSENGER_CANCEL);
                boardState = PASSENGER_CANCEL;
            }
            else if (--boardTimeout == 0)
            {
                board_fsm_reset_state(BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);
                boardState = BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION;
            }

            break;

        case PASSENGER_CANCEL:
            board_fsm_reset_state(WAITING);
            boardState = WAITING;
            break;

        default:
            boardState = INIT;
            break;
    }
}

void cancelProcess(void)
{
    if (isBusCancel)
    {
        isBusCancel = 0;

        board_fsm_reset_state(DRIVER_CANCEL);
        boardState = DRIVER_CANCEL;
    }
}

void guest_handler(void)
{
    static int key_code_timeout = 0;

    // read key_code and display LCD
    if (key_code[0] == 5)
    {
        guestNumber++;

        if (guestNumber == 100)
        {
            guestNumber = 99;
        }

        if (guestNumber < 10)
        {
            led7_write(1, 0);
            led7_write(0, guestNumber);
        }
        else
        {
            led7_write(1, guestNumber / 10);
            led7_write(0, guestNumber % 10);
        }
    }

    // display button led
    if (key_code_timeout > 0)
        key_code_timeout--;

    if (key_code[0] > 0)
    {
        key_code_timeout = 5;
        led_on();
    }
    else if (key_code_timeout == 0)
    {
        led_off();
    }
}

void station_ack_debuger(void)
{
    if (isReAckBusAccept)
    {
        isReAckBusAccept = 0;
        boardAckToStation(BUS_ACCEPT);
    }

    if (isReAckBusPass)
    {
        isReAckBusPass = 0;
        boardAckToStation(BUS_PASS);
    }

    if (isReAckBusCancel)
    {
        isReAckBusCancel = 0;
        boardAckToStation(DRIVER_CANCEL);
    }
}

void boardAckToStation(SYSTEM_STATE state)
{
    responseToStation[0] = state;
    responseToStationCRC = CRC16((char *)responseToStation, 1);
    responseToStation[1] = responseToStationCRC & 0xFF;
    responseToStation[2] = responseToStationCRC >> 8;
    responseToStation[3] = '\0';
    rs485_setmode(RS485_TRANSMIT);
    RS485_Serial.write((const uint8_t *)responseToStation, 4);
    rs485_setmode(RS485_RECEIVE);
}

void board_task(void *pvParameters)
{
    while (1)
    {
        cancelProcess();
        guest_handler();
        board_fsm();
        station_ack_debuger();

        vTaskDelay(pdMS_TO_TICKS(50));
    }
}

void board_init(void)
{
    xTaskCreate(board_task, "board Task", 8192, NULL, configMAX_PRIORITIES, &boardTaskHandle);
    Serial.println("board: \t [init]");
}