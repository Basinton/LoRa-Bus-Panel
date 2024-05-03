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

char debug_buffer[50];

uint8_t errorCount[BUTTON_COUNT] = {0};

BUTTON_ID buttonID = BUTTON_UNKNOWN;
SYSTEM_STATE boardState[BUTTON_COUNT] = {INIT};
uint8_t guestNumber[BUTTON_COUNT] = {0};

uint8_t responseToStation[128] = {0};
uint16_t responseToStationCRC = 0;

uint8_t boardTimeout[BUTTON_COUNT] = {0};

uint8_t isStationAccept[BUTTON_COUNT] = {0};
uint8_t isBusAccept[BUTTON_COUNT] = {0};
uint8_t isBusPass[BUTTON_COUNT] = {0};

uint8_t isReAckBusAccept[BUTTON_COUNT] = {0};
uint8_t isReAckBusPass[BUTTON_COUNT] = {0};
uint8_t isReAckBusCancel[BUTTON_COUNT] = {0};
uint8_t isReAckPassengerCancel[BUTTON_COUNT] = {0};

uint8_t isBusCancel[BUTTON_COUNT] = {0};
uint8_t isPassengerCancelAck[BUTTON_COUNT] = {0};

/* Functions -----------------------------------------------------------------*/
void boardAckToStation(BUTTON_ID buttonID, SYSTEM_STATE state);
void cancelProcess(void);

void board_fsm_reset_state(BUTTON_ID buttonID = BUTTON_UNKNOWN, SYSTEM_STATE state = INIT)
{
    switch (state)
    {
    case INIT:
        for (int ledIndex = 0; ledIndex < NO_OF_LED7; ledIndex++)
        {
            led7_write(ledIndex, 0); // Turn off all LEDs
        }
        memset(isStationAccept, 0, sizeof(isStationAccept));
        memset(isBusAccept, 0, sizeof(isBusAccept));
        memset(isBusPass, 0, sizeof(isBusPass));

        memset(isReAckBusAccept, 0, sizeof(isReAckBusAccept));
        memset(isReAckBusPass, 0, sizeof(isReAckBusPass));
        memset(isReAckBusCancel, 0, sizeof(isReAckBusCancel));
        memset(isReAckPassengerCancel, 0, sizeof(isReAckPassengerCancel));

        memset(isBusCancel, 0, sizeof(isBusCancel));
        memset(isPassengerCancelAck, 0, sizeof(isPassengerCancelAck));
        break;

    case WAITING:
        Serial.println("board: \t [fsm] waiting");
        break;

    case REQUEST_TO_STATION:
        boardAckToStation(buttonID, REQUEST_TO_STATION);
        boardTimeout[buttonID] = 40;
        sprintf(debug_buffer, "board: request bus %d to station", buttonID);
        Serial.println(debug_buffer);
        break;

    case REQUEST_TO_BUS:
        sprintf(debug_buffer, "board: station accept and request to bus %d", buttonID);
        Serial.println(debug_buffer);
        break;

    case BUS_ACCEPT:
        boardAckToStation(buttonID, BUS_ACCEPT);

        sprintf(debug_buffer, "board: bus %d accepted", buttonID);
        Serial.println(debug_buffer);
        break;

    case BUS_PASS:
        guestNumber[buttonID] = 0;
        led7_write_pro(buttonID, 0);
        boardAckToStation(buttonID, BUS_PASS);

        sprintf(debug_buffer, "board: bus %d pass", buttonID);
        Serial.println(debug_buffer);
        break;

    case DRIVER_CANCEL:
        guestNumber[buttonID] = 0;
        led7_write_pro(buttonID, guestNumber[buttonID]);
        boardAckToStation(buttonID, DRIVER_CANCEL);

        sprintf(debug_buffer, "board: bus %d cancel", buttonID);
        Serial.println(debug_buffer);
        break;

    case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
        if (long_press[buttonID])
        {
            guestNumber[buttonID]--;
            led7_write_pro(buttonID, guestNumber[buttonID]);
        }

        if (guestNumber[buttonID] == 0)
        {
            boardAckToStation(buttonID, PASSENGER_CANCEL);
            boardTimeout[buttonID] = 40;

            sprintf(debug_buffer, "board: notify passenger %d cancel", buttonID);
            Serial.println(debug_buffer);
        }

        break;

    case PASSENGER_CANCEL:
        guestNumber[buttonID] = 0;
        led7_write_pro(buttonID, guestNumber[buttonID]);

        sprintf(debug_buffer, "board: passenger %d cancel", buttonID);
        Serial.println(debug_buffer);
        break;

    case ERROR_TIMEOUT:
        sprintf(debug_buffer, "board: BUTTON %d error TIMEOUT", buttonID);
        Serial.println(debug_buffer);
        break;

    default:
        break;
    }
}

