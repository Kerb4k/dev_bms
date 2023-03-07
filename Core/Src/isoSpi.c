#include <stdint.h>
#include "isoSpi.h"
#include "stm32g4xx.h"
#include <stdio.h>


#define gpio_pin GPIO_PIN_4
#define gpio_type GPIOA



void cs_low(){
	HAL_GPIO_WritePin(gpio_type, gpio_pin, 0);
}

void cs_high(){
	HAL_GPIO_WritePin(gpio_type, gpio_pin, 1);
}


///////////////////////////////////
// library https://github.com/keatis/dwt_delay/
void delay_u(uint32_t microseconds)// microseconds
{
	 CoreDebug->DEMCR |= CoreDebug_DEMCR_TRCENA_Msk;
	    DWT->CTRL |= DWT_CTRL_CYCCNTENA_Msk;
	    // Get the current number of clock cycles
	    uint32_t const startTicks = DWT->CYCCNT;
	    // Calculate the number of clock cycles for the desired delay
	    uint32_t const delayTicks = (SystemCoreClock / 1000000) * microseconds;
	    // Wait until the number of clock cycles has elapsed
	    while (DWT->CYCCNT - startTicks < delayTicks);
}
//////////////////////////////////

void delay_m(uint16_t milli)
{
  HAL_Delay(milli);
}
/*
Writes an array of bytes out of the SPI port
*/
void spi_write_array(uint8_t len, // Option: Number of bytes to be written on the SPI port
                     uint8_t data[], //Array of bytes to be written on the SPI port
					SPI_HandleTypeDef *hspi
					)
{
	HAL_SPI_Transmit(hspi, data,len,1000); // might be changed in future

}

void spi_write_read(uint8_t tx_Data[],//array of data to be written on SPI port
        uint8_t tx_len, //length of the tx data arry
        uint8_t *rx_data,//Input: array that will store the data read by the SPI port
        uint8_t rx_len ,//Option: number of bytes to be read from the SPI port
		SPI_HandleTypeDef * hspi //spi reference
       ){

	HAL_GPIO_WritePin(gpio_type, gpio_pin, 0);

	HAL_SPI_Transmit(hspi, tx_Data, tx_len, 1000);
	HAL_SPI_Receive(hspi, rx_data, rx_len, 1000);

	HAL_GPIO_WritePin(gpio_type, gpio_pin, 1);

}


uint8_t spi_read_byte(uint8_t tx_dat,SPI_HandleTypeDef * hspi)
{
	uint8_t data;

	HAL_GPIO_WritePin(gpio_type, gpio_pin, 0);
	HAL_SPI_Receive(hspi, &data, 1, 500);
	HAL_GPIO_WritePin(gpio_type, gpio_pin, 1);

	return data;
}
