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
uint8_t uart_data_ready = 0;
uint8_t capsense_data_ready = 0;
//uint8_t capsense_bit;
uint8_t capsense_touch_status[128];
uint16_t capsense_minimum[128];
uint8_t capsense_low_bsln_flag[128];
uint16_t capsense_maxmium[128];
uint8_t capsense_sava_flag[128];
//uint16_t capsense_raw_history[3][128];
//uint16_t capsense_flitered_history[3][128];
uint16_t capsense_history[3][128];
uint8_t capsense_history_header = 0;
uint8_t capsense_history_body = 0;
uint8_t capsense_history_tail = 0;
uint8_t capsense_history_filled_flag = 0;
uint8_t capsense_image[4][32];
uint8_t capsense_image_index[4][32];
uint8_t capsense_tap_flag[128];
uint8_t capsense_blind_flag[128];

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
    		uart_data_ready |= 1;
			memcpy(dest, src+1, 64);
    	}
    }
    else if(huart->Instance == USART2){
    	src = uart2_buf;
    	dest = Touch.touc_data_d;
    	if(check_sum(src)){
    		uart_data_ready |= 2;
			memcpy(dest, src+1, 64);
		}
    }
    else if(huart->Instance == USART3){
    	src = uart3_buf;
    	dest = Touch.touc_data_c;
    	if(check_sum(src)){
    		uart_data_ready |= 4;
			memcpy(dest, src+1, 64);
		}
	}
    else if(huart->Instance == LPUART1){
    	src = lpuart1_buf;
    	dest = Touch.touc_data_b;
    	if(check_sum(src)){
    		uart_data_ready |= 8;
			memcpy(dest, src+1, 64);
		}
    }
    else{
    	return;
    }
    if(uart_data_ready == 0xf){
//    	capsense_history_operate();
    	uart_data_ready = 0;
    	capsense_data_ready = 0xf;
    }
}

/************************************************
 * 历史周期数值计算
 * 如果中间数值比头和尾都大/小（尖刺形状），则将其赋值为头和尾的平均数
 ************************************************/