void board_fsm(void)
{
    for (int button_i = BUTTON_0; button_i <= BUTTON_4; button_i++)
    {
        buttonID = (BUTTON_ID)button_i; // Cast integer back to BUTTON_ID for usage

        switch (boardState[buttonID])
        {
        case INIT:
            board_fsm_reset_state(buttonID, INIT);
            boardState[buttonID] = WAITING;
            Serial.println("board: \t [fsm] init");
            break;

        case WAITING:
            if (guestNumber[buttonID] > 0)
            {
                sprintf(debug_buffer, "board: passenger count bus %d: %d", buttonID, guestNumber[buttonID]);
                Serial.println(debug_buffer);
                board_fsm_reset_state(buttonID, REQUEST_TO_STATION);
                boardState[buttonID] = REQUEST_TO_STATION;
            }
            break;

        case REQUEST_TO_STATION:
            if (isStationAccept[buttonID])
            {
                isStationAccept[buttonID] = 0;

                board_fsm_reset_state(buttonID, REQUEST_TO_BUS);
                boardState[buttonID] = REQUEST_TO_BUS;
            }
            else if (--boardTimeout[buttonID] == 0)
            {
                if (errorCount[buttonID] >= 20)
                {
                    errorCount[buttonID] = 0;
                    board_fsm_reset_state(buttonID, ERROR_TIMEOUT);
                    boardState[buttonID] = ERROR_TIMEOUT;
                }
                errorCount[buttonID]++;
                board_fsm_reset_state(buttonID, REQUEST_TO_STATION);
                boardState[buttonID] = REQUEST_TO_STATION;
            }
            break;

        case REQUEST_TO_BUS:
            if (isBusAccept[buttonID])
            {
                isBusAccept[buttonID] = 0;

                board_fsm_reset_state(buttonID, BUS_ACCEPT);
                boardState[buttonID] = BUS_ACCEPT;
            }
            break;

        case BUS_ACCEPT:
            if (isBusPass[buttonID])
            {
                isBusPass[buttonID] = 0;

                board_fsm_reset_state(buttonID, BUS_PASS);
                boardState[buttonID] = BUS_PASS;
            }

            if (long_press[buttonID])
            {
                Serial.println("cancle 1");
                board_fsm_reset_state(buttonID, BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);
                boardState[buttonID] = BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION;
            }
            break;

        case BUS_PASS:
            board_fsm_reset_state(buttonID, WAITING);
            boardState[buttonID] = WAITING;
            break;

        case DRIVER_CANCEL:
            board_fsm_reset_state(buttonID, WAITING);
            boardState[buttonID] = WAITING;
            break;

        case BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION:
            if (isPassengerCancelAck[buttonID])
            {
                isPassengerCancelAck[buttonID] = 0;

                board_fsm_reset_state(buttonID, PASSENGER_CANCEL);
                boardState[buttonID] = PASSENGER_CANCEL;
            }
            else if (guestNumber[buttonID] == 0 && --boardTimeout[buttonID] == 0)
            {
                if (errorCount[buttonID] >= 20)
                {
                    errorCount[buttonID] = 0;
                    board_fsm_reset_state(buttonID, ERROR_TIMEOUT);
                    boardState[buttonID] = ERROR_TIMEOUT;
                }
                errorCount[buttonID]++;
                board_fsm_reset_state(buttonID, BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);
                boardState[buttonID] = BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION;
            }
            else if (guestNumber[buttonID] > 0)
            {
                board_fsm_reset_state(buttonID, BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION);
            }
            break;

        case PASSENGER_CANCEL:
            board_fsm_reset_state(buttonID, WAITING);
            boardState[buttonID] = WAITING;
            break;

        case ERROR_TIMEOUT:
            board_fsm_reset_state(buttonID, WAITING);
            boardState[buttonID] = WAITING;
            break;

        default:
            boardState[buttonID] = INIT;
            break;
        }
    }
}

