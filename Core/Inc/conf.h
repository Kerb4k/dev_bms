/*
 * conf.h
 *
 *  Created on: Apr 11, 2023
 *      Author: Dovlat Ibragimov
 */

// File is used for BMS configuration

#ifndef INC_CONF_H_
#define INC_CONF_H_
#include <stdint.h>
#include "stdbool.h"
#include "stm32g4xx_hal.h"



#define CS_PIN 4
#define CS_PIN_TYPE GPIOA

#define CELL_NUM 18	// Number of cells in one stack
#define IC_NUM 1 // Length of a daisy chain
#define GPIO_NUM 12 //GPIO + Vref

#define SPI_TIMEOUT 15000

#define NUMB_REASON_CODES	13

#define FREQUENCY 1


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
	uint16_t tolerance;				//Deprecated
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
	int8_t mode;
	//rtc_t rtc;
	float time;
	float time_prev;
	float soc;
	float soc_pre;
	int32_t current;
	int32_t power;
	int32_t IVT_voltage;
	int16_t delta;
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
	bool logging;
	bool safe_state_executed;
	uint8_t reason_code;
	bool manual_fan_dc;
	int32_t IVT_U1;
	float IVT_U1_f;
	int32_t IVT_U2;
	float IVT_U2_f;
	int32_t IVT_Wh;
	float IVT_Wh_f;
	bool precharge_flag;
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

//////////////////////////////////////////////////////////////////////

/*!
	debug functionality enable/disable
 */
#define IVT							1
#define CAN_DEBUG					1
#define CAN_ENABLED					1
#define FAN_DEBUG					1
#define BMS_RELAY_CTRL_BYPASS		0
#define STOP_CORE_ON_SAFE_STATE		0
#define START_DEBUG_ON_SAFE_STATE	1
#define BYPASS_INITIAL_CHECK		0
#define SKIP_PEC_ERROR_ACTIONS		0
#define ERROR_COUNT_LIMIT			2 //0 = shut down on first error
#define ERROR_COUNT_LIMIT_LOST		1
#define LIMP_COUNT_LIMIT			2
/*!
	Board connection definitions
 */

#define PRE				J9_PIN4_5V
#define AIR				J8_PIN4_5V
#define FAN_24			J3_PIN1_24V
//#define FAN_PWM			J3_PIN4_5V
/*!
	Test enable/disable
 */
#define TEST_OVERVOLTAGE					1
#define TEST_UNDERVOLTAGE					1
#define TEST_OVERTEMPERATURE				1
#define TEST_UNDERTEMPERATURE				1
#define TEST_OVERPOWER						0
#define TEST_ACCU_UNDERVOLTAGE				0 //this is for testing undervoltage with IVT
#define CHECK_IVT							1 //to completely disable IVT TEST_ACCU_UNDERVOLTAGE needs to be set to 0
#define TEST_OVERTEMPERATURE_CHARGING		0
#define TEST_OVERCURRENT					0
#define IVT_TIMEOUT							1

/*!
	Reason codes for entering safe-state
 */
#define UNDEFINED				0
#define OVERVOLTAGE				1
#define UNDERVOLTAGE			2
#define OVERTEMP				3
#define UNDERTEMP				4
#define OVERCURR				5
#define OVERPOWER				6
#define EXTERNAL				7
#define PEC_ERROR				8
#define ACCU_UNDERVOLTAGE		9
#define IVT_LOST				10
#define OVERTEMP_CHARGING		11

/* Bit definitions in NLG5 Control Bitmap (NLG5_CTLB) */
#define NLG5_C_C_EN				(1<<7)
#define NLG5_C_C_EL				(1<<6)
#define NLG5_C_CP_V				(1<<5)
//////////////////////////////////////////////////////////////////////

#define BATTERY_CAPACITY 13000 //13Ah
#endif /* INC_CONF_H_ */