//void capsense_history_operate(){
////    	for(uint8_t i =0;i<128;i++){
////    		capsense_history[capsense_history_header][i] = Touch.channel_raw[i];
////    	}
//	memcpy(capsense_history[capsense_history_header],,256);
//	if(capsense_history_filled_flag != 2){
//		capsense_history_filled_flag ++;
//		return;
//	}
//	capsense_history_body = (capsense_history_header == 0) ? 2 : capsense_history_header - 1;
//	capsense_history_tail = (capsense_history_body == 0) ? 2 : capsense_history_body - 1;
//	for(uint8_t i = 0;i<128;i++){
//		if(capsense_history[capsense_history_header][i] < capsense_history[capsense_history_body][i]){
//			if(capsense_history[capsense_history_tail][i] < capsense_history[capsense_history_body][i]){
//				capsense_history[capsense_history_body][i] = (capsense_history[capsense_history_header][i] + capsense_history[capsense_history_tail][i]) / 2;
//			}
//		}
//		if(capsense_history[capsense_history_header][i] > capsense_history[capsense_history_body][i]){
//			if(capsense_history[capsense_history_tail][i] > capsense_history[capsense_history_body][i]){
//				capsense_history[capsense_history_body][i] = (capsense_history[capsense_history_header][i] + capsense_history[capsense_history_tail][i]) / 2;
//			}
//		}
//	}
//	capsense_history_header = (capsense_history_header + 1) % 3;
//}

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
	while(HAL_UARTEx_ReceiveToIdle_DMA(&huart1, uart1_buf, UART_FRAME_LEN) != HAL_OK);
	while(HAL_UARTEx_ReceiveToIdle_DMA(&huart2, uart2_buf, UART_FRAME_LEN) != HAL_OK);
	while(HAL_UARTEx_ReceiveToIdle_DMA(&huart3, uart3_buf, UART_FRAME_LEN) != HAL_OK);
	while(HAL_UARTEx_ReceiveToIdle_DMA(&hlpuart1, lpuart1_buf, UART_FRAME_LEN) != HAL_OK);

    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel0, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel1, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel2, DMA_IT_HT);
    __HAL_DMA_DISABLE_IT(&handle_GPDMA1_Channel3, DMA_IT_HT);

	memset(capsense_image,0,128);
	for(uint8_t i = 0;i<8;i++){
		for(uint8_t j=0;j<4;j++){
			capsense_image_index[j][i] = 32*3 + i*4 +j;
			capsense_image_index[j][i+8] = 32*2 + i*4 +j;
			capsense_image_index[j][i+16] = 32 + i*4 +j;
			capsense_image_index[j][i+24] = i*4 +j;
		}
	}

	for(uint8_t i = 0;i<128;i++){
		capsense_threshold[i] = 400;
		capsense_minimum[i] = 0xffff;
		capsense_low_bsln_flag[i] = 0;
		capsense_baseline[i] = 0xffff;
		capsense_sava_flag[i] = 0;
		capsense_maxmium[i] = 0;
		capsense_history[0][i] = 0;
		capsense_history[1][i] = 0;
		capsense_history[2][i] = 0;
		capsense_tap_flag[i] = 0;
		capsense_blind_flag[i] = 0;
	}

    for(uint8_t i=0;i<10;i++){
    	while(capsense_data_ready != 0xf){
			osDelay(1);
		}
    	capsense_data_ready = 0;
    }

    if(EEPROM_Load() == 1){
    	for(uint8_t i = 0;i<128;i++){
    		capsense_maxmium[i] = g_eeprom.data[i];
    	}
		for(uint8_t j = 0;j<100;j++){
			while(capsense_data_ready != 0xf);
			for(uint8_t i = 0;i<128;i++){
//				uint16_t raw = capsense_history[capsense_history_tail][i];
				uint16_t raw = Touch.channel_raw[i];
				if(capsense_minimum[i] > raw){
					capsense_minimum[i] = raw;
				}
			}
			capsense_data_ready = 0;
		}
    }else{
		for(uint8_t j = 0;j<100;j++){
			while(capsense_data_ready != 0xf);
			for(uint8_t i = 0;i<128;i++){
//				uint16_t raw = capsense_history[capsense_history_tail][i];
				uint16_t raw = Touch.channel_raw[i];
				if(capsense_minimum[i] > raw){
					capsense_minimum[i] = raw;
				}
				if(capsense_maxmium[i] < raw){
					capsense_maxmium[i] = raw;
					capsense_sava_flag[i] = 1;
				}
			}
			capsense_data_ready = 0;
		}
    }
}

/************************************************
 * 触摸数值作为图像触摸
 * 检测到某点一定被按下，则停止更新它周围一圈传感器的基线 。这有助于改善单指按下在多个传感器之间滑动的情况。
 * 单指按下时，目标邻近的传感器区块会有小幅缓慢的数值提升，若滑动过去有可能因为基线同步提升而导致不触发。
 ************************************************/

