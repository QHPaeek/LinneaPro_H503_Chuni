#ifndef _LED_H
#define _LED_H

#include "stdio.h"
#include "gpdma.h"
#include "tim.h"
#include "stdbool.h"

extern uint8_t RGB_data[];
extern bool Air_LED_flag;

void LED_set(uint8_t led_no,uint8_t r,uint8_t g,uint8_t b);
void LED_refresh();
#endif
