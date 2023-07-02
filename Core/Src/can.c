/*
 * can.c
 *
 *  Created on: Jun 14, 2023
 *      Author: dovlati
 */


#include "stm32g4xx.h"
#include "conf.h"


extern FDCAN_HandleTypeDef hfdcan1;
extern FDCAN_TxHeaderTypeDef TxHeader;
extern FDCAN_RxHeaderTypeDef RxHeader;

uint8_t nlg5a_buffer[4];
uint8_t nlg5b_buffer[4];



#define ERR_CANOFFLINE				11
uint8_t canSendErrorFlag;

uint8_t CheckCanError( void )
{
	FDCAN_ProtocolStatusTypeDef CAN1Status;

	static uint8_t offcan1 = 0;

	HAL_FDCAN_GetProtocolStatus(&hfdcan1, &CAN1Status);

	static uint8_t offcan = 0;

	if ( !offcan1 && CAN1Status.BusOff) // detect passive error instead and try to stay off bus till clears?
	{
		  HAL_FDCAN_Stop(&hfdcan1);
		  Set_Error(ERR_CANOFFLINE);
		  // set LED.
		  offcan = 1;
		  return 0;
	}

	// use the senderrorflag to only try once a second to get back onbus.
	if ( CAN1Status.BusOff && canSendErrorFlag )
	{
		if (HAL_FDCAN_Start(&hfdcan1) == HAL_OK)
		{
			offcan = 0;
		}
	}

	return offcan;
}

void CanSend(uint8_t *TxData, uint8_t identifier ){

	TxHeader.Identifier = identifier;

	if(HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK){
	        // Transmission request Error
		Error_Handler();
	}

}

int ReadCANBusMessage(uint32_t messageIdentifier, uint8_t* RxData1)
{
    /* Infinite loop to keep trying to read the message */
	uint8_t t = 0;

    while(t < 100)
    {
    	t++;
        /* Check if a new message is available in RX FIFO 0 */
        if(HAL_FDCAN_GetRxMessage(&hfdcan1, FDCAN_RX_FIFO0, &RxHeader, RxData1) == HAL_OK)
        {
            /* Validate the Identifier */
            if(RxHeader.Identifier == messageIdentifier)
            {
                return 0; // Message successfully read and validated
            }
            delay_u(10);
        }
        // Else, ignore the error and try again
    }
    return 1;
}



void Send_cell_data(cell_data_t cell_data[][CELL_NUM]){

	uint8_t cell_id = 0;
	for(int i = 0; i < IC_NUM; i++){
		for(int j = 0; j < CELL_NUM; j += 3){
			uint16_t buf = cell_data[i][j].voltage;
			uint16_t buf2 = cell_data[i][j+1].voltage;
			uint16_t buf3 = cell_data[i][j+2].voltage;

			uint8_t c1_1 = buf;
			uint8_t c2_1 = buf >> 8;

			uint8_t c1_2 = buf2;
			uint8_t c2_2 = buf2 >> 8;

			uint8_t c1_3 = buf3;
			uint8_t c2_3 = buf3 >> 8;



			cell_id = i * 18 + j/3 + 1912;
			uint8_t TxData[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, 0, 0};

			CanSend(TxData, cell_id);
			delay_u(100);
		}
	}
}

#define TEMP_FIXED 0
void Send_temp_data(temp_data_t temp_data[][GPIO_NUM]){

#if TEMP_FIXED
	uint8_t temp_id = 0;
		for(int i = 0; i < IC_NUM; i++){
			for(int j = 0; j < GPIO_NUM; j += 3){
				uint16_t buf = temp_data[i][j].temp;
				uint16_t buf2 = temp_data[i][j+1].temp;
				uint16_t buf3 = temp_data[i][j+2].temp;

				uint8_t c1_1 = buf;
				uint8_t c2_1 = buf >> 8;

				uint8_t c1_2 = buf2;
				uint8_t c2_2 = buf2 >> 8;

				uint8_t c1_3 = buf3;
				uint8_t c2_3 = buf3 >> 8;



				temp_id = i * 18 + j + 200;
				uint8_t TxData[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, 0, 0};

				CanSend(TxData, temp_id);
				delay_u(100);
			}
		}
#else
	uint8_t temp_id = 0;
	for(int i = 0; i < IC_NUM; i++){

			uint16_t buf = temp_data[i][0].temp;
			uint16_t buf2 = temp_data[i][1].temp;
			uint16_t buf3 = temp_data[i][2].temp;

			uint8_t c1_1 = buf;
			uint8_t c2_1 = buf >> 8;

			uint8_t c1_2 = buf2;
			uint8_t c2_2 = buf2 >> 8;

			uint8_t c1_3 = buf3;
			uint8_t c2_3 = buf3 >> 8;



			temp_id = i * 18 + 0 + 200;
			uint8_t TxData[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, 0, 0};

			CanSend(TxData, temp_id);
			delay_u(100);

			buf = temp_data[i][3].temp;
			buf2 = temp_data[i][4].temp;
			buf3 = temp_data[i][3].temp;

			c1_1 = buf;
			c2_1 = buf >> 8;

			c1_2 = buf2;
			c2_2 = buf2 >> 8;

			c1_3 = buf3;
			c2_3 = buf3 >> 8;



			temp_id = i * 18 + 3 + 200;
			uint8_t TxData1[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, 0, 0};

			CanSend(TxData, temp_id);
			delay_u(100);

			buf = temp_data[i][2].temp;
			buf2 = temp_data[i][1].temp;
			buf3 = temp_data[i][0].temp;

			c1_1 = buf;
			c2_1 = buf >> 8;

			c1_2 = buf2;
			c2_2 = buf2 >> 8;

			c1_3 = buf3;
			c2_3 = buf3 >> 8;



			temp_id = i * 18 + 6 + 200;
			uint8_t TxData2[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, 0, 0};
			CanSend(TxData, temp_id);
			delay_u(100);
		}
#endif



}



void Send_Soc(status_data_t *status_data){
	uint8_t Tx_Data[8];

	Tx_Data[0] = (uint8_t)status_data->soc;
	uint16_t buf = status_data->max_voltage / 10;
	uint8_t c1 = buf;
	uint8_t c2 = buf >> 8;
	Tx_Data[1] = c1;
	Tx_Data[2] = c2;
	if(status_data->air_s == true)
	Tx_Data[3] = 0;
	else
	Tx_Data[3] = 1;

	uint16_t buf1 = (uint16_t)status_data->sum_of_cells;

	Tx_Data[4]= (uint8_t)(buf1);
	Tx_Data[5]= (uint8_t)(buf1 >> 8);



	CanSend(Tx_Data, CAN_SOC);

}


