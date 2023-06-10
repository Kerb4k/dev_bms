/*
 * can.h
 *
 *  Created on: 10.6.2023
 *      Author: dovlati
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

void HandleRxMessage(FDCAN_RxHeaderTypeDef* rxHeader, uint8_t* rxData);

void SendCanMessage(uint32_t messageId, uint8_t* data, uint8_t dataLength);

#endif /* INC_CAN_H_ */
