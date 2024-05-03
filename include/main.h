#ifndef __MAIN_H_
#define __MAIN_H_

typedef enum
{
    BUTTON_UNKNOWN = -1,
    BUTTON_0,
    BUTTON_1,
    BUTTON_2,
    BUTTON_3,
    BUTTON_4,
    BUTTON_COUNT
} BUTTON_ID;

extern BUTTON_ID buttonID;

#endif // MAIN