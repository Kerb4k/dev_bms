/*
 * can.c
 *
 *  Created on: Jun 5, 2023
 *      Author: dovlati
 */


#include "stm32g4xx.h"

extern FDCAN_HandleTypeDef hfdcan1;

#define CLOCK_ID 0
#define IVT_ID 1
#define COMMAND_ID 2
#define STATUS_ID 3

void HandleRxMessage(FDCAN_RxHeaderTypeDef* rxHeader, uint8_t* rxData)
{
    if (rxHeader->Identifier == CLOCK_ID)
    {
        /* Extract the clock value from the received message */
        uint32_t clockValue = 0;
        clockValue |= (uint32_t)rxData[0] << 24;
        clockValue |= (uint32_t)rxData[1] << 16;
        clockValue |= (uint32_t)rxData[2] << 8;
        clockValue |= (uint32_t)rxData[3];

        /* Process the clock value */
        /* ... */
    }
}

void SendCanMessage(uint32_t messageId, uint8_t* data, uint8_t dataLength)
{

	if(!(HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) > 0))
		{
			//Set_Error(ERR_CAN_FIFO_FULL);
			return;
		}


    FDCAN_TxHeaderTypeDef TxHeader;
    TxHeader.Identifier = messageId;
    TxHeader.DataLength = FDCAN_DLC_BYTES_4;
    TxHeader.IdType = FDCAN_STANDARD_ID;
    TxHeader.TxFrameType = FDCAN_DATA_FRAME;
    TxHeader.ErrorStateIndicator = FDCAN_ESI_ACTIVE;
    TxHeader.BitRateSwitch = FDCAN_BRS_OFF;
    TxHeader.FDFormat = FDCAN_CLASSIC_CAN;
    TxHeader.TxEventFifoControl = FDCAN_NO_TX_EVENTS;
    TxHeader.MessageMarker = 0;

    	if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, data) != HAL_OK)
    	{
    		return;
    	}
}
