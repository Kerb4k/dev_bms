

#include <stdint.h>
#include "isoSpi.h"
#include "stm32g4xx.h"
#include <stdio.h>
#include "conf.h"

extern SPI_HandleTypeDef hspi1;


void delay_u(uint32_t us){
	CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
		    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
		    // Get the current number of clock cycles
		    uint32_t const startTicks = DWT->CYCCNT;
		    // Calculate the number of clock cycles for the desired delay
		    uint32_t const delayTicks = (SystemCoreClock / 1000000) * us;
		    // Wait until the number of clock cycles has elapsed
		    while (DWT->CYCCNT - startTicks < delayTicks);
}

void delay_m(uint32_t ms){
	HAL_Delay(ms);
}

uint8_t spi_write_read_byte(uint8_t wbyte){

	uint8_t rxByte;

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);
	HAL_SPI_TransmitReceive(&hspi1, &wbyte, &rxByte, 1, SPI_TIMEOUT);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 1);

	return rxByte;
}

uint32_t spi_write_array(uint8_t len, uint8_t *data){

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);
	HAL_SPI_Transmit(&hspi1, data, len, SPI_TIMEOUT);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 1);

	return 0;
}

uint32_t spi_write_then_read_array_ltc(uint8_t wlen, uint8_t *wbuffer, uint8_t rlen, uint8_t *rbuffer){

	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 0);
	HAL_SPI_Transmit(&hspi1, wbuffer, wlen, SPI_TIMEOUT);
	HAL_SPI_Receive(&hspi1, rbuffer, rlen, SPI_TIMEOUT);
	HAL_GPIO_WritePin(GPIOA, GPIO_PIN_4, 1);

	return 0;

}
