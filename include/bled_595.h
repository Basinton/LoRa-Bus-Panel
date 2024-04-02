#pragma once
#ifndef __BLED_595_H_
#define __BLED_595_H_

/* Includes ------------------------------------------------------------------*/
#include <stdint.h>
#include "Arduino.h"

/* Define --------------------------------------------------------------------*/

#define bled_clock 4
#define bled_latch 14
#define bled_data 13

/* Variables -----------------------------------------------------------------*/

/* Functions -----------------------------------------------------------------*/

void bled_init(void);
void bled_write(int index,int value);

#endif