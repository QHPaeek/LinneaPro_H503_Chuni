#include "flash.h"
#include "string.h"
#include "stm32h5xx_hal.h"

#define FLASH_SAVE_ADDR    0x0801C000U  // 扇区7起始地址，确保不在代码区
#define FLASH_SECTOR       FLASH_SECTOR_7

#define FLASH_DATA_SIZE      sizeof(FlashData)
#define FLASH_QUADWORD_SIZE  16U
// 向上取整到16字节的倍数
#define FLASH_QUADWORD_NUM   (sizeof(FlashData) / 16)

// 全局工作副本
FlashData Flash;

void Flash_Save(FlashData *data)
{
	__disable_irq();
    HAL_StatusTypeDef status;

    // 1. 解锁
    HAL_FLASH_Unlock();

    // 2. 擦除扇区
    FLASH_EraseInitTypeDef EraseInit = {0};
    uint32_t SectorError = 0;

    EraseInit.TypeErase = FLASH_TYPEERASE_SECTORS;
    EraseInit.Banks     = FLASH_BANK_1;
    EraseInit.Sector    = FLASH_SECTOR;
    EraseInit.NbSectors = 1;

    status = HAL_FLASHEx_Erase(&EraseInit, &SectorError);
    if (status != HAL_OK)
    {
        // 擦除失败，直接上锁并返回
        HAL_FLASH_Lock();
        return;
    }

    // 3. 按四字（16字节）写入
    uint32_t flash_addr = FLASH_SAVE_ADDR;
    // 将 FlashData 看作 uint32_t 数组，每4个uint32_t为一组四字
    uint32_t *src = (uint32_t *)data;

    for (uint32_t i = 0; i < FLASH_QUADWORD_NUM; i++)
    {
        // 关键修正：把src[i*4]这个“值”传进去，而不是地址
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD,
                                   flash_addr,
                                   src[i * 4]);  // 这里直接传值
        if (status != HAL_OK)
        {
            break;
        }
        flash_addr += FLASH_QUADWORD_SIZE;
    }

    // 4. 上锁
    HAL_FLASH_Lock();
    __enable_irq();
}

void Flash_Load(FlashData *data)
{
    // 直接内存拷贝，读Flash和读RAM一样
    memcpy(data, (const void *)FLASH_SAVE_ADDR, sizeof(FlashData));
}
