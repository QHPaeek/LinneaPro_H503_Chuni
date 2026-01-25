/*
 * air_string.c
 *
 *  Created on: Nov 9, 2025
 *      Author: Qinh
 */
#include "stdint.h"
#include "gpio.h"
#include "tim.h"

uint8_t Air_Expect = 0;
uint8_t Air_Trigger_tmp = 0b00111111;
volatile uint8_t Air_Trigger_Status;

//void Air_Set_38kHz(void)
//{
//    TIM3->PSC = 1;
//    TIM3->ARR = 3289;
//    TIM3->CCR1 = 823;  // 25%
//    TIM3->EGR = TIM_EGR_UG;
//}

void Air_Set_38kHz(void)
{
    //完全停止 TIM3
    HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
    HAL_TIM_PWM_Stop_DMA(&htim3, TIM_CHANNEL_1);
    __HAL_TIM_DISABLE(&htim3);

    //关闭相关中断
    __HAL_DMA_DISABLE_IT(htim3.hdma[TIM_DMA_ID_CC1], DMA_IT_HT | DMA_IT_TC);

    //配置 PWM 模式
    TIM3->CCMR1 = (6 << TIM_CCMR1_OC1M_Pos) | TIM_CCMR1_OC1PE; // PWM1 + preload
    TIM3->CCER  = TIM_CCER_CC1E;                              // enable output

    //配置 38kHz 参数
    TIM3->PSC = 1;
    TIM3->ARR = 3289;
    TIM3->CCR1 = 823;  // 25% 占空比

    //更新寄存器
    TIM3->EGR = TIM_EGR_UG;
    TIM3->CR1 |= TIM_CR1_ARPE;

    //重启 PWM
    __HAL_TIM_ENABLE(&htim3);
    HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
}

void Air_Select_Channel(uint8_t no){
	if(no >= 8){
		return;
	}
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,(no & 1) ? 1 : 0);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,(no & 2) ? 1 : 0);
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,(no & 4) ? 1 : 0);
}

void Air_String_Poll(){
	Air_Set_38kHz();
	HAL_TIM_PWM_Start(&htim3, TIM_CHANNEL_1);
	Air_Trigger_tmp = 0b00111111;
	for(uint8_t i = 0;i<6;i++){
		osDelay(1);
		Air_Expect = i;
		Air_Select_Channel(i);
		osDelay(1);
		Air_Select_Channel(7);
	}
	Air_Trigger_Status = Air_Trigger_tmp;
	osDelay(5);
}

void Air_String_IRQHandler(uint8_t no){
	if(no == Air_Expect){
		Air_Trigger_tmp &= ~(1 << no);
	}
}

