#ifndef INC_FLASH_H_
#define INC_FLASH_H_

#include "stdint.h"

#define FLASH_DATA_RAW_SIZE  288  // 你需要的总字节数

typedef union
{
    uint32_t quadword[FLASH_DATA_RAW_SIZE / 4] __attribute__((aligned(16)));
    struct
    {
        uint16_t capsense_maxmium[128]; // 256 字节
        uint8_t  value8[16];            // 16 字节
        uint8_t  reserve[16];           // 16 字节
    };
} FlashData;

_Static_assert(sizeof(FlashData) == FLASH_DATA_RAW_SIZE, "FlashData size is wrong!");
_Static_assert(sizeof(FlashData) % 16 == 0, "FlashData must be 16-byte aligned!");

extern FlashData Flash;

void Flash_Save(FlashData *data);
void Flash_Load(FlashData *data);

#endif
