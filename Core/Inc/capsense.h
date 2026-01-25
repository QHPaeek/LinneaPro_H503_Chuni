/*
 * capsense.h
 *
 *  Created on: Jan 8, 2025
 *      Author: Qinh
 */

#ifndef INC_CAPSENSE_H_
#define INC_CAPSENSE_H_

#include <stdint.h>

typedef union{
	struct{
		uint8_t touc_data_a[64];
		uint8_t touc_data_b[64];
		uint8_t touc_data_c[64];
		uint8_t touc_data_d[64];
	};
	struct{
		uint16_t channel_raw[128];
	};
}packet_capsense_t;

extern packet_capsense_t Touch;
extern uint8_t capsense_touch_status[128];
extern uint16_t capsense_baseline[128];

void capsense_init();
//void capsense_baseline_updata();
void capsense_poll();

#endif /* INC_CAPSENSE_H_ */
