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

#define BATTERY_CAPACITY 13000 //13Ah

#define CS_PIN 4
#define CS_PIN_TYPE GPIOA

#define CELL_NUM 18	// Number of cells in one stack
#define IC_NUM 1 // Length of a daisy chain
#define GPIO_NUM 12 //GPIO + Vref

#define SPI_TIMEOUT 15000

#define NUMB_REASON_CODES	13

#define FREQUENCY 1

#define ACCU_Wh 6600

typedef struct cell_data_t
{
	uint16_t voltage;
}cell_data_t;

typedef struct temp_data_t
{
	uint16_t raw;
	int16_t temp;
}temp_data_t;



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
	uint16_t tolerance;
	float max_current;
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

	int32_t uptime;
	int8_t mode;
	float soc;
	float soc_pre;
	int32_t current;
	int32_t power;
	int32_t IVT_voltage;
	int16_t delta;

	int16_t min_temp;
	int16_t max_temp;
	uint8_t min_temp_id;
	uint8_t max_temp_id;
	uint16_t min_voltage;
	uint16_t max_voltage;
	uint8_t min_voltage_id;
	uint8_t max_voltage_id;

	float sum_of_cells;

	bool air_s;
	bool pre_s;
	bool air_pre;
	bool air_p;
	bool air_m;

	uint8_t opmode;


	bool safe_state_executed;
	uint8_t reason_code;

	int32_t IVT_U1;
	float IVT_U1_f;
	int32_t IVT_U2;
	float IVT_U2_f;
	int32_t IVT_Wh;
	float IVT_Wh_f;
	int32_t IVT_I;
	float IVT_I_f;

	float pre_percentage;

	uint32_t pec_error_counter;
	uint32_t pec_error_counter_last;
	float pec_error_average;
	uint32_t limping;
	uint8_t recieved_IVT;
}status_data_t;

//////////////////////////////////////////////////////////////////////
/*CAN bus IDs
 *
 */

#define CAN_IVT_I 0x521
#define CAN_IVT_U1 0x522
#define CAN_IVT_U2 0x523
#define CAN_IVT_Wh 0x528
#define CAN_LIMP 0x96
#define CAN_SOC 0x97


/////////////////////////////////////////////////////////////////////
/*!
	debug functionality enable/disable
 */
#define IVT							1
#define CAN_ENABLED					1
#define ERROR_COUNT_LIMIT			2 //0 = shut down on first error


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

#endif /* INC_CONF_H_ */
