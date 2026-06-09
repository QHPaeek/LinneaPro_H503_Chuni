/*
 * serial_protocol.h
 *
 *  Created on: Nov 29, 2025
 *      Author: Qinh
 */

#ifndef INC_SERIAL_PROTOCOL_H_
#define INC_SERIAL_PROTOCOL_H_

#include "stdint.h"

extern uint8_t debug_channel;

void Serial_Receive_Handle(uint8_t* data,uint8_t len);

#endif /* INC_SERIAL_PROTOCOL_H_ */
