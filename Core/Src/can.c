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

    while(t < 101)
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
            else
            {
                return 1; // Message Identifier did not match
            }
        }
        // Else, ignore the error and try again
    }
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



			cell_id = i * 18 + j;
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

			buf = temp_data[i][2.temp;
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
	uint8_t TxData[8];
	TxData[0] = (uint8_t)status_data->soc;
	TxData[1] = 0;
	TxData[2] = 0;
	TxData[3] = 0;
	TxData[4] = 0;
	TxData[5] = 0;
	TxData[6] = 0;
	TxData[7] = 0;
	CanSend(TxData, CAN_SOC);
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





