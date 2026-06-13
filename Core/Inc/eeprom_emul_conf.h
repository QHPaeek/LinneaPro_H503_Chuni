/**
  ******************************************************************************
  * @file    eeprom_emul_conf.h
  * @author  MCD Application Team
  * @brief   EEPROM emulation configuration file.
  *          Tailored for STM32H503CBT6 (128 KB Flash)
  ******************************************************************************
  */

#ifndef __EEPROM_EMUL_CONF_H
#define __EEPROM_EMUL_CONF_H

#ifdef __cplusplus
 extern "C" {
#endif

/* 引入芯片的底层宏定义，确保能够识别 LL 库 */
#include "stm32h5xx_ll_crc.h"

/* Private constants ---------------------------------------------------------*/

/* 配置说明（针对 STM32H503CBT6 - 128KB FLASH）：
 * H503CBT6 物理 FLASH 地址：0x08000000 -> 0x0801FFFF，共 16 个页（Page 0 ~ Page 15），每页 8KB。
 *
 * 方案 A（启用 EDATA 高寿命）：通过选项字节将最后的 Page 14 和 Page 15 设为 EDATA。
 * 方案 B（常规 FLASH 仿真）：直接占用最后的几个页面。
 */
#ifdef EDATA_ENABLED
  /* 开启 EDATA 时，高寿命区的映射起始地址（具体需配合 CubeProgrammer 的 EDATA_STRT 选项字节设置） */
  /* 示例：假设将物理最后两页转为 EDATA，地址一般保持默认或映射基地址 */
  #define START_PAGE_ADDRESS      0x09000000U
#else
  /* 未开启 EDATA 时，占用普通 FLASH 最后的两页（Page 14 和 Page 15） */
  /* 0x0801C000 是 Page 14 的起始地址（距离末尾还剩 16KB 空间，刚好够仿真库运转） */
  #define START_PAGE_ADDRESS      0x0801C000U
#endif

/* 寿命倍增系数：H503 EDATA 原生支持 10万次，这里保持 1U 即可 */
#define CYCLES_NUMBER           1U

/* 警戒页数量：必须是 2 的倍数。由于 H503 一页高达 8KB，对于 128 个 uint16_t 来说空间极其充裕，
 * 保持 2U 即可（此时库一共会占用 START_PAGE_ADDRESS 开始的共 4 个页面，即 32KB 空间）。
 * 注意：如果 128KB 闪存不够用，可以将此值改为 0U，库将只占用 2 个页面（16KB）。 */
#define GUARD_PAGES_NUMBER      0U

#define CRC_POLYNOMIAL_LENGTH   LL_CRC_POLYLENGTH_16B
#define CRC_POLYNOMIAL_VALUE    0x8005U

/* Exported constants --------------------------------------------------------*/

/* 支持管理的虚拟变量总数*/
#define NB_OF_VARIABLES         128U

#ifdef __cplusplus
}
#endif

#endif /* __EEPROM_EMUL_CONF_H */