void cancelProcess(void)
{
    for (int button_i = BUTTON_0; button_i <= BUTTON_4; button_i++)
    {
        buttonID = (BUTTON_ID)button_i; // Cast integer back to BUTTON_ID for usage
        if (isBusCancel[buttonID])
        {
            isBusCancel[buttonID] = 0;

            board_fsm_reset_state(buttonID, DRIVER_CANCEL);
            boardState[buttonID] = DRIVER_CANCEL;
        }
    }
}

void guest_handler(void)
{
    static int key_code_timeout[BUTTON_COUNT] = {0};

    for (int button_i = BUTTON_0; button_i <= BUTTON_4; button_i++)
    {
        buttonID = (BUTTON_ID)button_i; // Cast integer back to BUTTON_ID for usage

        if (press_count[buttonID])
        {
            guestNumber[buttonID]++;

            if (guestNumber[buttonID] == 100)
            {
                guestNumber[buttonID] = 99;
            }

            if (guestNumber[buttonID] < 10)
            {
                if (buttonID == BUTTON_0)
                {
                    led7_write(buttonID * 2 + 1, 0);
                    led7_write(buttonID * 2, guestNumber[buttonID]);
                }
                else
                {
                    led7_write(buttonID - 1, 0);
                    led7_write(buttonID - 2, guestNumber[buttonID]);
                }
            }
            else
            {
                if (buttonID == BUTTON_0)
                {
                    led7_write(buttonID * 2 + 1, guestNumber[buttonID] / 10);
                    led7_write(buttonID * 2, guestNumber[buttonID] % 10);
                }
                else
                {
                    led7_write(buttonID - 1, guestNumber[buttonID] / 10);
                    led7_write(buttonID - 2, guestNumber[buttonID] % 10);
                }
            }
        }

        // display button led
        if (key_code_timeout[buttonID] > 0)
            key_code_timeout[buttonID]--;

        if (key_code[buttonID] > 0)
        {
            key_code_timeout[buttonID] = 5;
            led_on(buttonID);
        }
        else if (key_code_timeout[buttonID] == 0)
        {
            led_off(buttonID);
        }
    }
    // read key_code and display LCD
}

void station_ack_debuger(void)
{
    for (int button_i = BUTTON_0; button_i <= BUTTON_4; button_i++)
    {
        buttonID = (BUTTON_ID)button_i; // Cast integer back to BUTTON_ID for usage

        if (isReAckBusAccept[buttonID])
        {
            isReAckBusAccept[buttonID] = 0;
            boardAckToStation(buttonID, BUS_ACCEPT);
        }

        if (isReAckBusPass[buttonID])
        {
            isReAckBusPass[buttonID] = 0;
            boardAckToStation(buttonID, BUS_PASS);
        }

        if (isReAckBusCancel[buttonID])
        {
            isReAckBusCancel[buttonID] = 0;
            boardAckToStation(buttonID, DRIVER_CANCEL);
        }
    }
}

void boardAckToStation(BUTTON_ID buttonID, SYSTEM_STATE state)
{
    responseToStation[0] = buttonID;
    responseToStation[1] = state;
    responseToStationCRC = CRC16((char *)responseToStation, 2);
    responseToStation[2] = responseToStationCRC & 0xFF;
    responseToStation[3] = responseToStationCRC >> 8;
    responseToStation[4] = '\0';
    rs485_setmode(RS485_TRANSMIT);
    RS485_Serial.write((const uint8_t *)responseToStation, 5);
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