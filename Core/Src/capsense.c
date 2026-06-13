/*
 * capsense.c
 *
 *  Created on: Jan 8, 2025
 *      Author: Qinh
 */
#include "capsense.h"
#include "usart.h"
#include "string.h"
#include "stdbool.h"
#include <math.h>
#include "flash.h"

#define UART_FRAME_LEN 66
#define CAPSENSE_BASELINE_VARIANCE 255
#define CAPSENSE_LOW_BASELINE_DURATION_A 20
#define CAPSENSE_LOW_BASELINE_DURATION_B 400
#define CAPSENSE_BSLN_SENS_FACTOR 0.03

extern UART_HandleTypeDef huart1;
extern UART_HandleTypeDef huart2;
extern UART_HandleTypeDef huart3;
extern UART_HandleTypeDef hlpuart1;

extern DMA_HandleTypeDef handle_GPDMA1_Channel0;
extern DMA_HandleTypeDef handle_GPDMA1_Channel1;
extern DMA_HandleTypeDef handle_GPDMA1_Channel2;
extern DMA_HandleTypeDef handle_GPDMA1_Channel3;

uint8_t uart1_buf[UART_FRAME_LEN];
uint8_t uart2_buf[UART_FRAME_LEN];
uint8_t uart3_buf[UART_FRAME_LEN];
uint8_t lpuart1_buf[UART_FRAME_LEN];

packet_capsense_t Touch;
//uint16_t capsense_raw_windows[10][34];
//uint8_t capsense_raw_bet = 0;
uint16_t capsense_low_baseline_duration[128];
uint16_t capsense_orinigal[128];
uint16_t capsense_baseline[128];
uint16_t capsense_threshold[128];
uint8_t capsense_data_ready = 0;
//uint8_t capsense_bit;
uint8_t capsense_touch_status[128];
uint8_t uart_data_ready[4] = {0,0,0,0};
uint16_t capsense_minimum_baseline[128];
uint8_t capsense_low_bsln_flag[128];
uint16_t capsense_maxmium[128];
uint8_t capsense_sava_flag[128];

extern EEPROM_DATA_t g_eeprom;

bool check_sum(uint8_t* data){
	uint8_t checksum = 0;
	for(uint8_t i = 1;i<65;i++){
		checksum += data[i];
	}
	return checksum == data[65] ? true : false;
}

void HAL_UARTEx_RxEventCallback(UART_HandleTypeDef *huart, uint16_t Size)
{
    if(Size != UART_FRAME_LEN){
    	return;
    }

    uint8_t *src = NULL;
    uint8_t *dest = NULL;

    if(huart->Instance == USART1){
    	src = uart1_buf;
    	dest = Touch.touc_data_a;
    	if(check_sum(src)){
			capsense_data_ready |= 1;
			memcpy(dest, src+1, 64);
    	}
    }
    else if(huart->Instance == USART2){
    	src = uart2_buf;
    	dest = Touch.touc_data_d;
    	if(check_sum(src)){
			capsense_data_ready |= 2;
			memcpy(dest, src+1, 64);
		}
    }
    else if(huart->Instance == USART3){
    	src = uart3_buf;
    	dest = Touch.touc_data_c;
    	if(check_sum(src)){
			capsense_data_ready |= 4;
			memcpy(dest, src+1, 64);
		}
	}
    else if(huart->Instance == LPUART1){
    	src = lpuart1_buf;
    	dest = Touch.touc_data_b;
    	if(check_sum(src)){
			capsense_data_ready |= 8;
			memcpy(dest, src+1, 64);
		}
    }
    else{
    	return;
    }

}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
	HAL_Delay(1);
    HAL_UART_DMAStop(huart);
    __HAL_UART_CLEAR_FLAG(huart, UART_CLEAR_FEF | UART_CLEAR_NEF | UART_CLEAR_OREF | UART_CLEAR_PEF);

    if (huart->Instance == USART1)
    {
        while(HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_buf, UART_FRAME_LEN) != HAL_OK);
    }
    else if(huart->Instance == USART2){
    	while(HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart2_buf, UART_FRAME_LEN) != HAL_OK);
    }
    else if(huart->Instance == USART3){
    	while(HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart3_buf, UART_FRAME_LEN) != HAL_OK);
    }
    else if(huart->Instance == LPUART1){
    	while(HAL_UARTEx_ReceiveToIdle_DMA(&hlpuart1, lpuart1_buf, UART_FRAME_LEN) != HAL_OK);
    }
}

void capsense_init()
{
	for(uint8_t i = 0;i<128;i++){
		capsense_threshold[i] = 600;
		capsense_minimum_baseline[i] = 0xffff;
		capsense_low_bsln_flag[i] = 0;
		capsense_baseline[i] = 0xffff;
		capsense_sava_flag[i] = 0;
		capsense_maxmium[i] = 0;
	}
//	Flash_Load(&Flash);


	while(HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_buf, UART_FRAME_LEN) != HAL_OK);
	while(HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart2_buf, UART_FRAME_LEN) != HAL_OK);
	while(HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart3_buf, UART_FRAME_LEN) != HAL_OK);
	while(HAL_UARTEx_ReceiveToIdle_DMA(&hlpuart1, lpuart1_buf, UART_FRAME_LEN) != HAL_OK);

    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel0, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel1, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel2, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel3, DMA_IT_HT);


    osDelay(100);
    while(capsense_data_ready == 0);
    if(EEPROM_Load() == 1){
    	for(uint8_t i = 0;i<128;i++){
    		capsense_maxmium[i] = g_eeprom.data[i];
    	}
		for(uint8_t j = 0;j<100;j++){
			while(capsense_data_ready != 0xf);
			for(uint8_t i = 0;i<128;i++){
				if(capsense_minimum_baseline[i] > Touch.channel_raw[i]){
					capsense_minimum_baseline[i] = Touch.channel_raw[i];
				}
			}
			capsense_data_ready = 0;
		}
    }else{
		for(uint8_t j = 0;j<100;j++){
			while(capsense_data_ready != 0xf);
			for(uint8_t i = 0;i<128;i++){
				if(capsense_minimum_baseline[i] > Touch.channel_raw[i]){
					capsense_minimum_baseline[i] = Touch.channel_raw[i];
				}
				if(capsense_maxmium[i] < Touch.channel_raw[i]){
					capsense_maxmium[i] = Touch.channel_raw[i];
					capsense_sava_flag[i] = 1;
				}
			}
			capsense_data_ready = 0;
		}
    }
}

