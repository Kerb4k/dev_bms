/*
 * can.c
 *
 *  Created on: Jun 14, 2023
 *      Author: dovlati
 */


#include "stm32g4xx.h"

extern FDCAN_HandleTypeDef hfdcan;
extern FDCAN_TxHeaderTypeDef TxHeader;

#define ERR_CANOFFLINE				11
uint8_t canSendErrorFlag;

uint8_t CheckCanError( void )
{
	FDCAN_ProtocolStatusTypeDef CAN1Status;

	static uint8_t offcan1 = 0;

	HAL_FDCAN_GetProtocolStatus(&hfdcan, &CAN1Status);

	static uint8_t offcan = 0;

	if ( !offcan1 && CAN1Status.BusOff) // detect passive error instead and try to stay off bus till clears?
	{
		  HAL_FDCAN_Stop(&hfdcan);
		  Set_Error(ERR_CANOFFLINE);
		  // set LED.
		  offcan = 1;
		  return 0;
	}

	// use the senderrorflag to only try once a second to get back onbus.
	if ( CAN1Status.BusOff && canSendErrorFlag )
	{
		if (HAL_FDCAN_Start(&hfdcan) == HAL_OK)
		{
			offcan = 0;
		}
	}

	return offcan;
}

void CanSend(uint8_t *TxData ){

	if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan, &TxHeader, TxData) != HAL_OK){
	        // Transmission request Error
		Error_Handler();
	}

}


