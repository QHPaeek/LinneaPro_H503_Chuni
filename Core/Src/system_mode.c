/*
 * system_mode.c
 *
 *  Created on: Jan 26, 2026
 *      Author: Qinh
 */
#include "system_mode.h"
#include "gpio.h"
#include "usb_device.h"

struct mode_t sys_mode;
extern PCD_HandleTypeDef hpcd_USB_DRD_FS;
extern USBD_HandleTypeDef hUsbDevice;

void System_Mode_Poll(){
	uint8_t tmp = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7) && (!HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0));
	if(tmp != sys_mode.Power_Mode){
		if(tmp){
			Ground_LED_show(0,0,0);
			Air_LED_show(0,0,0);
			sys_mode.Power_Mode = POWER_OFF;
		}else{
			sys_mode.Power_Mode = POWER_ON;
			Ground_LED_show(0,0,128);
			Air_LED_show(128,128,128);
//			HAL_PCD_DeInit(&hpcd_USB_DRD_FS);
//			GPIO_InitTypeDef GPIO_InitStruct = {0};
//			GPIO_InitStruct.Pin = GPIO_PIN_12;
//			GPIO_InitStruct.Mode = GPIO_MODE_OUTPUT_OD;
//			HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
		}
	}
}

void System_Mode_Init(){
	sys_mode.Game_Mode = 0;
	GPIO_InitTypeDef GPIO_InitStruct = {0};

	GPIO_InitStruct.Pin = GPIO_PIN_7;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
	GPIO_InitStruct.Pin = GPIO_PIN_0;
	GPIO_InitStruct.Pull = GPIO_PULLDOWN;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	osDelay(1);
	sys_mode.Power_Mode = HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_7) && (!HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_0));
	if(sys_mode.Power_Mode){

	}else{
		Ground_LED_show(0,0,128);
		Air_LED_show(128,128,128);
	}
}
