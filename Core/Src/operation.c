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

//extern int rtc_event_flag;

nlg_setpnt_t nlg5_ctrl = {
	.mc_limit = 160, // Max current to be drawn from mains outlet (16 Amps)
	.oc_limit = 60,  // Charging current (6 Amp)
	.ov_limit = 6000 // Charging voltage (600 Volt)
};

status_data_t status_data;

limit_t limits  = {
	.max_voltage = 42000,
	.min_voltage = 30000,
	.max_charge_temp = 4400,
	.max_temp = 57,
	.min_temp = 0,
	.power = (8 * (10^6)),
	.max_current = 180.0,
	.charger_dis = 41800,
	.charger_en = 41500,
	.tolerance = 100,
	.accu_min_voltage = 490.0,
	.precharge_min_start_voltage = 450.0,
	.precharge_max_end_voltage = 450.0,
	.limp_min_voltage = 34000
};

int charger_event_flag;
static uint8_t charger_event_counter;

void operation_main(void){

	open_AIR();
	open_PRE();

	CanSend(start_ivt, 0x411);

	initialize();
	//fan_energize();
	init_slave_cfg();

	for(uint32_t i=0; i<NUMB_REASON_CODES; i++)
		{
			status_data.error_counters[i]=0;
		}

		status_data.soc = 100;
		status_data.pec_error_counter = 0;
		status_data.pec_error_counter_last = 0;

		status_data.limping = 0;
		status_data.recieved_IVT = 0;

		status_data.opmode = 0;
		status_data.opmode = (1 << 0)|(1 << 4);

		status_data.mode = 0;

	while(1){


		switch (status_data.mode){
			case 0:
				core_routine(RETEST_YES);

			    HAL_Delay(900);

				break;
			case 1:
				read_cell_voltage();
				get_minmax_voltage(IC_NUM, cell_data, &status_data);
				balance_routine();
				HAL_Delay(2000);

				break;
			case 2:
				//charge_routine();
				break;
			case 3:
				//debug_routine();
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
}

void close_AIR(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_10, SET);
}

void close_PRE(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, SET);

}

void open_PRE(void){
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_9, RESET);

}

int AMS_OK(status_data_t *status_data, limit_t *limit){
	if(status_data->min_voltage > limit->min_voltage && status_data->max_voltage < limit->max_voltage){
		if(status_data->min_temp > limit->min_temp && status_data->max_temp < limit->max_temp){
			close_AIR();
			return 0;
		}
	}
	open_AIR();
	return 1;
}




int8_t core_routine(int32_t retest){
	empty_disch_cfg();
	read_cell_voltage();
	read_temp_measurement();
	get_minmax_voltage(IC_NUM, cell_data, &status_data);
	get_minmax_temperature(IC_NUM, temp_data, &status_data);
	calc_sum_of_cells(IC_NUM, cell_data, &status_data);
	AMS_OK(&status_data, &limits);
	fan_control(&status_data);
#if CAN_ENABLED
	Send_cell_data(cell_data);

#endif
	//calculate_power(&status_data);
	//set_fan_duty_cycle(get_duty_cycle(status_data.max_temp), status_data.manual_fan_dc);

#if IVT
	read_IVT(&status_data);
	calculate_soc(&status_data);
	precharge_compare();
#endif

	test_limp(&status_data, &limits);


	return test_limits(&status_data, &limits, retest);
}

