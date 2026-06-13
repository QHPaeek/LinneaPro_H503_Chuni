/**
  ******************************************************************************
  * @file    EEPROM_Emul/Porting/STM32H5/flash_interface.c
  * @author  MCD Application Team
  * @brief   This file provides all the EEPROM emulation flash interface functions.
  ******************************************************************************
  * @attention
  *
  * <h2><center>&copy; Copyright (c) 2020 STMicroelectronics.
  * All rights reserved.</center></h2>
  *
  * This software component is licensed by ST under BSD 3-Clause license,
  * the "License"; You may not use this file except in compliance with the
  * License. You may obtain a copy of the License at:
  *                        opensource.org/licenses/BSD-3-Clause
  *
  ******************************************************************************
  */

/* Includes ------------------------------------------------------------------*/
#include "eeprom_emul.h"
#include "flash_interface.h"

/** @addtogroup EEPROM_Emulation
  * @{
  */

/* Private typedef -----------------------------------------------------------*/
/* Private constants ---------------------------------------------------------*/
/* Private macro -------------------------------------------------------------*/
/* Private variables ---------------------------------------------------------*/
uint64_t FlashWord[2] =
{
  0x0, 0x0
};
uint8_t FlashWord_status = 0; /* 0 is FlashWord is empty, 1 it is full */
const uint32_t QuadWord[4] = 
{ 
0x00000000,
0x00000000,
0x00000000,
0x00000000
};

/* Private function prototypes -----------------------------------------------*/
#ifdef FLASH_BANK_2
static uint32_t GetBankNumber(uint32_t Address);
#endif

/* Exported functions --------------------------------------------------------*/
/* Private functions ---------------------------------------------------------*/
/** @addtogroup EEPROM_Private_Functions
  * @{
  */

/**
  * @brief  Write a quad word at the given address in Flash (function is called FI_WriteDoubleWord to respect legacy for X-CUBE-EEPROM)
  * @param  Address Where to write
  * @param  Data What to write
  * @param  Write_type Type of writing on going (see EE_Write_type)
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE_WRITE_ERROR: if an error occurs
  *           - EE_FLASH_USED: flash currently used by CPU2
  */
#ifdef EDATA_ENABLED
EE_Status FI_WriteDoubleWord(uint32_t Address, uint64_t Data)
{
  uint16_t *HalfWordData = (uint16_t*)&Data;  
  EE_Status status = EE_OK;
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD_EDATA, Address, (uint32_t)&(HalfWordData[0])) != HAL_OK)
  {
    status = EE_WRITE_ERROR;
  }
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD_EDATA, Address+2, (uint32_t)&(HalfWordData[1])) != HAL_OK)
  {
    status = EE_WRITE_ERROR;
  }
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD_EDATA, Address+4, (uint32_t)&(HalfWordData[2])) != HAL_OK)
  {
    status = EE_WRITE_ERROR;
  }
  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD_EDATA, Address+6, (uint32_t)&(HalfWordData[3])) != HAL_OK)
  {
    status = EE_WRITE_ERROR;
  }

  return status;
}
#else 
//EE_Status FI_WriteDoubleWord(uint32_t Address, uint64_t* Data, EE_Write_type Write_type)
//{
//  EE_Status status = EE_OK;
//  if(Write_type == EE_SET_PAGE)
//  {
//    FlashWord[0] = Data[0];
//    FlashWord[1] = Data[0];
//  }
//  else
//  {
//    FlashWord[0] = Data[0];
//    FlashWord[1] = Data[1];
//  }
//
//  if(HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, Address, (uint32_t)FlashWord) != HAL_OK)
//  {
//      status = EE_WRITE_ERROR;
//  }
//  return status;
//}
EE_Status FI_WriteDoubleWord(uint32_t Address, uint64_t* Data, EE_Write_type Write_type)
{
  // 1. 将官方核心算法传进来的伪指针安全还原为真实的 64 位特征数据
  uint64_t data = (uint64_t)Data;
  HAL_StatusTypeDef hal_status;

  // 2. 利用官方自带的全局 uint64_t FlashWord[2] 数组进行 128 位打包
  FlashWord[0] = data;                  // 低 64 位填入特征数据
  FlashWord[1] = 0xFFFFFFFFFFFFFFFFULL; // 高 64 位用闪存默认空白值填充占位

  // 3. 严格的 H503 16字节对齐物理检查
  if (Address % 16 != 0) {
      return EE_WRITE_ERROR;
  }
  
  // 4. 清除先前由于任何潜在读取或越界引发的物理 Flash 硬件报错挂起
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);

  // 5. 【核心修正】：直接传入 FlashWord 数组名（它本身就是指针）
  // 或者写成 (uint32_t)&FlashWord[0]，千万不要直接写 (uint32_t)FlashWord ！！！
  hal_status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, Address, (uint32_t)FlashWord);

  // 6. 返回结果对齐
  if (hal_status == HAL_OK) {
      return EE_OK;
  } else {
      return EE_WRITE_ERROR;
  }
}
#endif

