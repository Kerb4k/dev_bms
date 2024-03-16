/*
 * operation.c
 *
 *  Created on: Apr 19, 2023
 *      Author: dovlat
 */

#include "operation.h"
#include "conf.h"
#include "temp_calc.h"
#include "LTC681x.h"
#include "calculations.h"


#define RETEST_YES	1
#define RETEST_NO	0


uint8_t start_ivt[] = {0x34, 0x01, 0x1, 0x0, 0x0, 0x0, 0x0, 0x0};
/*
	Memory allocation for large data arrays and structs
*/
cell_data_t cell_data[IC_NUM][CELL_NUM];
temp_data_t temp_data[IC_NUM][GPIO_NUM]; //night should be changed to 12
uint8_t slave_cfg_tx[IC_NUM][6];
uint8_t slave_cfgb_tx[IC_NUM][6];
uint8_t slave_cfg_rx[IC_NUM][8];
uint8_t slave_cfgb_rx[IC_NUM][8];


status_data_t status_data;

limit_t limits  = {
	.max_voltage = 42000,
	.min_voltage = 31000,
	.max_charge_temp = 4400,
	.max_temp = 59,
	.min_temp = 0,
	.power = (8 * (10^6)),
	.tolerance = 100,
	.max_current = 180.0,
	.accu_min_voltage = 450.0,
	.precharge_min_start_voltage = 450.0,
	.precharge_max_end_voltage = 450.0,
	.limp_min_voltage = 34000
};

void operation_main(void){

	open_AIR();
	open_PRE();


	initialize();
	init_slave_cfg();

		status_data.pec_error_counter = 0;
		status_data.pec_error_counter_last = 0;

		status_data.limping = 0;
		status_data.recieved_IVT = 0;


		status_data.mode = 0;

		//Set Fans on
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, SET);


	while(1){


		switch (status_data.mode){
			case 0:
				core_routine(RETEST_YES);
				status_data.uptime++;

			    HAL_Delay(100);

				break;
			case 1:
				read_cell_voltage();
				read_temp_measurement();
				get_minmax_voltage(IC_NUM, cell_data, &status_data);
				get_minmax_temperature(IC_NUM, temp_data, &status_data);
				calc_sum_of_cells(IC_NUM, cell_data, &status_data);
				balance_routine();
				status_data.uptime++;
				HAL_Delay(1900);

				break;
			case 2:

				charge_routine();
				status_data.uptime++;
				break;
			case 3:
				//debug_routine();
				read_cell_voltage();
				read_temp_measurement();
				get_minmax_temperature(IC_NUM, temp_data, &status_data);
				get_minmax_voltage(IC_NUM, cell_data, &status_data);
				status_data.uptime++;
				HAL_Delay(100);

				break;
			default:
				break;
		}
	}
}

/*!
	\brief	Main routine. This function contains code that must be executed at all times.

	Following actions are performed:
		Discharge configuration register on LTC-6813 is emptied.
		Cell voltage registers of all LTC-6813s are read.
		Auxiliary voltage (temperature measurement) registers of all LTC-6813s are read.
		Sum of Cells is calculated.
		Minimum and maximum cell voltages are found.
		Minimum and maximum temperature voltages are found.
		Power is calculated.
		Duty cycle of the cooling fan is updated.
		CAN buffer for settings message is checked and processed.
		Data is checked against limits and a return value is generated.

	\return status of test_limits function (0: OK, -1 FAIL).
*/

void open_AIR(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, RESET);
	status_data.air_s = false;

}

void close_AIR(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, SET);
	status_data.air_s = true;
}

void close_PRE(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, SET);
	status_data.pre_s = true;
}

void open_PRE(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, RESET);
	status_data.pre_s = false;

}

int AMS_OK(status_data_t *status_data, limit_t *limit){
	if(status_data->min_voltage > limit->min_voltage && status_data->max_voltage < limit->max_voltage){
		if(status_data->min_temp > limit->min_temp && status_data->max_temp < limit->max_temp){
			if(status_data->recieved_IVT){
				close_AIR();
				return 0;
			}
		}
	}
	open_AIR();
	return 1;
}




void charge_routine(void){

	uint8_t RxData2[8];
		while(ReadCANBusMessage(0x96, &RxData2)){
			delay_u(200);
		}

	while(1){
		status_data.air_m = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);
		status_data.air_p = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);
		status_data.air_pre = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4);

		empty_disch_cfg();
		read_cell_voltage();
		read_temp_measurement();
		get_minmax_voltage(IC_NUM, cell_data, &status_data);
		get_minmax_temperature(IC_NUM, temp_data, &status_data);
		calc_sum_of_cells(IC_NUM, cell_data, &status_data);
		AMS_OK(&status_data, &limits);
		set_fan_duty_cycle(&status_data);

	#if IVT
		calculate_soc(&status_data);
		precharge_compare();
		calculate_soc(&status_data);
	#endif

	#if CAN_ENABLED

		Send_cell_data(cell_data);

		Send_temp_data(temp_data);
		Send_Soc(&status_data);
	#endif

		balance_routine();
		HAL_Delay(100);

	}


}




void core_routine(int32_t retest){

	status_data.air_m = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_6);
	status_data.air_p = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_7);
	status_data.air_pre = HAL_GPIO_ReadPin(GPIOB, GPIO_PIN_4);

	empty_disch_cfg();
	read_cell_voltage();
	read_temp_measurement();
	get_minmax_voltage(IC_NUM, cell_data, &status_data);
	get_minmax_temperature(IC_NUM, temp_data, &status_data);
	calc_sum_of_cells(IC_NUM, cell_data, &status_data);
	AMS_OK(&status_data, &limits);
	set_fan_duty_cycle(&status_data);

