#include "eeprom_emul.h"
#include "stdint.h"
#include "usbd_def.h"
#include <stdarg.h>
#include <stdio.h>
#include <string.h>
#include "flash.h"
#include "capsense.h"

extern uint16_t capsense_maxmium[128];
extern uint8_t capsense_sava_flag[128];

/************************************************
 * STM32H503 Flash EEPROM Emulation
 ************************************************/

/************************************************
 * RAM镜像
 ************************************************/

EEPROM_DATA_t g_eeprom;


/************************************************
 * CRC16
 ************************************************/

static uint16_t EEPROM_CalcCRC(uint8_t *buf,uint32_t len)
{
    uint16_t crc = 0xFFFF;

    while(len--)
    {
        crc ^= *buf++;

        for(uint8_t i=0;i<8;i++)
        {
            if(crc & 1)
                crc = (crc >> 1) ^ 0xA001;
            else
                crc >>= 1;
        }
    }

    return crc;
}


/************************************************
 * Sector计算
 ************************************************/

static uint32_t GetSector(uint32_t Address)
{
    uint32_t sector;

    if((Address >= FLASH_BASE) &&
       (Address < FLASH_BASE + FLASH_BANK_SIZE))
    {
        sector =
            (Address - FLASH_BASE)
            / FLASH_SECTOR_SIZE;
    }
    else
    {
        sector =
            ((Address - FLASH_BASE)
             - FLASH_BANK_SIZE)
            / FLASH_SECTOR_SIZE;
    }

    return sector;
}


/************************************************
 * Bank计算
 ************************************************/

static uint32_t GetBank(uint32_t Addr)
{
    if(Addr < (FLASH_BASE + FLASH_BANK_SIZE))
    {
        return FLASH_BANK_1;
    }

    return FLASH_BANK_2;
}


/************************************************
 * 擦除EEPROM Sector
 ************************************************/

static HAL_StatusTypeDef EEPROM_EraseSector(void)
{
    FLASH_EraseInitTypeDef erase;

    uint32_t sector_error;

    erase.TypeErase = FLASH_TYPEERASE_SECTORS;

    erase.Banks =
        GetBank(EEPROM_FLASH_ADDR);

    erase.Sector =
        GetSector(EEPROM_FLASH_ADDR);

    erase.NbSectors = 1;

    return HAL_FLASHEx_Erase(
                &erase,
                &sector_error);
}

/************************************************
 * 保存到Flash
 ************************************************/

HAL_StatusTypeDef EEPROM_Save(void)
{
    HAL_StatusTypeDef ret;

    uint32_t addr;

    uint8_t *src;

    g_eeprom.crc =EEPROM_CalcCRC((uint8_t *)&g_eeprom,sizeof(EEPROM_DATA_t)- sizeof(uint16_t));

    HAL_FLASH_Unlock();

//    if(HAL_ICACHE_Disable()!=HAL_OK)
//    {
//        HAL_FLASH_Lock();
//        return HAL_ERROR;
//    }

    ret = EEPROM_EraseSector();

    if(ret != HAL_OK)
    {
//        HAL_ICACHE_Enable();
        HAL_FLASH_Lock();
        return ret;
    }

    addr = EEPROM_FLASH_ADDR;

    src = (uint8_t *)&g_eeprom;

    while(addr < EEPROM_FLASH_ADDR + sizeof(EEPROM_DATA_t)){
        ret = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD,addr,(uint32_t)src);
        if(ret != HAL_OK){
//            HAL_ICACHE_Enable();
            HAL_FLASH_Lock();

            return ret;
        }

        addr += 16;
        src  += 16;
    }

//    HAL_ICACHE_Enable();

    HAL_FLASH_Lock();
    return HAL_OK;
}

/************************************************
 * 从Flash读取
 ************************************************/

uint8_t EEPROM_Load(void)
{
    EEPROM_DATA_t *flash = (EEPROM_DATA_t *)EEPROM_FLASH_ADDR;
    memcpy(&g_eeprom,flash,sizeof(EEPROM_DATA_t));
    if(g_eeprom.magic != EEPROM_MAGIC){
        memset(&g_eeprom,0,sizeof(g_eeprom));
        g_eeprom.magic = EEPROM_MAGIC;
        EEPROM_Save();
    	capsense_maxmium[0] = 65454;
        return 0;
    }

	capsense_maxmium[0] = 65252;
	return 1;
}

/************************************************
 * 读一个参数
 ************************************************/

uint16_t EEPROM_Read(uint16_t index)
{
    if(index >= EEPROM_ITEM_COUNT)
    {
        return 0;
    }

    return g_eeprom.data[index];
}


/************************************************
 * 写一个参数
 ************************************************/

HAL_StatusTypeDef EEPROM_Write(
        uint16_t index,
        uint16_t value)
{
    if(index >= EEPROM_ITEM_COUNT)
    {
        return HAL_ERROR;
    }

    g_eeprom.data[index] = value;

    return EEPROM_Save();
}


/************************************************
 * 获取RAM缓存指针
 ************************************************/

uint16_t* EEPROM_GetBuffer(void)
{
    return g_eeprom.data;
}

void App_EEPROM_Save(){
	uint8_t ret = 0;
	for(uint8_t i = 0;i<128;i++){
		if(capsense_sava_flag[i]){
			g_eeprom.data[i] = capsense_maxmium[i];
			capsense_sava_flag[i] = 0;
			ret++;
		}
	}
	if(ret){
		EEPROM_Save();
	}
}
