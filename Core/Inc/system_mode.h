/*
 * system_mode.h
 *
 *  Created on: Jan 26, 2026
 *      Author: Qinh
 */

#ifndef INC_SYSTEM_MODE_H_
#define INC_SYSTEM_MODE_H_

#include "stdint.h"

struct mode_t {
	uint8_t Power_Mode;
	uint8_t Game_Mode;
	uint8_t Output_Mode;
};

enum{
	POWER_ON = 0,
	POWER_OFF = 1,
};

extern struct mode_t sys_mode;

void System_Mode_IROHandler();
void System_Mode_Init();

#endif /* INC_SYSTEM_MODE_H_ */
