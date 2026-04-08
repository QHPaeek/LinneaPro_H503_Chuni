/*
 * serial_protocol.c
 *
 *  Created on: Nov 29, 2025
 *      Author: Qinh
 */
#include "serial_protocol.h"
#include "system_mode.h"

typedef enum slider_cmd {
	SERIAL_CMD_NOP = 0,
	SERIAL_CMD_AUTO_SCAN = 0x01,
	SERIAL_CMD_SET_LED = 0x02,
	SERIAL_CMD_AUTO_SCAN_START = 0x03,
	SERIAL_CMD_AUTO_SCAN_STOP = 0x04,
	SERIAL_CMD_AUTO_AIR = 0x05,
	SERIAL_CMD_AUTO_AIR_START = 0x06,
	SERIAL_CMD_SET_AIR_LED = 0x07,
	SERIAL_CMD_RESET = 0x10,
} slider_cmd_t;

typedef union serial_packet {
	struct {
		uint8_t syn;
		uint8_t cmd;
		uint8_t size;
		union {
			struct {
				uint8_t led_unk;
				uint8_t leds[96];
			};
			char version[32];
			struct{
				uint8_t pressure[32];
				uint8_t air_status;
			};
			uint8_t air_leds[3];
			uint8_t _air_status;
		};
	};
	uint8_t data[128];
} serial_packet_t;

serial_packet_t serial_packet;

void Serial_Receive_Handle(uint8_t* data,uint8_t len){
	memcpy(serial_packet.data,data,len);
	if(serial_packet.syn != 0xff)return;
	switch(serial_packet.cmd){
		case SERIAL_CMD_SET_LED:{
			sys_mode.Game_Mode = 1;
			for(uint8_t i = 0;i<31;i++){
				Ground_LED_set(i,serial_packet.leds[i*3+1],serial_packet.leds[i*3+2],serial_packet.leds[i*3]);
			}
			Ground_LED_refresh();
			break;
		}
		case SERIAL_CMD_SET_AIR_LED:{
			sys_mode.Game_Mode = 1;
			Air_LED_show(serial_packet.air_leds[1],serial_packet.air_leds[2],serial_packet.air_leds[0]);
			break;
		}
		default:
			break;
	}
	return;
}


