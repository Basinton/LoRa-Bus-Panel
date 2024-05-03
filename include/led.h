#ifndef __LED_H_
#define __LED_H_

/* Includes ------------------------------------------------------------------*/
#include "main.h"
/* Define --------------------------------------------------------------------*/
#define PIN_ALARM 2 // use this

/* Variables -----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/
void led_init();

void led_off(BUTTON_ID btn);
void led_on(BUTTON_ID btn);
void led_toggle_1s();

#endif