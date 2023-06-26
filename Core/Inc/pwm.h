/*
 * pwm.h
 *
 *  Created on: May 16, 2023
 *      Author: Dovlat
 */

#ifndef SRC_PWM_H_
#define SRC_PWM_H_



void pwm_init(void);
void set_fan_duty_cycle(uint8_t dc, int manual_mode_bit);
void fan_control(status_data_t *status_data);

#endif /* SRC_PWM_H_ */
