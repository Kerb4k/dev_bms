/*
 * conf.h
 *
 *  Created on: Apr 11, 2023
 *      Author: Dovlat Ibragimov
 */

#ifndef INC_CONF_H_
#define INC_CONF_H_


#define CS_PIN 4
#define CS_PIN_TYPE GPIOA

#define CELL_NUM 18	// Number of cells in one stack
#define IC_NUM 2 // Length of a daisy chain
#define GPIO_NUM 10 //GPIO + Vref

#define SPI_TIMEOUT 15000



typedef struct cell_data_t
{
	uint16_t voltage;
}cell_data_t;

typedef struct temp_data_t
{
	uint16_t raw;
	int16_t temp;
}temp_data_t;

////////////////////////////////////
/*!
	Structure containing set-points of charger
*/
typedef struct nlg_setpnt_t
{
	uint8_t ctrl;
	uint16_t mc_limit; // Max current to be drawn from mains outlet (0.1 Amp per bit. Valid values 0 Amp to 50 Amp)
	uint16_t oc_limit; // Desired battery current (0.1 Amp per bit. Valid values 0 Amp to 14 Amp)
	uint16_t ov_limit; // Desired output voltage (0.1 Volt per bit. Valid values 0 Volt to 600 Volt)
}nlg_setpnt_t;
////////////////////////////////////

/*!
	Structure containing limits
*/
typedef struct limit_t
{
	uint16_t max_voltage;
	uint16_t min_voltage;
	int16_t max_temp;
	int16_t max_charge_temp;
	int16_t min_temp;
	int32_t current;
	int32_t power;
	float max_current;
	uint16_t charger_en;			//Deprecated
	uint16_t charger_dis;			//Deprecated
	uint16_t delta;				//Deprecated
	float accu_min_voltage;
	float precharge_min_start_voltage;
	float precharge_max_end_voltage;
	uint16_t limp_min_voltage;
}limit_t;


/*!
	Main status data structure
*/
typedef struct status_data_t
{
	//uint8_t mcusr;				//Deprecated
	//uint8_t ext_status_flags;		//Deprecated
	//uint8_t settings_flags;		//Deprecated
	//uint8_t ext_settings_flags;	//Deprecated
	//uint8_t io_flags;				//Deprecated
	int32_t uptime;
	rtc_t rtc;
	int32_t current;
	int32_t power;
	//uint8_t fan1_dc;				//Deprecated
	//uint8_t fan2_dc;				//Deprecated
	int16_t min_temp;
	int16_t max_temp;
	uint8_t min_temp_id;
	uint8_t max_temp_id;
	uint16_t min_voltage;
	uint16_t max_voltage;
	uint8_t min_voltage_id;
	uint8_t max_voltage_id;
	uint16_t sum_of_cells;
	uint8_t opmode;
	uint8_t discharge_mode;
	Bool logging;
	Bool safe_state_executed;
	uint8_t reason_code;
	Bool manual_fan_dc;
	int32_t IVT_U1;
	float IVT_U1_f;
	int32_t IVT_U2;
	float IVT_U2_f;
	Bool precharge_flag;
	float pre_percentage;
	int32_t IVT_I;
	float IVT_I_f;
	uint32_t pec_error_counter;
	uint32_t pec_error_counter_last;
	float pec_error_average;
	uint32_t error_counters[NUMB_REASON_CODES];
	uint32_t limping;
	uint32_t recieved_IVT;
	uint32_t limp_counters[10]; //this 10 shouldn't be hardcoded here but it is now
}status_data_t;



#endif /* INC_CONF_H_ */
