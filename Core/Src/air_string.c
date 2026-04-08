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
    //配置 38kHz 参数
    TIM3->PSC = 1;
    TIM3->ARR = 3289;
    TIM3->CCR1 = 823;  // 25% 占空比

    //更新寄存器
    TIM3->EGR = TIM_EGR_UG;
    TIM3->CR1 |= TIM_CR1_ARPE;
}

void Air_Select_Channel(uint8_t no){
	if(no >= 8){
		return;
	}
	HAL_GPIO_WritePin(GPIOB,GPIO_PIN_15,(no & 4) ? 1 : 0);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_8,(no & 2) ? 1 : 0);
	HAL_GPIO_WritePin(GPIOA,GPIO_PIN_10,(no & 1) ? 1 : 0);
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
//		Air_String_Check(i);
		Air_Select_Channel(7);
	}
	Air_Trigger_Status = Air_Trigger_tmp;
	__HAL_TIM_SET_COMPARE(&htim3 , TIM_CHANNEL_1 , 0);
	osDelay(1);
	HAL_TIM_PWM_Stop(&htim3, TIM_CHANNEL_1);
}

void Air_String_IRQHandler(uint8_t no){
	if(no == Air_Expect){
		Air_Trigger_tmp &= ~(1 << no);
	}
}

void Air_String_Check(uint8_t no){
	switch(no){
	case 0:{
		if(!HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_5)){
			Air_Trigger_tmp &= ~(1 << no);
		}
	}
	case 1:{
		if(!HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_6)){
			Air_Trigger_tmp &= ~(1 << no);
		}
	}
	case 2:{
		if(!HAL_GPIO_ReadPin(GPIOB,GPIO_PIN_8)){
			Air_Trigger_tmp &= ~(1 << no);
		}
	}
	case 3:{
		if(!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_13)){
			Air_Trigger_tmp &= ~(1 << no);
		}
	}
	case 4:{
		if(!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_14)){
			Air_Trigger_tmp &= ~(1 << no);
		}
	}
	case 5:{
		if(!HAL_GPIO_ReadPin(GPIOC,GPIO_PIN_15)){
			Air_Trigger_tmp &= ~(1 << no);
		}
	}
	}
//		Air_Trigger_tmp &= ~(1 << no);
}
