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

extern status_data_t status_data;


extern uint8_t               CAN_TxData[8];
extern uint8_t               CAN_RxData[8];

uint8_t nlg5a_buffer[4];
uint8_t nlg5b_buffer[4];



#define ERR_CANOFFLINE				11
uint8_t canSendErrorFlag;

void read_IVT_I(){
	status_data.recieved_IVT = 0;

	status_data.IVT_I = (uint32_t)(CAN_RxData[5] | (CAN_RxData[4] << 8) | (CAN_RxData[3] << 16) | (CAN_RxData[2] << 24) );
	status_data.IVT_I_f = status_data.IVT_I / 1000.0f;
}

void read_IVT_U1(){
	status_data.recieved_IVT = 0;

	status_data.IVT_U1 = (uint32_t)(CAN_RxData[5] | (CAN_RxData[4] << 8) | (CAN_RxData[3] << 16) | (CAN_RxData[2] << 24) );
	status_data.IVT_U1_f = status_data.IVT_U1 / 1000.0f;
}

void read_IVT_U2(){
	status_data.recieved_IVT = 0;

	status_data.IVT_U2 = (uint32_t)(CAN_RxData[5] | (CAN_RxData[4] << 8) | (CAN_RxData[3] << 16) | (CAN_RxData[2] << 24) );
	status_data.IVT_U2_f = status_data.IVT_U2 / 1000.0f;
}

void read_IVT_Wh(){
	status_data.recieved_IVT = 0;

	status_data.IVT_Wh = (uint32_t)(CAN_RxData[5] | (CAN_RxData[4] << 8) | (CAN_RxData[3] << 16) | (CAN_RxData[2] << 24) );
	status_data.IVT_Wh_f = status_data.IVT_Wh / 1000.0f;
}





void HAL_FDCAN_RxFifo0Callback(FDCAN_HandleTypeDef *hfdcan, uint32_t RxFifo0ITs)
{
  if((RxFifo0ITs & FDCAN_IT_RX_FIFO0_NEW_MESSAGE) != RESET)
  {
    /* Retreive Rx messages from RX FIFO0 */
    if (HAL_FDCAN_GetRxMessage(hfdcan, FDCAN_RX_FIFO0, &RxHeader, CAN_RxData) != HAL_OK)
    {
    /* Reception Error */
    Error_Handler();
    }
    else{
    	switch(RxHeader.Identifier){
    	case CAN_IVT_I:
			read_IVT_I();
    		break;
    	case CAN_IVT_U1:
			read_IVT_U1();
			break;
		case CAN_IVT_U2:
			read_IVT_U2();
			break;
		case CAN_IVT_Wh:
			read_IVT_Wh();
			break;

	

    	}
    }

    if (HAL_FDCAN_ActivateNotification(hfdcan, FDCAN_IT_RX_FIFO0_NEW_MESSAGE, 0) != HAL_OK)
    {
      /* Notification Error */
      Error_Handler();
    }


  }
}


void CanSend(uint8_t *TxData, uint32_t identifier ){

	TxHeader.Identifier = identifier;


	while(HAL_FDCAN_GetTxFifoFreeLevel(&hfdcan1) != 0 && HAL_FDCAN_AddMessageToTxFifoQ(&hfdcan1, &TxHeader, TxData) != HAL_OK){
		delay_u(10);
	}

}

int ReadCANBusMessage(uint32_t messageIdentifier, uint8_t* RxData1)
{
    /* Infinite loop to keep trying to read the message */
	uint32_t t = 0;

    while(t < 4294967295)
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


			uint8_t TxData[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, i, j};

			uint32_t id_c = 0x778;
			CanSend(TxData, id_c);
			delay_u(100);
		}
	}
}