void read_IVT(status_data_t *status_data){

	uint8_t RxData1[8];
	while(ReadCANBusMessage(0x522, &RxData1)){
		delay_u(20000);
	}

	//delay_u(500);

	status_data->IVT_U1 = (uint32_t)(RxData1[5] | (RxData1[4] << 8) | (RxData1[3] << 16) | (RxData1[2] << 24) );
	status_data->IVT_U1_f = status_data->IVT_U1 / 1000.0f;
	uint8_t RxData2[8];
	while(ReadCANBusMessage(0x523, &RxData2)){
		delay_u(20000);
	}
	//delay_u(500);
	status_data->IVT_U2 = (uint32_t)(RxData2[5] | (RxData2[4] << 8) | (RxData2[3] << 16) | (RxData2[2] << 24) );
	status_data->IVT_U2_f = status_data->IVT_U2 / 1000.0f;
	uint8_t RxData3[8];
	while(ReadCANBusMessage(0x528, &RxData3)){
		delay_u(20000);
	}
	//delay_u(500);
	status_data->IVT_Wh = (uint32_t)(RxData3[5] | (RxData3[4] << 8) | (RxData3[3] << 16) | (RxData3[2] << 24) );
	status_data->IVT_Wh_f = status_data->IVT_Wh / 1000.0f;

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
			close_PRE();
		}
		/*else
		{
			open_PRE();
		}*/
		/*else {
			open_PRE(); //it is maybe not a great idea to actively open the precharge without opening all the contactors
		}*/
	} /*else {
		goto_safe_state(IVT_LOST);
	}*/ //this was maybe not working as intended
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
	// TODO build_disch_cfg(IC_NUM, cell_data, slave_cfg_tx, &status_data, &limits);
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
	wrcfgb(IC_NUM, slave_cfgb_tx); //TODO
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
#if BMS_RELAY_CTRL_BYPASS
	// Do nothing.
#else
#if SKIP_PEC_ERROR_ACTIONS
	if (reason != PEC_ERROR)
	{
		open_AIR();
		open_PRE();
	}
#else
	open_AIR();
	open_PRE();
#endif
#endif

#if STOP_CORE_ON_SAFE_STATE
	status_data.opmode &= ~(1 << 0);
#endif

#if START_DEBUG_ON_SAFE_STATE
	status_data.opmode |= (1 << 3);
#endif

	status_data.safe_state_executed = true;
	status_data.reason_code = reason;
}

int32_t test_limp(status_data_t *status_data, limit_t *limit)
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

/*#if TEST_UNDERVOLTAGE
	if(status_data->min_voltage < limit->limp_min_voltage)
	{
		if(!(status_data->limp_counters[0]<=LIMP_COUNT_LIMIT))
		{
			status_data->limping = 1;
			status_data->limp_counters[0]+=10; //we have this here so it takes a bit of time for bms to exit limp mode once it enters


		}
		else
		{
			status_data->limp_counters[0]++;
		}
	}
	else if (status_data->limp_counters[0]>0)
	{
		status_data->limp_counters[0]--;
		if(status_data->limp_counters[0]==0)
		{
			status_data->limping = 0;
		}
	}
#endif*/
}

/*!
	\brief	Charger current limit and enabled state is set.
	\todo	Implement a proper charging algorithm.
*/

void set_charge_current(void)
{
	//nlg5_ctrl.oc_limit = 120; //DONT SET IT HERE

	if (status_data.max_voltage > limits.charger_dis) {
		nlg5_ctrl.ctrl = 0;
	} else {
		if (status_data.max_voltage < limits.charger_en) {
			nlg5_ctrl.ctrl = NLG5_C_C_EN;
		}
	}
}

