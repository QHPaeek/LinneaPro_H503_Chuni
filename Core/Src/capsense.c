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

#define UART_FRAME_LEN 66
#define CAPSENSE_BASELINE_VARIANCE 255

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
packet_capsense_t Touch_2;
packet_capsense_t Touch_3;
packet_capsense_t Touch_4;
packet_capsense_t Touch_5;
//uint16_t capsense_raw_windows[10][34];
//uint8_t capsense_raw_bet = 0;
//uint16_t capsense_duration[32] = {0};
uint16_t capsense_orinigal[128];
uint16_t capsense_baseline[128];
uint16_t capsense_threshold[128];
uint8_t capsense_data_ready = 0;
//uint8_t capsense_bit;
uint8_t capsense_touch_status[128];
uint8_t uart_data_ready[4] = {0,0,0,0};

bool check_sum(uint8_t* data){
	uint8_t checksum = 0;
	for(uint8_t i = 1;i<65;i++){
		checksum += data[i];
	}
	return checksum == data[65] ? true : false;
}

static inline uint16_t median(uint16_t a, uint16_t b, uint16_t c)
{
    uint16_t t;

    if (a > b) { t = a; a = b; b = t; }
    if (b > c) { t = b; b = c; c = t; }
    if (a > b) { t = a; a = b; b = t; }

    return b;
}

static inline uint16_t closest3(uint16_t a, uint16_t b, uint16_t c, uint16_t target)
{
    uint32_t da = (a > target) ? (a - target) : (target - a);
    uint32_t db = (b > target) ? (b - target) : (target - b);
    uint32_t dc = (c > target) ? (c - target) : (target - c);

    uint16_t best = a;
    uint32_t dmin = da;

    if (db < dmin) { dmin = db; best = b; }
    if (dc < dmin) { best = c; }

    return best;
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
//    		uart_data_ready[0] ++;
//    		if(uart_data_ready[0] == 1){
//    			memcpy(Touch_2.touc_data_a, src+1, 64);
//    		}else if(uart_data_ready[0] == 2){
//    			memcpy(Touch_3.touc_data_a, src+1, 64);
//    		}else{
//    			uart_data_ready[0] = 0;
    			capsense_data_ready |= 1;
    			memcpy(dest, src+1, 64);
//    			for(uint8_t i = 0;i<32;i++){
//    				Touch.channel_raw[i] = closest3(Touch.channel_raw[i],Touch_2.channel_raw[i],Touch_3.channel_raw[i],capsense_baseline[i]);
//    			}
//    		}
    	}
    }
    else if(huart->Instance == USART2){
    	src = uart2_buf;
    	dest = Touch.touc_data_d;
    	if(check_sum(src)){
//    		uart_data_ready[1] ++;
//    		if(uart_data_ready[1] == 1){
//    			memcpy(Touch_2.touc_data_d, src+1, 64);
//    		}else if(uart_data_ready[1] == 2){
//    			memcpy(Touch_3.touc_data_d, src+1, 64);
//    		}else{
//    			uart_data_ready[1] = 0;
    			capsense_data_ready |= 2;
    			memcpy(dest, src+1, 64);
//    			for(uint8_t i = 0;i<32;i++){
//    				Touch.channel_raw[i+32] = closest3(Touch.channel_raw[i+32],Touch_2.channel_raw[i+32],Touch_3.channel_raw[i+32],capsense_baseline[i+32]);
//    			}
//    		}
		}
    }
    else if(huart->Instance == USART3){
    	src = uart3_buf;
    	dest = Touch.touc_data_c;
    	if(check_sum(src)){
//    		uart_data_ready[2] ++;
//    		if(uart_data_ready[2] == 1){
//    			memcpy(Touch_2.touc_data_c, src+1, 64);
//    		}else if(uart_data_ready[2] == 2){
//    			memcpy(Touch_3.touc_data_c, src+1, 64);
//    		}else{
//    			uart_data_ready[2] = 0;
    			capsense_data_ready |= 4;
    			memcpy(dest, src+1, 64);
//    			for(uint8_t i = 0;i<32;i++){
//    				Touch.channel_raw[i+64] = closest3(Touch.channel_raw[i+64],Touch_2.channel_raw[i+64],Touch_3.channel_raw[i+64],capsense_baseline[i+64]);
//    			}
//    		}
		}
	}
    else if(huart->Instance == LPUART1){
    	src = lpuart1_buf;
    	dest = Touch.touc_data_b;
    	if(check_sum(src)){
//    		uart_data_ready[3] ++;
//    		if(uart_data_ready[3] == 1){
//    			memcpy(Touch_2.touc_data_b, src+1, 64);
//    		}else if(uart_data_ready[3] == 2){
//    			memcpy(Touch_3.touc_data_b, src+1, 64);
//    		}else{
//    			uart_data_ready[3] = 0;
    			capsense_data_ready |= 8;
    			memcpy(dest, src+1, 64);
//    			for(uint8_t i = 0;i<32;i++){
//    				Touch.channel_raw[i+96] = closest3(Touch.channel_raw[i+96],Touch_2.channel_raw[i+96],Touch_3.channel_raw[i+96],capsense_baseline[i+96]);
//    			}
//    		}
		}
    }
    else{
    	return;
    }

}

void capsense_init()
{
	for(uint8_t i = 0;i<128;i++){
		capsense_threshold[i] = 500;
	}
//	memset(Touch.touc_data_a,0,64);
//	memset(Touch.touc_data_b,0,64);
//	memset(Touch.touc_data_c,0,64);
//	memset(Touch.touc_data_d,0,64);
//
//	memset(Touch_tmp.touc_data_a,0xff,64);
//	memset(Touch_tmp.touc_data_b,0xff,64);
//	memset(Touch_tmp.touc_data_c,0xff,64);
//	memset(Touch_tmp.touc_data_d,0xff,64);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_buf, UART_FRAME_LEN);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart2_buf, UART_FRAME_LEN);
    HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart3_buf, UART_FRAME_LEN);
    HAL_UARTEx_ReceiveToIdle_DMA(&hlpuart1, lpuart1_buf, UART_FRAME_LEN);

    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel0, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel1, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel2, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel3, DMA_IT_HT);

}

void capsense_poll(){
	if(capsense_data_ready != 0xf){
		return;
	}
	for(uint8_t i = 0;i<128;i++){
		if(capsense_baseline[i] == 0){
			capsense_baseline[i] = Touch.channel_raw[i];
		}
//		else if(capsense_baseline[i] > Touch.channel_raw[i]){
//			capsense_baseline[i] = Touch.channel_raw[i];
//		}
		if(capsense_baseline[i] + capsense_threshold[i] > Touch.channel_raw[i]){
			capsense_baseline[i] = (capsense_baseline[i] * 0.99) + (Touch.channel_raw[i] * 0.01);
//			if(baseline + CAPSENSE_BASELINE_VARIANCE < Touch.channel_raw[i]){
//				capsense_baseline[i] = baseline;
//			}
		}
		uint16_t judge = (Touch.channel_raw[i] > capsense_baseline[i]) ? (Touch.channel_raw[i] - capsense_baseline[i]) / 10 : 0;
		capsense_touch_status[i] = judge > 255 ? 255 : judge;
		if(capsense_touch_status[i] > 0XFE){
			capsense_touch_status[i] = 0xfe;
		}
	}
	//capsense_data_ready = 0;
}
