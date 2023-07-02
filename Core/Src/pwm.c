/*
 * pwm.c
 *
 *  Created on: May 16, 2023
 *      Author: Dovlat
 */
#include "conf.h"
#include <stdint.h>
#include "stm32g4xx.h"
#include "pwm.h"
/** PWM frequency in Hz */
#define PWM_FREQUENCY      20000
/** Period value of PWM output waveform */
#define PERIOD_VALUE       100
/** Initial duty cycle value */
#define INIT_DUTY_VALUE    30

extern TIM_HandleTypeDef htim8;

void pwm_init(void){
	TIM8->CCR3 = INIT_DUTY_VALUE;
	HAL_TIMEx_PWMN_Start(&htim8, TIM_CHANNEL_3);
}


void fan_control(status_data_t *status_data){
	if(status_data->max_temp > 50)
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, SET);
	else
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, RESET);
}

void set_fan_duty_cycle(uint8_t dc, int manual_mode_bit){
	if (dc >= 100) {
			dc = 100;
		} else if (dc <= 0) {
			dc = 0;
		}

		if (manual_mode_bit == 0) {
			TIM8->CCR3 = INIT_DUTY_VALUE;
		}
}
