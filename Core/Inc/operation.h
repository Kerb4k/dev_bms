/*
 * operation.h
 *
 *  Created on: Apr 19, 2023
 *      Author: dovlat
 */

#ifndef INC_OPERATION_H_
#define INC_OPERATION_H_

#include "conf.h"


void operation_main(void);
int8_t core_routine(int32_t retest);
void balance_routine(void);
void charge_routine(void);
void debug_routine(void);
void datalog_routine(void);
void dis_check(void);
void ltc_init_cfg(uint8_t total_ic, uint8_t tx_config[][6]);
void adcv_delay(void);
void adax_delay(void);
uint8_t read_cell_voltage(void);
uint8_t read_temp_measurement(void);
uint8_t read_current_data(void);
void goto_safe_state(uint8_t reason);
void close_AIR(void);
void open_AIR(void);
int32_t test_limp(status_data_t *status_data, limit_t *limit);
int8_t test_limits(status_data_t *status_data, limit_t *limit, int32_t retest);
void set_charge_current(void);
void set_charger(void);
void fan_energize(void);
void logging_init(void);
void cfg_slaves(void);
void empty_disch_cfg(void);
void init_slave_cfg(void);
void precharge_compare(void);
void close_PRE(void);
void open_PRE(void);
int check_voltage_match(void);
void increase_pec_counter(void);

#endif /* INC_OPERATION_H_ */