#if IVT

	calculate_soc(&status_data);
	precharge_compare();
	calculate_soc(&status_data);
#endif

#if CAN_ENABLED

	Send_cell_data(cell_data);
	Send_temp_data(temp_data);
	Send_Soc(&status_data);
	test_limp(&status_data, &limits);
#endif

}



void precharge_compare(void)
{

//TODO

	float percentage;
	float pre = status_data.IVT_U1_f;
	float air_p = status_data.IVT_U2_f;
	percentage = (air_p * 100) / pre;
	status_data.pre_percentage = percentage;
	if (status_data.safe_state_executed == 0) {
		if ((percentage >= 95) && (check_voltage_match() == true) && status_data.IVT_U1_f > limits.precharge_min_start_voltage) {
			if(status_data.pre_s == false)
			{
				uint32_t starttick = HAL_GetTick();
				while ( HAL_GetTick() - starttick < 5000 )
				{
					calculate_soc(&status_data);
					Send_Soc(&status_data);
					HAL_Delay(100);
				}
			}
			close_PRE();
		}
		else
		{
			open_PRE();
		}
	}
}

int check_voltage_match(void)
{
	float percentage;
	float accu_volt = (float)status_data.sum_of_cells;
	float post_volt = (float)status_data.IVT_U2_f;
	percentage = (post_volt * 100) / accu_volt;
	percentage = percentage - 100;

	if ((percentage < 10) && (percentage > -10)) {
		return 1;
	}
	return 0;
}

/*!
	\brief	Balance routine of AMS.

	Discharge configuration bytes in slave_cfg_tx array are written, and
	configuration is sent to LTC-6811s. After some delay configuration is
	read back to slave_cfg_rx array.
*/
void balance_routine(void)
{
	build_disch_cfg(IC_NUM, cell_data, slave_cfg_tx, &status_data, &limits);
	build_disch_cfgb(IC_NUM, cell_data, slave_cfgb_tx, &status_data, &limits);

	cfg_slaves();

}

void empty_disch_cfg(void){
	WakeUp();

	for(int i = 0; i < IC_NUM; i++){
		slave_cfg_tx[i][4] = 0x00 ;
		slave_cfg_tx[i][5] = 0x00;

		slave_cfgb_tx[i][4] = 0x00 ;
		slave_cfgb_tx[i][5] = 0x00;
	}

	cfg_slaves();
}
/*!
	\brief			Read all cell voltages from LTC-6811 daisy chain.

	Up to five consecutive reads are performed in case a CRC (PEC) check fails.

	\return			-1 on pec error, 0 on successful read.
*/
uint8_t read_cell_voltage(void){
	int8_t pec;
	WakeUp();
	adcv();
	adcv_delay();

	WakeIdle();

	for(uint8_t reg = 0; reg < 5; reg++){
		pec = rdcv(0, IC_NUM, cell_data);

		if (pec == 0) {
			return 0;
		}
		else increase_pec_counter();
	}
	goto_safe_state(PEC_ERROR);
	return -1;

}
/*!
	\brief			Read all auxiliary voltages from LTC-6811 daisy chain.

	Up to five consecutive reads are performed in case a CRC (PEC) check fails.

	\return			-1 on pec error, 0 on successful read.
*/
uint8_t read_temp_measurement(void){
	int8_t pec;
	WakeUp();
	adax();
	adax_delay();
	WakeIdle();

	for (uint8_t i = 0; i < 5; i++)	{ //for (uint8_t i = 0; i < 5; i++)	{
			 pec = rdaux(0, IC_NUM, temp_data);  // pec = ltc6804_rdaux(0, IC_NUM, temp_data);
			 temp_calc(IC_NUM, temp_data); // Moved out of 'if' to execute even on pec error
			if (pec == 0) {
				return 0;
			} else {
				increase_pec_counter();
			}
		}
		goto_safe_state(PEC_ERROR);
		return -1;

}


void init_slave_cfg(void)
{
	for (uint8_t i = 0; i < IC_NUM; i++)
	{
		slave_cfg_tx[i][0] = 0xfe;
		slave_cfg_tx[i][1] = 0x00;
		slave_cfg_tx[i][2] = 0x00;
		slave_cfg_tx[i][3] = 0x00;
		slave_cfg_tx[i][4] = 0x00;
		slave_cfg_tx[i][5] = 0x00;
	}
}

void cfg_slaves(void){
	WakeUp();
	wrcfg(IC_NUM, slave_cfg_tx);
	WakeUp();
	wrcfgb(IC_NUM, slave_cfgb_tx);
	delay_u(500);
	rdcfg(IC_NUM, slave_cfg_rx);
	rdcfgb(IC_NUM, slave_cfgb_rx);
}

void increase_pec_counter(void)
{
	status_data.pec_error_counter++;
	status_data.pec_error_average = (float)status_data.pec_error_counter / status_data.uptime;
}

void goto_safe_state(uint8_t reason)
{

	open_AIR();
	open_PRE();

}

void test_limp(status_data_t *status_data, limit_t *limit)
{

	if(status_data->min_voltage < limit->limp_min_voltage){
		status_data->limping = 1;

		uint8_t data[8];

		data[0]=8;
		data[1]=0;
		data[2]=0;
		data[3]=(uint8_t)status_data->sum_of_cells;

		data[4]=status_data->limping;
		data[5]=0;
		data[6]=0xAB;
		data[7]=0xCD;

		CanSend(data, 0x08);
		}
	else{
		status_data->limping = 0;
	}

}
