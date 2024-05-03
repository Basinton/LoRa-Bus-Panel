#ifndef __BUTTON_165_H_
#define __BUTTON_165_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "Arduino.h"

/* Define --------------------------------------------------------------------*/
#define BUTTON_N 8

#define b_clock  27
#define B_DATA   33
#define b_pload  12

/* Variables -----------------------------------------------------------------*/
extern uint32_t key_code[BUTTON_N];
extern uint32_t press_count[BUTTON_N];
extern uint32_t long_press[BUTTON_N];

/* Functions -----------------------------------------------------------------*/
void b165_init(void);
uint8_t b165_read(int index);

#endif