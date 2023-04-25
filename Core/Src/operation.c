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

#define RETEST_YES	1
#define RETEST_NO	0

/*
	Memory allocation for large data arrays and structs
*/
cell_data_t cell_data[IC_NUM][CELL_NUM];
temp_data_t temp_data[IC_NUM][GPIO_NUM]; //night should be changed to 12
uint8_t slave_cfg_tx[IC_NUM][6];
uint8_t slave_cfg_rx[IC_NUM][8];
extern Bool rtc_event_flag;
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
	.max_temp = 5900,
	.min_temp = -1500,
	.power = (8 * (10^6)),
	.max_current = 180.0,
	.charger_dis = 41800,
	.charger_en = 41500,
	.delta = 100,
	.accu_min_voltage = 490.0,
	.precharge_min_start_voltage = 470.0,
	.precharge_max_end_voltage = 450.0,
	.limp_min_voltage = 34000
};

int charger_event_flag;
static uint8_t charger_event_counter;

void operation_main(void){

	//initial setting of AIR and PRECHARGE pins
	open_AIR();
	open_PRE();

	//System initialization
	initialize();
	fan_energize();
	init_slave_cfg();

#if CAN_ENABLED
// can initialization
#endif

	for(uint32_t i=0; i<NUMB_REASON_CODES; i++)
		{
			status_data.error_counters[i]=0;
		}

	status_data.pec_error_counter = 0;
	status_data.pec_error_counter_last = 0;

	status_data.limping = 0;
	status_data.recieved_IVT = 0; // might need to be deleted

	status_data.opmode = 0;
	status_data.opmode = (1 << 0)|(1 << 4); //17
	status_data.logging = true; //Always true

#if BYPASS_INITIAL_CHECK
	close_AIR();
	delay_ms(5000);
	close_PRE();
#else
	if (core_routine(RETEST_NO) == 0) {	// Initial check before closing AIRs
		delay_m(1000);
		close_AIR();
#if !CHECK_IVT
		delay_ms(5000);
		close_PRE();
#endif
	}
#endif

	while(1){

#if CAN_ENABLED
		//can_check_opmode_setting(); // can check
#endif
		delay_m(100);

		/*	*/
		// TODO operation statments
		/*	*/

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

int8_t core_routine(int32_t retest){
	empty_disch_cfg();
	read_cell_voltage();
	read_temp_measurement();
	/* TODO
	 * calc_sum_of_cells(IC_NUM, cell_data, &status_data);
	 * get_minmax_voltage(IC_NUM, cell_data, &status_data);
	 * get_minmax_temperature(IC_NUM, temp_data, &status_data);
	 * calculate_power(&status_data);
	 * set_fan_duty_cycle(get_duty_cycle(status_data.max_temp), status_data.manual_fan_dc);
	 * and etc. look in NIK object
	 */
	return test_limits(&status_data, &limits, retest);
}
void precharge_compare(void)
{
	//TODO recheck
	float percentage;
	float pre = status_data.IVT_U1_f;
	float air_p = status_data.IVT_U2_f;
	percentage = (pre * 100) / air_p;
	status_data.pre_percentage = percentage;
	if (status_data.safe_state_executed == 0) {
		if ((percentage >= 93) && (check_voltage_match() == true) && status_data.IVT_U1_f > limits.precharge_min_start_voltage && status_data.IVT_U1_f) {
			close_PRE();
		}
		else if(status_data.IVT_U1_f < limits.precharge_max_end_voltage || status_data.IVT_U1_f < limits.precharge_max_end_voltage)
		{
			open_PRE();
		}
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
	cfg_slaves();

}