/*!
	\brief	Send charger command message on CAN bus.

	Every fifth time charger_event_flag is set a reset command is sent,
	if charger is in fault state. Otherwise a charge command is sent.
*/
void set_charger(void){
	if(charger_event_flag){
		if (((nlg5a_buffer[0] == 136) || (nlg5a_buffer[0] == 152)) && ((nlg5b_buffer[0] == 136) || (nlg5b_buffer[0] == 152))) {
	}
}



int8_t test_limits(status_data_t *status_data, limit_t *limit, int32_t retest)
{
		//MAYBE WE DON'T WANT 50% ERRORS TO BE ALLOWED
#if TEST_OVERVOLTAGE
	if (status_data->max_voltage > limit->max_voltage)
	{
		if(!(status_data->error_counters[OVERVOLTAGE]<=ERROR_COUNT_LIMIT && retest))
		{
			goto_safe_state(OVERVOLTAGE);
			return -1;
		}
		else
		{
			status_data->error_counters[OVERVOLTAGE]++;
		}
	}
	else if (status_data->error_counters[OVERVOLTAGE]>0)
	{
		status_data->error_counters[OVERVOLTAGE]--;
	}
#endif

#if TEST_UNDERVOLTAGE
	if (status_data->min_voltage < limit->min_voltage)
	{
		if(!(status_data->error_counters[UNDERVOLTAGE]<=ERROR_COUNT_LIMIT && retest))
		{
			goto_safe_state(UNDERVOLTAGE);
			return -1;
		}
		else
		{
			status_data->error_counters[UNDERVOLTAGE]++;
		}
	}
	else if (status_data->error_counters[UNDERVOLTAGE]>0)
	{
		status_data->error_counters[UNDERVOLTAGE]--;
	}
#endif

#if TEST_OVERTEMPERATURE
	if (status_data->max_temp > limit->max_temp)
	{
		if(!(status_data->error_counters[OVERTEMP]<=ERROR_COUNT_LIMIT && retest))
		{
			goto_safe_state(OVERTEMP);
			return -1;
		}
		else
		{
			status_data->error_counters[OVERTEMP]++;
		}
	}
	else if (status_data->error_counters[OVERTEMP]>0)
	{
		status_data->error_counters[OVERTEMP]--;
	}
#endif

#if TEST_OVERTEMPERATURE_CHARGING
	if(status_data->opmode&(1<<2) && (status_data->max_temp > limit->max_charge_temp))
	{
		if(!(status_data->error_counters[OVERTEMP_CHARGING]<=ERROR_COUNT_LIMIT && retest))
		{
			goto_safe_state(OVERTEMP_CHARGING);
			return -1;
		}
		else
		{
			status_data->error_counters[OVERTEMP_CHARGING]++;
		}
	}
	else if (status_data->error_counters[OVERTEMP_CHARGING]>0)
	{
		status_data->error_counters[OVERTEMP_CHARGING]--;
	}
#endif

#if TEST_UNDERTEMPERATURE
	if (status_data->min_temp < limit->min_temp)
	{
		if(!(status_data->error_counters[UNDERTEMP]<=ERROR_COUNT_LIMIT && retest))
		{
			goto_safe_state(UNDERTEMP);
			return -1;
		}
		else
		{
			status_data->error_counters[UNDERTEMP]++;
		}
	}
	else if (status_data->error_counters[UNDERTEMP]>0)
	{
		status_data->error_counters[UNDERTEMP]--;
	}
#endif

#if TEST_OVERPOWER
	if (status_data->power > limit->power)
	{
		if(!(status_data->error_counters[OVERPOWER]<=ERROR_COUNT_LIMIT && retest))
		{
			goto_safe_state(OVERPOWER);
			return -1;
		}
		else
		{
			status_data->error_counters[OVERPOWER]++;
		}
	}
	else if (status_data->error_counters[OVERPOWER]>0)
	{
		status_data->error_counters[OVERPOWER]--;
	}
#endif

#if TEST_OVERCURRENT
if (status_data->IVT_I_f > limit->max_current)
{
	if(!(status_data->error_counters[OVERCURR]<=ERROR_COUNT_LIMIT && retest))
	{
		goto_safe_state(OVERCURR);
		return -1;
	}
	else
	{
		status_data->error_counters[OVERCURR]++;
	}
}
else if (status_data->error_counters[OVERCURR]>0)
{
	status_data->error_counters[OVERCURR]--;
}
#endif

#if TEST_ACCU_UNDERVOLTAGE
	if (status_data->IVT_U2_f < limit->accu_min_voltage)
	{
		if(!(status_data->error_counters[ACCU_UNDERVOLTAGE]<=ERROR_COUNT_LIMIT && retest))
		{
			goto_safe_state(ACCU_UNDERVOLTAGE);
			return -1;
		}
		else
		{
			status_data->error_counters[ACCU_UNDERVOLTAGE]++;
		}
	}
	else if (status_data->error_counters[ACCU_UNDERVOLTAGE]>0)
	{
		status_data->error_counters[ACCU_UNDERVOLTAGE]--;
	}
#endif

#if IVT_TIMEOUT
	if (status_data->recieved_IVT != 1 )
	{
		if(!(status_data->error_counters[IVT_LOST]<=ERROR_COUNT_LIMIT_LOST && retest))
		{
			goto_safe_state(IVT_LOST);
			return -1;
		}
		else
		{
			status_data->error_counters[IVT_LOST]++;
		}
	}
	else
	{
		status_data->recieved_IVT = 0;

		if((status_data->error_counters[ACCU_UNDERVOLTAGE]>0))
		{
			status_data->error_counters[IVT_LOST]--;
		}
	}
#endif

	return 0;
}