/**
  * @brief  Erase a page in polling mode
  * @param  Page Page number
  * @param  NbPages Number of pages to erase
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_PageErase(uint32_t Page, uint16_t NbPages)
{
  EE_Status status = EE_OK;
  FLASH_EraseInitTypeDef s_eraseinit;
  uint32_t bank = FLASH_BANK_1, page_error = 0U;

#ifdef FLASH_BANK_2
  bank = GetBankNumber(PAGE_ADDRESS(Page));
#endif
  
  s_eraseinit.TypeErase   = FLASH_TYPEERASE_SECTORS; /* if TrustZone secure activated -> FLASH_TYPEERASE_PAGES_NS; */
  s_eraseinit.NbSectors   = NbPages;
#ifdef EDATA_ENABLED
  s_eraseinit.Sector      = Page + 120;
#else
  s_eraseinit.Sector      = Page;
#endif
  s_eraseinit.Banks       = bank;

  /* Erase the Page: Set Page status to ERASED status */
  if (HAL_FLASHEx_Erase(&s_eraseinit, &page_error) != HAL_OK)
  {
    status = EE_ERASE_ERROR;
  }
  return status;
}

/**
  * @brief  Erase a page with interrupt enabled
  * @param  Page Page number
  * @param  NbPages Number of pages to erase
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_PageErase_IT(uint32_t Page, uint16_t NbPages)
{
  EE_Status status = EE_OK;
  FLASH_EraseInitTypeDef s_eraseinit;
  uint32_t bank = FLASH_BANK_1;

#ifdef FLASH_BANK_2
  bank = GetBankNumber(PAGE_ADDRESS(Page));
#endif
  
  s_eraseinit.TypeErase   = FLASH_TYPEERASE_SECTORS; /* if TrustZone secure activated -> FLASH_TYPEERASE_PAGES_NS; */
  s_eraseinit.NbSectors   = NbPages;
#ifdef EDATA_ENABLED
  s_eraseinit.Sector      = Page + 120;
#else
  s_eraseinit.Sector      = Page;
#endif
  s_eraseinit.Banks       = bank;

  /* Erase the Page: Set Page status to ERASED status */
  if (HAL_FLASHEx_Erase_IT(&s_eraseinit) != HAL_OK)
  {
    status = EE_ERASE_ERROR;
  }
  return status;
}

/**
  * @brief  Flush the caches if needed to keep coherency when the flash content is modified
  */
void FI_CacheFlush()
{
  /* No flush needed. EEPROM flash area is defined as non-cacheable thanks to the MPU in main.c. */
  return;
}

#ifdef FLASH_BANK_2
/**
  * @brief  Gets the bank of a given address
  * @param  Address Address of the FLASH Memory
  * @retval Bank_Number The bank of a given address
  */
static uint32_t GetBankNumber(uint32_t Address)
{
  uint32_t bank = 0U;

  if (READ_BIT(FLASH->OPTSR_CUR, FLASH_OPTSR_SWAP_BANK) == 0U)
  {
    /* No Bank swap */
#if EDATA_ENABLED
    if (Address < (FLASH_EDATA_BASE + (FLASH_EDATA_SIZE/2)))
#else
    if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
#endif
    {
      bank = FLASH_BANK_1;
    }
    else
    {
      bank = FLASH_BANK_2;
    }
  }
  else
  {
    /* Bank swap */
#if USE_EDATA_AREA
    if (Address < (FLASH_EDATA_BASE + (FLASH_EDATA_SIZE/2)))
#else
    if (Address < (FLASH_BASE + FLASH_BANK_SIZE))
#endif
    {
      bank = FLASH_BANK_2;
    }
    else
    {
      bank = FLASH_BANK_1;
    }
  }
  
  return bank;
}
#endif

/**
  * @brief  Delete corrupted Flash address, can be called from NMI. No Timeout.
  * @param  Address Address of the FLASH Memory to delete
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_DeleteCorruptedFlashAddress(uint32_t Address)
{
#ifdef EDATA_ENABLED
  uint16_t HalfWord[4] = {0U, 0U, 0U, 0U};
#endif
  EE_Status status = EE_OK;
  /* Wait for previous programmation completion */
  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY))
  {
  }
  
  /* Clear previous non-secure error programming flag */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ALL_ERRORS);
  
  /* Set FLASH Programmation bit */
  SET_BIT(FLASH->NSCR, FLASH_CR_PG);

  /* Program quad word of value 0 */
  
#ifdef EDATA_ENABLED
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD_EDATA, Address, (uint32_t)&(HalfWord[0]));
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD_EDATA, Address+2, (uint32_t)&(HalfWord[1]));
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD_EDATA, Address+4, (uint32_t)&(HalfWord[2]));
  HAL_FLASH_Program(FLASH_TYPEPROGRAM_HALFWORD_EDATA, Address+6, (uint32_t)&(HalfWord[3]));
