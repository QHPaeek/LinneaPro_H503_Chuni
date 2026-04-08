/*
 * keyboard.c
 *
 *  Created on: Jan 20, 2026
 *      Author: Qinh
 */
#include <string.h>
#include "stdint.h"
#include "usb_device.h"
#include "usbd_hid_keyboard.h"
#include "keyboard.h"
#include "LED.h"

extern USBD_HandleTypeDef hUsbDevice;
HID_Keyboard_Report_t keyboard;
static HID_Keyboard_Report_t last_report = {0};

void keyboard_init(){
	keyboard.report_id = 0x01;
}

void keyboard_refresh(void)
{
    if (memcmp(&keyboard, &last_report, sizeof(HID_Keyboard_Report_t)) == 0){
        return;
    }
    USBD_HID_Keybaord_SendReport(&hUsbDevice,
                        (uint8_t *)&keyboard,
                        sizeof(HID_Keyboard_Report_t));
    memcpy(&last_report,&keyboard, sizeof(HID_Keyboard_Report_t));
}

void keyboard_poll(){
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_13) == 0){
		keyboard.keycode[36] = 0x3A; //F1
		keyboard.keycode[38] = keycode_sheet['1'];
	}else{
		keyboard.keycode[36] = 0;
		keyboard.keycode[38] = 0;
	}
	if(HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_14) == 0){
		uint8_t check = 0;
		for(uint8_t i = 0;i<93;i++){
			check += RGB_data[i];
		}
		if(check){
			keyboard.keycode[37] = 0x3C; //F3
			keyboard.keycode[39] = keycode_sheet['3'];
		}else{
			keyboard.keycode[37] = 0x3B; //F2
			keyboard.keycode[39] = keycode_sheet['2'];
		}
	}else{
		keyboard.keycode[37] = 0;
		keyboard.keycode[39] = 0;
	}
	keyboard_refresh();
}
/* ASCII (0x00–0x7F) → USB HID KeyCode */
const uint8_t keycode_sheet[128] =
{
    /* 0x00–0x07 */
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /* 0x08–0x0F */
    0x2A, /* BS  */
    0x2B, /* TAB */
    0x28, /* LF(Enter) */
    0x00,0x00,0x00,0x00,0x00,

    /* 0x10–0x17 */
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /* 0x18–0x1F */
    0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,

    /* 0x20 ' ' */
    0x2C, /* Space */

    /* 0x21–0x27 */
    0x00, /* ! */
    0x00, /* " */
    0x00, /* # */
    0x00, /* $ */
    0x00, /* % */
    0x00, /* & */
    0x34, /* ' */

    /* 0x28–0x2F */
    0x26, /* ( */
    0x27, /* ) */
    0x25, /* * */
    0x2E, /* + */
    0x36, /* , */
    0x2D, /* - */
    0x37, /* . */
    0x38, /* / */

    /* 0x30–0x39 */
    0x27, /* 0 */
    0x1E, /* 1 */
    0x1F, /* 2 */
    0x20, /* 3 */
    0x21, /* 4 */
    0x22, /* 5 */
    0x23, /* 6 */
    0x24, /* 7 */
    0x25, /* 8 */
    0x26, /* 9 */

    /* 0x3A–0x40 */
    0x33, /* : */
    0x33, /* ; */
    0x36, /* < */
    0x2E, /* = */
    0x37, /* > */
    0x38, /* ? */
    0x1F, /* @ */

    /* 0x41–0x5A 'A'–'Z' */
    0x04, /* A */
    0x05, /* B */
    0x06, /* C */
    0x07, /* D */
    0x08, /* E */
    0x09, /* F */
    0x0A, /* G */
    0x0B, /* H */
    0x0C, /* I */
    0x0D, /* J */
    0x0E, /* K */
    0x0F, /* L */
    0x10, /* M */
    0x11, /* N */
    0x12, /* O */
    0x13, /* P */
    0x14, /* Q */
    0x15, /* R */
    0x16, /* S */
    0x17, /* T */
    0x18, /* U */
    0x19, /* V */
    0x1A, /* W */
    0x1B, /* X */
    0x1C, /* Y */
    0x1D, /* Z */

    /* 0x5B–0x60 */
    0x2F, /* [ */
    0x31, /* \ */
    0x30, /* ] */
    0x23, /* ^ */
    0x2D, /* _ */
    0x35, /* ` */

    /* 0x61–0x7A 'a'–'z' */
    0x04, /* a */
    0x05, /* b */
    0x06, /* c */
    0x07, /* d */
    0x08, /* e */
    0x09, /* f */
    0x0A, /* g */
    0x0B, /* h */
    0x0C, /* i */
    0x0D, /* j */
    0x0E, /* k */
    0x0F, /* l */
    0x10, /* m */
    0x11, /* n */
    0x12, /* o */
    0x13, /* p */
    0x14, /* q */
    0x15, /* r */
    0x16, /* s */
    0x17, /* t */
    0x18, /* u */
    0x19, /* v */
    0x1A, /* w */
    0x1B, /* x */
    0x1C, /* y */
    0x1D, /* z */

    /* 0x7B–0x7F */
    0x2F, /* { */
    0x31, /* | */
    0x30, /* } */
    0x35, /* ~ */
    0x00
};
