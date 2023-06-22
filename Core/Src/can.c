/*
 * can.c
 *
 *  Created on: Jun 14, 2023
 *      Author: dovlati
 */


#include "stm32g4xx.h"
#include "conf.h"


extern FDCAN_HandleTypeDef hfdcan;
extern FDCAN_TxHeaderTypeDef TxHeader;
extern FDCAN_RxHeaderTypeDef RxHeader;


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

void CanSend(uint8_t *TxData, uint8_t identifier ){

	TxHeader.Identifier = identifier;

	if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan, &TxHeader, TxData) != HAL_OK){
	        // Transmission request Error
		Error_Handler();
	}

}

int ReadCANBusMessage(uint32_t messageIdentifier, uint8_t* RxData1, size_t size)
{

    /* Check if a new message is available in RX FIFO 0 */
    if(HAL_FDCAN_GetRxMessage(&hfdcan, FDCAN_RX_FIFO0, &RxHeader, RxData1) != HAL_OK)
    {
        // If there's an error reading the message, you can handle it here
        // For example, you could print an error message
        Error_Handler();
    }

    /* Validate the Identifier */
    if(RxHeader.Identifier == messageIdentifier)
    {
        return 0; // Message successfully read and validated
    }
    return -1; // Message Identifier did not match
}



void Send_cell_data(cell_data_t cell_data[][CELL_NUM]){

	uint8_t cell_id = 0;
	for(int i = 0; i < IC_NUM; i++){
		for(int j = 0; j < CELL_NUM; j += 3){
			uint16_t buf = cell_data[i][j].voltage;
			uint16_t buf2 = cell_data[i][j+1].voltage;
			uint16_t buf3 = cell_data[i][j+3].voltage;

			uint8_t c1_1 = buf;
			uint8_t c2_1 = buf >> 8;

			uint8_t c1_2 = buf2;
			uint8_t c2_2 = buf2 >> 8;

			uint8_t c1_3 = buf3;
			uint8_t c2_3 = buf3 >> 8;



			cell_id = i * 18 + j;
			uint8_t TxData[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, 0, 0};

			CanSend(TxData, cell_id);
			delay_u(100);
		}
	}
}

//////////////////////////////////////////////////////////////////////
static void payload_sort_old_to_new(uint32_t *payload, uint8_t *data)
{
	payload[0] = (data[0] << 0)|(data[1] << 8)|(data[2] << 16)|(data[3] << 24);
	payload[1] = (data[4] << 0)|(data[5] << 8)|(data[6] << 16)|(data[7] << 24);
}

static void payload_sort_new_to_old(uint8_t * data, uint32_t *payload)
{

	data[0] = (payload[0] >> 0);
	data[1] = (payload[0] >> 8);
	data[2] = (payload[0] >> 16);
	data[3] = (payload[0] >> 24);

	data[4] = (payload[1] >> 0);
	data[5] = (payload[1] >> 8);
	data[6] = (payload[1] >> 16);
	data[7] = (payload[1] >> 24);


}

/*int32_t cantx_voltage_limp_total(void)
{
    uint32_t ret;
    uint8_t data[8];
    uint32_t sum_of_cells = status_data.sum_of_cells;
    uint8_t limping = status_data.limping;

    data[0] = sum_of_cells >> 24;
    data[1] = sum_of_cells >> 16;
    data[2] = sum_of_cells >> 8;
    data[3] = sum_of_cells;

    data[4] = limping;
    data[5] = 0;
    data[6] = 0xAB;
    data[7] = 0xCD;

    uint32_t mailbox;


    TxHeader.Identifier = CAN_ID_VOLT_TOTAL;
    TxHeader. = 0;
    TxHeader.RTR = CAN_RTR_DATA;
    TxHeader.IDE = CAN_ID_STD;
    TxHeader.DLC = 8;

    ret = HAL_CAN_AddTxMessage(&hcan1, &TxHeader, data, &mailbox);
    if (ret != HAL_OK)
    {
        // Handle Error
        return -1;
    }
    return 0;
}*/





