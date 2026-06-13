#ifndef __EEPROM_FLASH_H
#define __EEPROM_FLASH_H

#include "main.h"

#define EEPROM_FLASH_ADDR      0x0801E000UL

#define EEPROM_MAGIC           0x55AA1234UL

#define EEPROM_ITEM_COUNT      128

typedef struct
{
    uint32_t magic;

    uint16_t data[EEPROM_ITEM_COUNT];

    uint16_t crc;

} EEPROM_DATA_t;

uint8_t EEPROM_Load(void);

HAL_StatusTypeDef EEPROM_Save(void);

uint16_t EEPROM_Read(uint16_t index);

HAL_StatusTypeDef EEPROM_Write(
        uint16_t index,
        uint16_t value);

uint16_t* EEPROM_GetBuffer(void);

#endif