#else
 // HAL_FLASH_Program(FLASH_TYPEPROGRAM_QUADWORD, Address, ((uint32_t)QuadWord));
  *(__IO uint64_t*)(Address) = (uint64_t)0U;
  *(__IO uint64_t*)(Address+8U) = (uint64_t)0U;
#endif 
  /* Wait programmation completion */
  while(__HAL_FLASH_GET_FLAG(FLASH_FLAG_BSY))
  {
  }

  /* Check if error occured */
  if((__HAL_FLASH_GET_FLAG(FLASH_FLAG_WRPERR))  || (__HAL_FLASH_GET_FLAG(FLASH_FLAG_PGSERR)) ||
     (__HAL_FLASH_GET_FLAG(FLASH_FLAG_STRBERR)) || (__HAL_FLASH_GET_FLAG(FLASH_FLAG_INCERR)))
  {
    status = EE_DELETE_ERROR;
  }

  /* Check FLASH End of Operation flag  */
  if (__HAL_FLASH_GET_FLAG(FLASH_FLAG_EOP))
  {
    /* Clear FLASH End of Operation pending bit */
    __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_EOP);
  }

  /* Clear FLASH Programmation bit */
  CLEAR_BIT(FLASH->NSCR, FLASH_CR_PG);

  /* Clear FLASH ECCD bit */
  __HAL_FLASH_CLEAR_FLAG(FLASH_FLAG_ECCD);

  return status;
}

/**
  * @brief  Check if the configuration is 128-bits bank or 2*64-bits bank
  * @param  None
  * @retval EE_Status
  *           - EE_OK: on success
  *           - EE error code: if an error occurs
  */
EE_Status FI_CheckBankConfig(void)
{
#if defined (FLASH_OPTR_DBANK)
  FLASH_OBProgramInitTypeDef sOBCfg;
  EE_Status status;

  /* Request the Option Byte configuration :
     - User and RDP level are always returned
     - WRP and PCROP are not requested */
  sOBCfg.WRPArea     = 0xFF;
  HAL_FLASHEx_OBGetConfig(&sOBCfg);

  /* Check the value of the DBANK user option byte */
  if ((sOBCfg.USERConfig & OB_DBANK_64_BITS) != 0)
  {
    status = EE_OK;
  }
  else
  {
    status = EE_INVALID_BANK_CFG;
  }

  return status;
#else
  /* No feature 128-bits single bank, so always 64-bits dual bank */
  return EE_OK;
#endif
}

/**
  * @brief  INSERT BRIEF
  * @param  INSERT BRIEF
  *         INSERT BRIEF
  *         INSERT BRIEF
  *         INSERT BRIEF
  *         INSERT BRIEF
  * @retval None
  */
void OB_Init( void )
{
//  uint32_t bank = FLASH_BANK_2;
//  uint32_t num_of_edata_page = 1;
//  uint32_t register_value;
//  uint32_t edata_enable = 0;
//
//
//  num_of_edata_page = ((FLASH_EDATA_BASE + (FLASH_EDATA_SIZE>>1)) - START_PAGE_ADDRESS) / FLASH_PAGE_SIZE;
//  register_value = FLASH->EDATA2R_CUR;
//  edata_enable = FLASH_EDATAR_EDATA_EN;
//
//  /* If current EDATA configration is different from new configration, Reprogram Option Byte EDATA settings */
//  if(register_value != (edata_enable | (num_of_edata_page - 1)))
//  {
//    /* Variable used for OB Program procedure */
//    FLASH_OBProgramInitTypeDef FLASH_OBInitStruct;
//
//    /* Current EDATA configuration doesn't match with new configuration. */
//    /* Unlock the Flash option bytes to enable the flash option control register access */
//    HAL_FLASH_OB_Unlock();
//
//    /* Configure 8 sectors for FLASH high-cycle data */
//    FLASH_OBInitStruct.Banks = bank;
//    FLASH_OBInitStruct.OptionType = OPTIONBYTE_EDATA;
//    FLASH_OBInitStruct.EDATASize = edata_enable | num_of_edata_page;
//
//    if(HAL_FLASHEx_OBProgram(&FLASH_OBInitStruct) != HAL_OK)
//    {
//      Error_Handler();
//    }
//
//    /* Start option byte load operation after successful programming operation */
//    HAL_FLASH_OB_Launch();
//
//    /* Lock the Flash control option to restrict register access */
//    HAL_FLASH_OB_Lock();
//  }

}

/**
  * @brief  This function is executed in case of error occurrence.
  * @retval None
  */
//static void Error_Handler(void)
//{
//  while(1)
//  {
//
//  }
//}

/**
  * @}
  */

/**
  * @}
  */

/************************ (C) COPYRIGHT STMicroelectronics *****END OF FILE****/

