/*
 * dwt.c
 *
 *  Created on: Jul 11, 2026
 *      Author: Qinh
 */
#include "main.h"
/**
  * @brief  初始化 Cortex-M33 内核的 DWT 周期计数器
  */
void DWT_Init(void)
{
    CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
#ifdef DWT_LSR_Present
    DWT->LSR = 0xC5ACCE55;
#endif
    DWT->CYCCNT = 0;
    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
}

/**
  * @brief  使用内核 DWT 实现的阻塞式微秒延时（免中断，对 FreeRTOS 隐藏）
  * @param  us: 延时微秒数
  */
void DWT_Delay_us(uint32_t us)
{
    uint32_t ticks_per_us = SystemCoreClock / 1000000;
    uint32_t total_ticks = us * ticks_per_us;
    uint32_t start_tick = DWT->CYCCNT;
    while ((DWT->CYCCNT - start_tick) < total_ticks)
    {
        __NOP();
    }
}

/**
  * @brief  毫秒级延时封装
  */
void DWT_Delay_ms(uint32_t ms)
{
    while(ms--)
    {
        DWT_Delay_us(1000);
    }
}



