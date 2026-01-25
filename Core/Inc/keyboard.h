/*
 * keyboard.h
 *
 *  Created on: Jan 20, 2026
 *      Author: Qinh
 */

#ifndef INC_KEYBOARD_H_
#define INC_KEYBOARD_H_
#include "stdint.h"

#define F1 0x3A
#define F2 0x3B
#define F3 0x3C
#define F4 0x3D
#define F5 0x3E
#define F6 0x3F

typedef struct __attribute__((packed))
{
    uint8_t report_id;
    uint8_t keycode[40];
} HID_Keyboard_Report_t;

extern HID_Keyboard_Report_t keyboard;

extern const uint8_t keycode_sheet[128];

void keyboard_init();
void keyboard_refresh(void);

#endif /* INC_KEYBOARD_H_ */
