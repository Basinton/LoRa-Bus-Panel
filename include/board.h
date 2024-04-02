#pragma once
#ifndef __BOARD_H_
#define __BOARD_H_

/* Includes ------------------------------------------------------------------*/
#include <stdio.h>

/* Define --------------------------------------------------------------------*/
typedef enum
{
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
extern uint8_t isStationAccept;
extern uint8_t isBusAccept;
extern uint8_t isBusPass;

extern uint8_t isReAckBusAccept;
extern uint8_t isReAckBusPass;
extern uint8_t isReAckBusCancel;
extern uint8_t isReAckPassengerCancel;

extern uint8_t numberGuest;
extern SYSTEM_STATE boardState;

extern uint8_t isBusCancel;
extern uint8_t isPassengerCancelAck;

/* Functions -----------------------------------------------------------------*/
void board_init(void);
void busCancel(void);

#endif