/*
 * can.h
 *
 *  Created on: Jun 14, 2023
 *      Author: dovlati
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

uint8_t CheckCanError( void );

void CanSend(uint8_t *TxData, uint8_t identifier );

void Send_cell_data(cell_data_t cell_data[][CELL_NUM]);
#endif /* INC_CAN_H_ */