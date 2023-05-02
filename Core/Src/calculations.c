/*
 * calculations.c
 *
 *  Created on: May 2, 2023
 *      Author: dovlati
 */
#include "conf.h"

/*!
	\brief	Calculates the Sum of Cells.
*/
void calc_sum_of_cells(uint8_t total_ic, cell_data_t cell_data[][CELL_NUM], status_data_t *status_data)
{

}
/*!
	\brief	Calculate power from current data and Sum of Cells.
*/
void calculate_power(status_data_t *status_data)
{

}


/*!
	Loops though all temp values and finds the highest and lowest temp values and corresponding
	cell IDs

	\param total_ic				Number of ltc6804 ICs in stack
	\param cell_data[][12]		Cell data array
	\param *status_data			Pointer to status_data struct
*/
void get_minmax_temperature(uint8_t total_ic, temp_data_t temp_data[][GPIO_NUM], status_data_t *status_data)
{
	int32_t min = temp_data[0][0].temp;
	int32_t max = temp_data[0][0].temp;
	uint8_t min_id = 0;
	uint8_t max_id = 0;

	for (uint8_t i = 0; i < total_ic; i++)
	{
		for (uint8_t j = 0; j < GPIO_NUM; j++)
		{

			if (temp_data[i][j].temp > max)
			{

				max = temp_data[i][j].temp;
				max_id = (i << 4) | (j & 0x0f);

			}
			if (temp_data[i][j].temp < min)
			{

				min = temp_data[i][j].temp;
				min_id = (i << 4) | (j & 0x0f);

			}
		}
	}
	status_data->min_temp = min;
	status_data->max_temp = max;
	status_data->min_temp_id = min_id;
	status_data->max_temp_id = max_id;
}

/*!
	Loops though all cell values and finds the highest and lowest values and corresponding
	cell IDs

	\param total_ic				Number of ltc6804 ICs in stack
	\param cell_data[][CELL_NUM]		Cell data array
	\param *status_data			Pointer to status_data struct
*/
void get_minmax_voltage(uint8_t total_ic, cell_data_t cell_data[][CELL_NUM], status_data_t *status_data)
{
	uint16_t min = cell_data[0][0].voltage;
	uint16_t max = cell_data[0][0].voltage;
	uint8_t min_id = 0;
	uint8_t max_id = 0;

	for (uint8_t i = 0; i < total_ic; i++)
	{
		for (uint8_t j = 0; j < CELL_NUM; j++)
		{
			if (cell_data[i][j].voltage > max)
			{
				max = cell_data[i][j].voltage;
				max_id = (i << 4) | (j & 0x0f);
			}
			if (cell_data[i][j].voltage < min && cell_data[i][j].voltage > 5000) //ignore cells under .5V
			{
				if( i==2 && j==11 )
				{
					//remove this if when working stack is put in car
				}
				else
				{
					min = cell_data[i][j].voltage;
					min_id = (i << 4) | (j & 0x0f);
				}
			}
		}
	}
	status_data->min_voltage = min;
	status_data->max_voltage = max;
	status_data->min_voltage_id = min_id;
	status_data->max_voltage_id = max_id;
}

/*!
	\brief Returns the duty cycle of fan based on temperature input.
*/
uint8_t get_duty_cycle(int16_t temperature){
}

/*!
	Parses data in cell_data array

	\param total_ic	Number of measurement ICs connected

	\return void
*/
void build_disch_cfg(uint8_t total_ic, cell_data_t cell_data[][CELL_NUM], uint8_t tx_config[][6],\
					 status_data_t *status_data, limit_t *limit){
	uint16_t DCCx = 0x0000;
	for (uint8_t i = 0; i < total_ic; i++){ //discharge all cells
		DCCx = 0x0FFF; // Set all discharge bits to 1
		tx_config[i][4] = (DCCx & 0x00FF);
		tx_config[i][5] = ((DCCx >> 8) & 0x0F);
	}
}

void build_disch_cfgb(uint8_t total_ic, cell_data_t cell_data[][CELL_NUM], uint8_t tx_config[][6],\
status_data_t *status_data, limit_t *limit)
{
	uint16_t DCCx = 0x0000;


			for (uint8_t i = 0; i < total_ic; i++)
			{
				tx_config[i][0] = 0x00;
				tx_config[i][1] = 0x00;
				tx_config[i][2] = 0x00;
				tx_config[i][3] = 0x00;
				tx_config[i][4] = 0x00;
				tx_config[i][5] = 0x00;

				DCCx = 0x0FFF; // Set all discharge bits to 1
				tx_config[i][0] = (DCCx & 0x00FF);
				tx_config[i][1] = 0x03;
			}


}
