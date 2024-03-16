/*
 * calculations.h
 *
 *  Created on: May 2, 2023
 *      Author: dovlati
 */

#ifndef INC_CALCULATIONS_H_
#define INC_CALCULATIONS_H_

#include "conf.h"

void calc_sum_of_cells(uint8_t total_ic, cell_data_t cell_data[][CELL_NUM], status_data_t *status_data);
void get_minmax_temperature(uint8_t total_ic, temp_data_t temp_data[][GPIO_NUM], status_data_t *status_data);
void get_minmax_voltage(uint8_t total_ic, cell_data_t cell_data[][CELL_NUM], status_data_t *status_data);
void calculate_soc(status_data_t *status_data);
void build_disch_cfg(uint8_t total_ic, cell_data_t cell_data[][CELL_NUM], uint8_t tx_config[][6],\
status_data_t *status_data, limit_t *limit);
void mute( uint8_t tx_configb[][6]); //TODO check what this is
void build_disch_cfgb(uint8_t total_ic, cell_data_t cell_data[][CELL_NUM], uint8_t tx_config[][6],\
status_data_t *status_data, limit_t *limit);
#endif /* INC_CALCULATIONS_H_ */
