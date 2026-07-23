#include "LED.h"
#include "stdio.h"
#include "stdbool.h"
#include "air_string.h"
#include "system_mode.h"
#include "gpio.h"

#define NUM_LED 47
#define AIR_NUM_LED 16
#define WS2812_HIGH 200
#define WS2812_LOW 90
#define WS2812_TIM_HANDLE &htim2
#define WS2812_TIM_CH TIM_CHANNEL_1
#define AIR_LED_CHANNEL 6

uint8_t RGB_data[3 * NUM_LED];
uint8_t Air_LED_Refresh_Flag = 0;
static uint8_t RGB_data_DMA_buffer[64 + NUM_LED * 24 + 64] = {WS2812_HIGH + WS2812_LOW};
static uint8_t Air_RGB_data[3 * AIR_NUM_LED] = {0xff};
static uint8_t Air_RGB_data_DMA_buffer[AIR_NUM_LED * 24 + 64 + 64] = {WS2812_HIGH + WS2812_LOW};

extern uint8_t rxData[128];
extern uint32_t rxLen;

bool Air_LED_flag = false;
uint8_t LED_Merge_flag = 1;

const uint8_t gamma8[256] = {
  0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   0,   1,   1,   1,   1,   1,
  2,   2,   2,   2,   2,   3,   3,   3,   4,   4,   4,   5,   5,   5,   6,   6,
  6,   7,   7,   7,   8,   8,   9,   9,   9,  10,  10,  11,  11,  12,  12,  13,
 13,  14,  14,  15,  15,  16,  16,  17,  17,  18,  19,  19,  20,  20,  21,  22,
 22,  23,  23,  24,  25,  25,  26,  27,  27,  28,  29,  29,  30,  31,  32,  32,
 33,  34,  35,  35,  36,  37,  38,  38,  39,  40,  41,  42,  42,  43,  44,  45,
 46,  46,  47,  48,  49,  50,  51,  52,  53,  53,  54,  55,  56,  57,  58,  59,
 60,  61,  62,  63,  64,  65,  66,  67,  68,  69,  70,  71,  72,  73,  74,  75,
 76,  77,  78,  79,  80,  81,  82,  83,  84,  85,  87,  88,  89,  90,  91,  92,
 93,  94,  96,  97,  98,  99, 100, 101, 103, 104, 105, 106, 107, 109, 110, 111,
112, 114, 115, 116, 117, 119, 120, 121, 122, 124, 125, 126, 128, 129, 130, 131,
133, 134, 135, 137, 138, 139, 141, 142, 144, 145, 146, 148, 149, 150, 152, 153,
155, 156, 158, 159, 160, 162, 163, 165, 166, 168, 169, 171, 172, 174, 175, 177,
178, 180, 181, 183, 184, 186, 187, 189, 190, 192, 193, 195, 196, 198, 200, 201,
203, 204, 206, 208, 209, 211, 212, 214, 216, 217, 219, 221, 222, 224, 225, 227,
229, 231, 232, 234, 236, 237, 239, 241, 242, 244, 246, 248, 249, 251, 253, 255
};

void HAL_TIM_PWM_PulseFinishedCallback(TIM_HandleTypeDef *htim)
{
	if(htim->Instance == TIM1)
	{
		HAL_TIM_PWM_Stop_DMA(&htim1, TIM_CHANNEL_1);
	}
    if(htim->Instance == TIM2)
    {
        HAL_TIM_PWM_Stop_DMA(&htim2, TIM_CHANNEL_1);
    }
    if(htim->Instance == TIM3)
	{
    	Air_Select_Channel(7);
		HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_1);
		Air_LED_flag = true;
	}
}

void LED_init(){
	GPIO_InitTypeDef GPIO_InitStruct = {0};
	GPIO_InitStruct.Pin = GPIO_PIN_10;
	GPIO_InitStruct.Mode = GPIO_MODE_INPUT;
	GPIO_InitStruct.Pull = GPIO_PULLUP;
	HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);
	osDelay(100);
	LED_Merge_flag = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_10);
	memset(RGB_data_DMA_buffer,0,64 + NUM_LED * 24 + 64);
	memset(Air_RGB_data_DMA_buffer,0,AIR_NUM_LED * 24 + 64 + 64);
	Air_Select_Channel(7);
}

void Ground_LED_set(uint8_t led_idx,uint8_t r,uint8_t g,uint8_t b){
	if(led_idx >= 31){
		return;
	}
	if(LED_Merge_flag){
		RGB_data[led_idx * 3] = r;
		RGB_data[led_idx * 3 + 1] = g;
		RGB_data[led_idx * 3 + 2] = b;
	}else{
	   if (led_idx % 2 == 0){
		   uint8_t _led_idx = led_idx / 2 * 3;
			RGB_data[_led_idx * 3] = r;
			RGB_data[_led_idx * 3 + 1] = g;
			RGB_data[_led_idx * 3 + 2] = b;
			_led_idx++;
			RGB_data[_led_idx * 3] = r;
			RGB_data[_led_idx * 3 + 1] = g;
			RGB_data[_led_idx * 3 + 2] = b;
		}else{
			uint8_t _led_idx = (led_idx + 1) / 2 * 3 - 1;
			RGB_data[_led_idx * 3] = r;
			RGB_data[_led_idx * 3 + 1] = g;
			RGB_data[_led_idx * 3 + 2] = b;
		}
	}

}

