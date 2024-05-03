#ifndef __LED7_H_
#define __LED7_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Define --------------------------------------------------------------------*/
#define NO_OF_LED7 8

#define led7_clock 23
#define led7_latch 19
#define led7_data  21
#define led7_en    18

/* Variables -----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

void led7_init(void);
void led7_write(int index, int value);
void led7_write_pro(BUTTON_ID busstonID, uint8_t guestNumber);

#endif