/*
 * slider.c
 *
 *  Created on: Nov 6, 2025
 *      Author: Qinh
 */

#include "slider.h"
#include "capsense.h"
#include "stdbool.h"
#include "gpio.h"
#include "system_mode.h"

//#define AUTO_AIR

typedef union{
    float raw_data_fl[3];
    uint8_t raw_data_u8[12];
}vofa;

vofa vofa1;

uint8_t debug_channel = 0;

extern uint8_t capsense_data_ready;
extern uint8_t Air_Trigger_Status;
extern uint16_t capsense_maxmium[128];

uint8_t slider_status[32];
uint8_t touch_sheet[8][4] = {
		{28,29,24,25},
		{30,31,26,27},
		{20,21,16,17},
		{22,23,18,19},
		{12,13,8,9},
		{14,15,10,11},
		{4,5,0,1},
		{6,7,2,3},
};

uint8_t touch_sheet_B[8][4] = {
		{11,10,15,14},
		{9,8,13,12},
		{19,18,23,22},
		{17,16,21,20},
		{27,26,31,30},
		{25,24,29,28},
		{3,2,7,6},
		{1,0,5,4},
};


uint8_t slider_transfer_buf[38] = {
		0xFF,0x01,0x21,0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
		,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
		,0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
		,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xE0 ,0x00
};
void slider_init(){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_13;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

//	if(HAL_GPIO_ReadPin(GPIOA,GPIO_PIN_13)){
//	}else{
		memcpy(touch_sheet,touch_sheet_B,32);
//	}
	for(uint8_t i = 0;i<32;i++){
		slider_status[i] = 0;
	}
}

void slider_poll(){
	if(capsense_data_ready == 0xf){
		for(uint8_t i = 0;i<4;i++){ // senser
			for(uint8_t j = 0;j<8;j++){ //button
				slider_transfer_buf[3+i*8+j] = 0;
				for(uint8_t k = 0;k<4;k++){ //tap
					if(capsense_touch_status[touch_sheet[j][k] + i*32] > slider_transfer_buf[3+i*8+j]){
						slider_transfer_buf[3+i*8+j] = capsense_touch_status[touch_sheet[j][k] + i*32];
					}
				}
			}
		}
		if(sys_mode.Game_Mode == 1){
	#ifdef AUTO_AIR
			slider_transfer_buf[35] ++;
			if(slider_transfer_buf[35] > 0b111111){
				slider_transfer_buf[35] = 1;
			}
	#else
			slider_transfer_buf[35] = Air_Trigger_Status;
			for(uint8_t i = 0;i<32;i++){
				if(slider_transfer_buf[3+i] <= 39){
					slider_transfer_buf[3+i] = 0;
				}
			}
	#endif
			CDC_Transmit(0, slider_transfer_buf,38);
			capsense_data_ready = 0;
		}
		else{
			for(uint8_t i = 0;i<16;i++){
				if((slider_transfer_buf[2*i+3] > 60) || (slider_transfer_buf[2*i+4] > 60)){
					Ground_LED_set(2*i,255,0,128);
				}else{
					Ground_LED_set(2*i,0,0,128);
				}
			}
//			Ground_LED_refresh();
		}
	}
}

void slider_poll_debug(){
	if(capsense_data_ready == 0xf){
		vofa1.raw_data_fl[0] = Touch.channel_raw[debug_channel];
		vofa1.raw_data_fl[1] = capsense_baseline[debug_channel];
		vofa1.raw_data_fl[2] = capsense_maxmium[debug_channel];
		uint8_t tmp[16] = {0,0,0,0,0,0,0,0,0,0,0,0,0,0,0x80,0x7f};
		memcpy(tmp,vofa1.raw_data_u8,12);
		CDC_Transmit(0, tmp,16);
		capsense_data_ready = 0;
	}
}

bool tap_status[4][32];
void slider_poll_idle(){
	if(capsense_data_ready == 0xf){
		for(uint8_t i = 0;i<4;i++){ // senser
			for(uint8_t j = 0;j<8;j++){ //button
				slider_transfer_buf[3+i*8+j] = 0;
				for(uint8_t k = 0;k<4;k++){ //tap
					if(capsense_touch_status[touch_sheet[j][k] + i*32] > slider_transfer_buf[3+i*8+j]){
						slider_transfer_buf[3+i*8+j] = capsense_touch_status[touch_sheet[j][k] + i*32];
					}
				}
			}
		}
		for(uint8_t i = 0;i<16;i++){
			if((slider_transfer_buf[2*i+3] > 100) || (slider_transfer_buf[2*i+4] > 100)){
				Ground_LED_set(2*i,255,0,128);
			}else{
				Ground_LED_set(2*i,0,0,128);
			}
		}
	}
}
