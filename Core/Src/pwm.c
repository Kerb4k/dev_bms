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

#define MAX_PWM 40
/** PWM frequency in Hz */
#define PWM_FREQUENCY      25000
/** Period value of PWM output waveform */
#define PERIOD_VALUE       100
/** Initial duty cycle value */
#define INIT_DUTY_VALUE    30

extern TIM_HandleTypeDef htim8;




void fan_control(status_data_t *status_data){
	if(status_data->max_temp > 50)
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, SET);
	else
		HAL_GPIO_WritePin(GPIOA, GPIO_PIN_15, RESET);
}

void set_fan_duty_cycle(status_data_t *status_data){

	if(status_data->max_temp > 39){
		 __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, MAX_PWM);
	}
	else{
		 __HAL_TIM_SET_COMPARE(&htim8, TIM_CHANNEL_3, 18);
	}
}