void capsense_image_operate(){
	for(uint8_t i = 0;i<32;i++){
		for(uint8_t j = 0;j<4;j++){
			uint8_t cur = capsense_image_index[j][i];
//			float numerator = (float)(capsense_history[capsense_history_tail][cur] - capsense_minimum[cur]);
			float numerator = (float)(Touch.channel_raw[cur] - capsense_minimum[cur]);
			float denominator = (float)(capsense_maxmium[cur] - capsense_minimum[cur]);
			capsense_image[j][i] = (uint8_t)((numerator / denominator) * 255.0f);
		}
	}
	memset(capsense_blind_flag, 0, 128);
	for(uint8_t y = 0; y < 3; y++){
	    for(uint8_t x = 0; x < 31; x++){
	        bool valid = true;
	        /* 检查2×2内部 */
	        for(uint8_t dy = 0; dy < 2 && valid; dy++){
	            for(uint8_t dx = 0; dx < 2; dx++){
	                uint8_t v = capsense_image[y + dy][x + dx];
	                if(v <= 20 || v >= 40){
	                    valid = false;
	                    break;
	                }
	            }
	        }
	        if(!valid){
	            continue;
	        }
	        /* 检查外围一圈 */
	        for(int8_t dy = -1; dy <= 2 && valid; dy++){
	            for(int8_t dx = -1; dx <= 2; dx++){
	                /* 跳过中间2×2 */
	                if(dx >= 0 && dx <= 1 && dy >= 0 && dy <= 1){
	                    continue;
	                }
	                int16_t yy = y + dy;
	                int16_t xx = x + dx;
	                /* 越界忽略 */
	                if(yy < 0 || yy >= 4 || xx < 0 || xx >= 32){
	                    continue;
	                }
	                if(capsense_image[yy][xx] >= 10){
	                    valid = false;
	                    break;
	                }
	            }
	        }
	        if(valid){
	            /* 左下角 */
	            uint8_t idx = capsense_image_index[y + 1][x];
	            capsense_blind_flag[idx] = 1;
	            capsense_tap_flag[idx] = 1;
	        }
	    }
	}
	memset(capsense_tap_flag,0,128);
	for(uint8_t i = 0; i < 32; i++){
		for(uint8_t j = 0; j < 4; j++){
			uint8_t cur = capsense_image_index[j][i];
			if((capsense_image[j][i] <= 40) && (capsense_blind_flag[cur] == 0)){
				continue;
			}

			/* 遍历8邻域 */
			for(int8_t dy = -1; dy <= 1; dy++){
				for(int8_t dx = -1; dx <= 1; dx++){
				/* 跳过自身 */
					if(dx == 0 && dy == 0){
						continue;
					}

					int16_t ny = j + dy;
					int16_t nx = i + dx;

					/* 边界检查 */
					if(ny < 0 || ny >= 4 || nx < 0 || nx >= 32){
						continue;
					}

					uint8_t idx = capsense_image_index[ny][nx];
					capsense_tap_flag[idx] = (capsense_image[ny][nx] < 40);
				}
			}
		}
	}
}
void capsense_poll(){
	if(capsense_data_ready != 0xf){
		return;
	}
	for(uint8_t i = 0;i<128;i++){
//		uint16_t raw = capsense_history[capsense_history_tail][i];
		uint16_t raw = Touch.channel_raw[i];
		if(capsense_minimum[i] > raw){
			capsense_minimum[i] = raw;
		}
		if(capsense_maxmium[i] < raw){
			capsense_maxmium[i] = raw;
			capsense_sava_flag[i] = 1;
		}
		float threshold = (capsense_maxmium[i] - capsense_minimum[i]) * 0.1f;
		capsense_threshold[i] = threshold;
	}
	capsense_image_operate();
	for(uint8_t i = 0;i<128;i++){
//		uint16_t raw = capsense_history[capsense_history_tail][i];
		uint16_t raw = Touch.channel_raw[i];
		if(capsense_blind_flag[i]){
			raw += 2000;
		}
		if(capsense_baseline[i] == 0){
			capsense_baseline[i] = raw;
			capsense_low_bsln_flag[i] = 0;
		}else if(raw > capsense_baseline[i] + capsense_threshold[i]){
			capsense_low_bsln_flag[i] = 0;
		}else if(raw < capsense_baseline[i] - 200){
			if(capsense_low_bsln_flag[i] > 2){
				capsense_baseline[i] = (capsense_baseline[i] * 0.5) + (raw * 0.5);
			}else{
				capsense_low_bsln_flag[i]++;
			}
		}else if(capsense_tap_flag[i]){

		}else{
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
//				capsense_minimum[i] = Touch.channel_raw[i];
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
////				capsense_minimum[i] = Touch.channel_raw[i];
//			}
//		}
//		if(capsense_baseline[i] > 60000)capsense_baseline[i] = 60000;
//		if(capsense_baseline[i] < capsense_minimum[i])capsense_baseline[i] = capsense_minimum[i];
		float judge =(raw > capsense_baseline[i]) ? ((float)(raw - capsense_baseline[i])) / (((float)(capsense_maxmium[i] - capsense_minimum[i])) / 255.0f) : 0.0f;
		capsense_touch_status[i] = judge > 254 ? 254 : judge;
	}
	//Flash_Save(&Flash);
//	capsense_data_ready = 0;
}