void Air_LED_set(uint8_t led_idx,uint8_t r,uint8_t g,uint8_t b){
	if(led_idx >= AIR_NUM_LED){
		return;
	}
	Air_RGB_data[led_idx * 3] = r;
	Air_RGB_data[led_idx * 3 + 1] = g;
	Air_RGB_data[led_idx * 3 + 2] = b;
	Air_LED_Refresh_Flag = 1;
}

void Ground_LED_refresh()
{
	if(sys_mode.Power_Mode == POWER_OFF){
		for(uint16_t i = 0 ;i <NUM_LED * 24;i++)
		{
			RGB_data_DMA_buffer[i + 64] = WS2812_LOW;
		}
		HAL_TIM_PWM_Start_DMA(WS2812_TIM_HANDLE, WS2812_TIM_CH, (uint32_t *)RGB_data_DMA_buffer, NUM_LED * 24 + 64 + 64);
		return;
	}
	for(uint8_t i = 0 ;i<NUM_LED;i++)
	{
		for(uint8_t j = 0 ;j <8;j++)
		{
			RGB_data_DMA_buffer[(i*3)*8+j+64] = (gamma8[RGB_data[i*3+1]] & (1<<(7-j))) ? WS2812_HIGH:WS2812_LOW;
		}
		for(uint8_t j = 0 ;j <8;j++)
		{
			RGB_data_DMA_buffer[(i*3+1)*8+j+64] = (gamma8[RGB_data[i*3]] & (1<<(7-j))) ? WS2812_HIGH:WS2812_LOW;
		}
		for(uint8_t j = 0 ;j <8;j++)
		{
			RGB_data_DMA_buffer[(i*3+2)*8+j+64] = (gamma8[RGB_data[i*3+2]] & (1<<(7-j))) ? WS2812_HIGH:WS2812_LOW;
		}
	}
	HAL_TIM_PWM_Start_DMA(WS2812_TIM_HANDLE, WS2812_TIM_CH, (uint32_t *)RGB_data_DMA_buffer, NUM_LED * 24 + 64 + 64);
}

void Ground_LED_Poll(){
	Ground_LED_refresh();
//	if(rxLen != 0){
//		Serial_Receive_Handle(rxData, rxLen);
//	}
//	rxLen = 0;
}

void TIM3_Set_800K(void)
{
    //配置 38kHz 参数
    TIM3->PSC = 0;
    TIM3->ARR = 312;
    TIM3->CCR1 = 0;

    //更新寄存器
    TIM3->EGR = TIM_EGR_UG;
    TIM3->CR1 |= TIM_CR1_ARPE;
}



void Air_LED_refresh()
{
	if(!Air_LED_Refresh_Flag)return;
	TIM3_Set_800K();
//	Air_RGB_data_DMA_buffer[63] = 10;
	Air_Select_Channel(AIR_LED_CHANNEL);
	osDelay(1);
	Air_LED_flag = false;
	if(sys_mode.Power_Mode == POWER_OFF){
		for(uint16_t i = 0 ;i< AIR_NUM_LED * 24;i++)
		{
			Air_RGB_data_DMA_buffer[i + 64] = WS2812_LOW;
		}
		HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)Air_RGB_data_DMA_buffer, AIR_NUM_LED * 24 + 64 + 64);
		HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)Air_RGB_data_DMA_buffer, AIR_NUM_LED * 24 + 64 + 64);
		Air_Select_Channel(AIR_LED_CHANNEL);
		return;
	}
	for(uint8_t i = 0 ;i<AIR_NUM_LED;i++)
	{
		for(uint8_t j = 0 ;j <8;j++)
		{
			Air_RGB_data_DMA_buffer[(i*3)*8+j+64] = (gamma8[Air_RGB_data[i*3+1]] & (1<<(7-j))) ? WS2812_HIGH:WS2812_LOW;
		}
		for(uint8_t j = 0 ;j <8;j++)
		{
			Air_RGB_data_DMA_buffer[(i*3+1)*8+j+64] = (gamma8[Air_RGB_data[i*3]] & (1<<(7-j))) ? WS2812_HIGH:WS2812_LOW;
		}
		for(uint8_t j = 0 ;j <8;j++)
		{
			Air_RGB_data_DMA_buffer[(i*3+2)*8+j+64] = (gamma8[Air_RGB_data[i*3+2]] & (1<<(7-j))) ? WS2812_HIGH:WS2812_LOW;
		}
	}
	HAL_TIM_PWM_Start_DMA(&htim3, TIM_CHANNEL_1, (uint32_t *)Air_RGB_data_DMA_buffer, AIR_NUM_LED * 24 + 64 + 64);
	HAL_TIM_PWM_Start_DMA(&htim1, TIM_CHANNEL_1, (uint32_t *)Air_RGB_data_DMA_buffer, AIR_NUM_LED * 24 + 64 + 64);
}

void Air_LED_show(uint8_t r,uint8_t g,uint8_t b){
	for(uint8_t i = 0;i<AIR_NUM_LED;i++){
		Air_LED_set(i,gamma8[r],gamma8[g],gamma8[b]);
	}
//	Air_LED_refresh();
}

void Ground_LED_show(uint8_t r,uint8_t g,uint8_t b){
	for(uint8_t i = 0;i<NUM_LED;i++){
		Ground_LED_set(i,gamma8[r],gamma8[g],gamma8[b]);
	}
	Ground_LED_refresh();
}

