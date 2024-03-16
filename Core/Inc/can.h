/*
 * can.h
 *
 *  Created on: Jun 14, 2023
 *      Author: dovlati
 */

#ifndef INC_CAN_H_
#define INC_CAN_H_

#include "conf.h"

int ReadCANBusMessage(uint32_t messageIdentifier, uint8_t* RxData1);

uint8_t CheckCanError( void );

void CanSend(uint8_t *TxData[], uint32_t identifier );

uint8_t Send_cell_data(cell_data_t cell_data[][CELL_NUM]);

void Send_temp_data(temp_data_t temp_data[][GPIO_NUM]);

void Send_Soc(status_data_t *status_data);

#endif /* INC_CAN_H_ */