void capsense_poll(){
	if(capsense_data_ready != 0xf){
		return;
	}
	for(uint8_t i = 0;i<128;i++){
		uint16_t raw = Touch.channel_raw[i];
		if(capsense_minimum_baseline[i] > raw){
			capsense_minimum_baseline[i] = raw;
		}
		if(capsense_maxmium[i] < raw){
			capsense_maxmium[i] = raw;
			capsense_sava_flag[i] = 1;
		}
		if(capsense_baseline[i] == 0){
			capsense_baseline[i] = raw;
			capsense_low_bsln_flag[i] = 0;
		}else if(raw > capsense_baseline[i] + capsense_threshold[i]){
			capsense_low_bsln_flag[i] = 0;
		}else if(raw > capsense_baseline[i] - 200){
			capsense_low_bsln_flag[i] = 0;
			if(capsense_baseline[i] < raw){
				capsense_baseline[i] = (capsense_baseline[i] * (1-CAPSENSE_BSLN_SENS_FACTOR)) + (raw * CAPSENSE_BSLN_SENS_FACTOR);
				capsense_low_baseline_duration[i] = 0;
			}else if(capsense_low_baseline_duration[i] < CAPSENSE_LOW_BASELINE_DURATION_A){
				capsense_low_baseline_duration[i]++;
			}else if((capsense_low_baseline_duration[i] > CAPSENSE_LOW_BASELINE_DURATION_A) & (capsense_low_baseline_duration[i] < CAPSENSE_LOW_BASELINE_DURATION_B)){
				capsense_baseline[i] = (capsense_baseline[i] * (1-CAPSENSE_BSLN_SENS_FACTOR)) + (raw * CAPSENSE_BSLN_SENS_FACTOR);
				capsense_low_baseline_duration[i]++;
			}else{
				capsense_baseline[i] = (capsense_baseline[i] * (1-CAPSENSE_BSLN_SENS_FACTOR)) + (raw * CAPSENSE_BSLN_SENS_FACTOR);
//				capsense_minimum_baseline[i] = Touch.channel_raw[i];
			}
		}else{
			if(capsense_low_bsln_flag[i] > 2){
				capsense_baseline[i] = (capsense_baseline[i] * 0.5) + (raw * 0.5);
			}else{
				capsense_low_bsln_flag[i]++;
			}
		}
//		if(capsense_baseline[i] < (Touch.channel_raw[i] - capsense_threshold[i])){
//			if(capsense_low_bsln_flag > 2){
//				capsense_baseline[i] = (capsense_baseline[i] * 0.5) + (Touch.channel_raw[i] * 0.5);
//			}
//			capsense_low_bsln_flag[i]++;
//		}
//		else if(capsense_baseline[i] + capsense_threshold[i] > Touch.channel_raw[i]){
//			capsense_low_bsln_flag[i] = 0;
//			if(capsense_baseline[i] < Touch.channel_raw[i]){
//				capsense_baseline[i] = (capsense_baseline[i] * 0.98) + (Touch.channel_raw[i] * 0.02);
//				capsense_low_baseline_duration[i] = 0;
//			}else if(capsense_low_baseline_duration[i] < CAPSENSE_LOW_BASELINE_DURATION_A){
//				capsense_low_baseline_duration[i]++;
//			}else if((capsense_low_baseline_duration[i] > CAPSENSE_LOW_BASELINE_DURATION_A) & (capsense_low_baseline_duration[i] < CAPSENSE_LOW_BASELINE_DURATION_B)){
//				capsense_baseline[i] = (capsense_baseline[i] * 0.98) + (Touch.channel_raw[i] * 0.02);
//				capsense_low_baseline_duration[i]++;
//			}else{
//				capsense_baseline[i] = (capsense_baseline[i] * 0.98) + (Touch.channel_raw[i] * 0.02);
////				capsense_minimum_baseline[i] = Touch.channel_raw[i];
//			}
//		}
//		if(capsense_baseline[i] > 60000)capsense_baseline[i] = 60000;
//		if(capsense_baseline[i] < capsense_minimum_baseline[i])capsense_baseline[i] = capsense_minimum_baseline[i];
		uint16_t judge = (raw > capsense_baseline[i]) ? (raw - capsense_baseline[i]) / ((capsense_maxmium[i] - capsense_minimum_baseline[i] + 500) / 255) : 0;
		capsense_touch_status[i] = judge > 254 ? 254 : judge;
	}
	//Flash_Save(&Flash);
	//capsense_data_ready = 0;
}
