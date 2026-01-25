/*
 * slider.c
 *
 *  Created on: Nov 6, 2025
 *      Author: Qinh
 */

#include "slider.h"
#include "capsense.h"
#include "stdbool.h"

//#define PSOC_DEBUG
//#define AUTO_AIR

#ifdef PSOC_DEBUG
typedef union{
    float raw_data_fl[2];
    uint8_t raw_data_u8[8];
}vofa;

vofa vofa1;
#endif

uint8_t debug_channel = 0;

extern uint8_t capsense_data_ready;
extern uint8_t Air_Trigger_Status;

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
uint8_t slider_transfer_buf[38] = {
		0xFF,0x01,0x21,0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
		,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
		,0x00,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00
		,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0x00 ,0xE0 ,0x00
};
void slider_init(){
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
}

#ifdef PSOC_DEBUG

void slider_poll_debug(){
	if(capsense_data_ready == 0xf){

		vofa1.raw_data_fl[0] = Touch.channel_raw[debug_channel];
		vofa1.raw_data_fl[1] = capsense_baseline[debug_channel];
		uint8_t tmp[12] = {0,0,0,0,0,0,0,0,0,0,0x80,0x7f};
		memcpy(tmp,vofa1.raw_data_u8,8);
		CDC_Transmit(0, tmp,12);
		capsense_data_ready = 0;
	}
}
#endif

bool tap_status[4][32];
void slider_poll_idle(){
	if(capsense_data_ready == 0xf){

//		vofa1.raw_data_fl[0] = Touch.channel_raw[debug_channel];
//		vofa1.raw_data_fl[1] = capsense_baseline[debug_channel];
//		uint8_t tmp[12] = {0,0,0,0,0,0,0,0,0,0,0x80,0x7f};
//		memcpy(tmp,vofa1.raw_data_u8,8);
//		CDC_Transmit(0, tmp,12);
//		capsense_data_ready = 0;
	}
}