#define TEMP_FIXED 1
void Send_temp_data(temp_data_t temp_data[][GPIO_NUM]){

#if TEMP_FIXED

		for(int i = 0; i < IC_NUM; i++){
			uint32_t id_t = 1960;

						uint16_t buf = temp_data[i][0].temp;
						uint16_t buf2 = temp_data[i][1].temp;
						uint16_t buf3 = temp_data[i][2].temp;

						uint8_t c1_1 = buf;
						uint8_t c2_1 = buf >> 8;

						uint8_t c1_2 = buf2;
						uint8_t c2_2 = buf2 >> 8;

						uint8_t c1_3 = buf3;
						uint8_t c2_3 = buf3 >> 8;

						uint8_t TxData[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, i, 0};

						CanSend(TxData, id_t);
						delay_u(100);

						uint16_t buf4 = temp_data[i][3].temp;
						uint16_t buf5 = temp_data[i][4].temp;
						uint16_t buf6 = temp_data[i][6].temp;

						uint8_t c1_4 = buf4;
						uint8_t c2_4 = buf4 >> 8;

						uint8_t c1_5 = buf5;
						uint8_t c2_5 = buf5 >> 8;

						uint8_t c1_6 = buf6;
						uint8_t c2_6 = buf6 >> 8;

						uint8_t TxData1[8] = { c1_4, c2_4, c1_5, c2_5 ,c1_6, c2_6, i, 1};

						CanSend(TxData1, id_t);
						delay_u(100);

						uint16_t buf7 = temp_data[i][7].temp;
						uint16_t buf8 = temp_data[i][8].temp;
						uint16_t buf9 = temp_data[i][9].temp;

						uint8_t c1_7 = buf7;
						uint8_t c2_7 = buf7 >> 8;

						uint8_t c1_8 = buf8;
						uint8_t c2_8 = buf8 >> 8;

						uint8_t c1_9 = buf9;
						uint8_t c2_9 = buf9 >> 8;


						uint8_t TxData2[8] = { c1_7, c2_7, c1_8, c2_8 ,c1_9, c2_9, i, 2};
						CanSend(TxData2, id_t);
						delay_u(100);
		}
#else

	for(int i = 0; i < IC_NUM; i++){

			uint32_t id_t = 1960;

			uint16_t buf = temp_data[i][0].temp;
			uint16_t buf2 = temp_data[i][1].temp;
			uint16_t buf3 = temp_data[i][2].temp;

			uint8_t c1_1 = buf;
			uint8_t c2_1 = buf >> 8;

			uint8_t c1_2 = buf2;
			uint8_t c2_2 = buf2 >> 8;

			uint8_t c1_3 = buf3;
			uint8_t c2_3 = buf3 >> 8;

			uint8_t TxData[8] = { c1_1, c2_1, c1_2, c2_2 ,c1_3, c2_3, i, 0};

			CanSend(TxData, id_t);
			delay_u(100);

			uint16_t buf4 = temp_data[i][3].temp;
			uint16_t buf5 = temp_data[i][4].temp;
			uint16_t buf6 = temp_data[i][3].temp;

			uint8_t c1_4 = buf4;
			uint8_t c2_4 = buf4 >> 8;

			uint8_t c1_5 = buf5;
			uint8_t c2_5 = buf5 >> 8;

			uint8_t c1_6 = buf6;
			uint8_t c2_6 = buf6 >> 8;

			uint8_t TxData1[8] = { c1_4, c2_4, c1_5, c2_5 ,c1_6, c2_6, i, 1};

			CanSend(TxData1, id_t);
			delay_u(100);

			uint16_t buf7 = temp_data[i][2].temp;
			uint16_t buf8 = temp_data[i][1].temp;
			uint16_t buf9 = temp_data[i][0].temp;

			uint8_t c1_7 = buf7;
			uint8_t c2_7 = buf7 >> 8;

			uint8_t c1_8 = buf8;
			uint8_t c2_8 = buf8 >> 8;

			uint8_t c1_9 = buf9;
			uint8_t c2_9 = buf9 >> 8;


			uint8_t TxData2[8] = { c1_7, c2_7, c1_8, c2_8 ,c1_9, c2_9, i, 2};
			CanSend(TxData2, id_t);
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

	uint16_t buf2 = (uint16_t)status_data->max_temp;

	Tx_Data[6]= (uint8_t)(buf2);
	Tx_Data[7]= (uint8_t)(buf2 >> 8);

	CanSend(Tx_Data, CAN_SOC);

}


