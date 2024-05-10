#ifndef __BOARD_H_
#define __BOARD_H_

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>
#include "main.h"
/* Define --------------------------------------------------------------------*/
typedef enum
{
    ERROR_TIMEOUT                            = 50,
    INIT                                     = 0,
    WAITING                                  = 1,
    REQUEST_TO_STATION                       = 2,
    STATION_NOTIFY_ACCEPT_TO_BOARD           = 3,
    REQUEST_TO_BUS                           = 4,
    STATION_NOTIFY_BUS_ACCEPT_TO_BOARD       = 5,
    BUS_ACCEPT                               = 6,
    STATION_NOTIFY_BUS_PASS_TO_BOARD         = 7,
    BUS_PASS                                 = 8,
    STATION_NOTIFY_DRIVER_CANCEL_TO_BOARD    = 9,
    DRIVER_CANCEL                            = 10,
    BOARD_NOTIFY_PASSENGER_CANCEL_TO_STATION = 11,
    PASSENGER_CANCEL                         = 12,
    FINISHED                                 = 13
} SYSTEM_STATE;

/* Variables -----------------------------------------------------------------*/
extern uint8_t isStationAccept[BUTTON_COUNT];
extern uint8_t isBusAccept[BUTTON_COUNT];
extern uint8_t isBusPass[BUTTON_COUNT];

extern uint8_t isReAckBusAccept[BUTTON_COUNT];
extern uint8_t isReAckBusPass[BUTTON_COUNT];
extern uint8_t isReAckBusCancel[BUTTON_COUNT];
extern uint8_t isReAckPassengerCancel[BUTTON_COUNT];

extern uint8_t numberGuest[];
extern SYSTEM_STATE boardState[BUTTON_COUNT];

extern uint8_t isBusCancel[BUTTON_COUNT];
extern uint8_t isPassengerCancelAck[BUTTON_COUNT];

extern uint8_t isBusError[BUTTON_COUNT];

/* Functions -----------------------------------------------------------------*/
void board_init(void);
void busCancel(void);

#endif