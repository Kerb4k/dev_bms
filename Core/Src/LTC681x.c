/*
 * LTC&(!X.c
 *
 *  Created on: Apr 11, 2023
 *      Author: Dovlat Ibragimov
 */


#include "LTC681X.h"
#include <stdint.h>
#include "conf.h"

#define T_WAKE_MAX		400
#define T_REFUP_MAX		4400
#define T_CYCLE_FAST_MAX	1185	// Measure 12 Cells

void WakeIdle(void)
{
	spi_write_read_byte(0xFF);
	delay_u(IC_NUM * T_READY);
}

void WakeUp(void){
	spi_write_read_byte(0xFF);
	delay_u(IC_NUM * T_WAKE_MAX);

#if ((IC_NUM * T_WAKE_MAX) >= T_IDLE_MIN)
	spi_write_read_byte(0xFF);
	delay_u(IC_NUM * T_READY);
#endif
}

uint8_t ADCV[2]; //!< Cell Voltage conversion command
uint8_t ADAX[2]; //!< GPIO conversion command
uint8_t ADSTAT[2]; //!< STAT conversion command

void initialize(void)
{
	init(MD_NORMAL,DCP_DISABLED,CELL_CH_ALL,AUX_CH_ALL,STS_CH_ALL);
}

void init(uint8_t MD,	//ADC Mode
				  uint8_t DCP,	//Discharge Permit
				  uint8_t CH,	//Cell Channels to be measured
				  uint8_t CHG,	//GPIO Channels to be measured
				  uint8_t CHST	//Status Channels to be measured
				  )
{
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	ADCV[0] = md_bits + 0x02;
	md_bits = (MD & 0x01) << 7;
	ADCV[1] =  md_bits + 0x60 + (DCP<<4) + CH;

	md_bits = (MD & 0x02) >> 1;
	ADAX[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	ADAX[1] = md_bits + 0x60 + CHG;

	md_bits = (MD & 0x02) >> 1;
	ADSTAT[0] = md_bits + 0x04;
	md_bits = (MD & 0x01) << 7;
	ADSTAT[1] = md_bits + 0x68 + CHST;
}

uint16_t pec15_calc(uint8_t len,	//Number of bytes that will be used to calculate a PEC
                    uint8_t *data	//Array of data that will be used to calculate  a PEC
                   )
{
  uint16_t remainder, addr;

  remainder = 16;					//initialize the PEC
  for (uint8_t i = 0; i<len; i++)	// loops for each byte in data array
  {
    addr = ((remainder>>7)^data[i])&0xff;	//calculate PEC table address
    remainder = (remainder<<8)^crc15Table[addr];
  }
  return(remainder*2);	//The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}

void wrcfg(uint8_t total_ic,
				   uint8_t config[][6]
				  )
{
	const uint8_t BYTES_IN_REG = 6;
	const uint8_t CMD_LEN = 4 + (8 * total_ic);
	uint16_t cfg_pec;
	uint8_t cmd_index;

#if DYNAMIC_MEM
	uint8_t *cmd;
	cmd = (uint8_t *)malloc(CMD_LEN*sizeof(uint8_t));
#else
	uint8_t cmd[CMD_LEN];
#endif

	cmd[0] = 0x00;
	cmd[1] = 0x01;
	cmd[2] = 0x3d;
	cmd[3] = 0x6e;

	cmd_index = 4;
	for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--)
	{
		/* the last IC on the stack. The first configuration written is */
		/* received by the last IC in the daisy chain */

		for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)	/* executes for each of the 6 bytes in the CFGR register */
		{
			/* current_byte is the byte counter */

			cmd[cmd_index] = config[current_ic-1][current_byte];					/* adding the config data to the array to be sent */
			cmd_index = cmd_index + 1;
		}
		cfg_pec = (uint16_t)pec15_calc(BYTES_IN_REG, &config[current_ic-1][0]);		/* calculating the PEC for each ICs configuration register data */
		cmd[cmd_index] = (uint8_t)(cfg_pec >> 8);
		cmd[cmd_index + 1] = (uint8_t)cfg_pec;
		cmd_index = cmd_index + 2;
	}

	uint8_t rx_data;
	WakeIdle();
	//spi_write_array(CMD_LEN, cmd); //This function causes bad stuff!
	spi_write_then_read_array_ltc(CMD_LEN, cmd, 0, &rx_data);

#if DYNAMIC_MEM
	free(cmd);
#endif
}

int8_t rdcfg(uint8_t total_ic,
                     uint8_t r_config[][8]
                    )
{
	const uint8_t BYTES_IN_REG = 8;

	uint8_t cmd[4];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t received_pec;

#if DYNAMIC_MEM
	uint8_t *rx_data;
	rx_data = (uint8_t *) malloc((8*total_ic)*sizeof(uint8_t));
#else
	const uint8_t max_ic = 12;
	uint8_t rx_data[8 * max_ic];
#endif

	cmd[0] = 0x00;
	cmd[1] = 0x02;
	cmd[2] = 0x2b;
	cmd[3] = 0x0a;

	WakeIdle();
	spi_write_then_read_array_ltc(4, cmd, (BYTES_IN_REG*total_ic), rx_data);

	for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++)
	{
		// executes for each LTC6804 in the daisy chain and packs the data
		// into the r_config array as well as check the received Config data
		// for any bit errors

		for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
		{
			r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic*BYTES_IN_REG)];
		}

		received_pec = (r_config[current_ic][6]<<8) + r_config[current_ic][7];
		data_pec = pec15_calc(6, &r_config[current_ic][0]);
		if (received_pec != data_pec)
		{
			pec_error = -1;
		}
	}

#if DYNAMIC_MEM
	free(rx_data);
#endif
	return(pec_error);
}

void wrcfgb(uint8_t total_ic,
				   uint8_t config[][6]
				  ){
	const uint8_t BYTES_IN_REG = 6;
		const uint8_t CMD_LEN = 4 + (8 * total_ic);
		uint16_t cfg_pec;
		uint8_t cmd_index;

	#if DYNAMIC_MEM
		uint8_t *cmd;
		cmd = (uint8_t *)malloc(CMD_LEN*sizeof(uint8_t));
	#else
		uint8_t cmd[CMD_LEN];
	#endif

		cmd[0] = 0x00;
		cmd[1] = 0x24;
		//pec15_calc(2, cmd);
		cmd[2] = (pec15_calc(2, cmd) >> 8) & 0xFF;
		cmd[3] = (pec15_calc(2, cmd) >> 0) & 0xFF;

		cmd_index = 4;
		for (uint8_t current_ic = total_ic; current_ic > 0; current_ic--)
		{
			/* the last IC on the stack. The first configuration written is */
			/* received by the last IC in the daisy chain */

			for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)	/* executes for each of the 6 bytes in the CFGR register */
			{
				/* current_byte is the byte counter */

				cmd[cmd_index] = config[current_ic-1][current_byte];					/* adding the config data to the array to be sent */
				cmd_index = cmd_index + 1;
			}
			cfg_pec = (uint16_t)pec15_calc(BYTES_IN_REG, &config[current_ic-1][0]);		/* calculating the PEC for each ICs configuration register data */
			cmd[cmd_index] = (uint8_t)(cfg_pec >> 8);
			cmd[cmd_index + 1] = (uint8_t)cfg_pec;
			cmd_index = cmd_index + 2;
		}

		uint8_t rx_data;

		WakeIdle();
		spi_write_then_read_array_ltc(CMD_LEN, cmd, 0, &rx_data);
#if DYNAMIC_MEM
	free(cmd);
#endif

}

int8_t rdcfgb(uint8_t total_ic,
                     uint8_t r_config[][8]
                    )
{
	const uint8_t BYTES_IN_REG = 8;

	uint8_t cmd[4];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t received_pec;

#if DYNAMIC_MEM
	uint8_t *rx_data;
	rx_data = (uint8_t *) malloc((8*total_ic)*sizeof(uint8_t));
#else
	const uint8_t max_ic = 12;
	uint8_t rx_data[8 * max_ic];
#endif

	cmd[0] = 0x00;
	cmd[1] = 0x26;
	cmd[2] = (pec15_calc(2, cmd) >> 8) & 0xFF;
	cmd[3] = (pec15_calc(2, cmd) >> 0) & 0xFF;

	WakeIdle();
	spi_write_then_read_array_ltc(4, cmd, (BYTES_IN_REG*total_ic), rx_data);

	for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++)
		{
			// executes for each LTC6804 in the daisy chain and packs the data
			// into the r_config array as well as check the received Config data
			// for any bit errors

			for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
			{
				r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic*BYTES_IN_REG)];
			}

			received_pec = (r_config[current_ic][6]<<8) + r_config[current_ic][7];
			data_pec = pec15_calc(6, &r_config[current_ic][0]);
			if (received_pec != data_pec)
			{
				pec_error = -1;
			}
		}

	#if DYNAMIC_MEM
		free(rx_data);
	#endif
		return(pec_error);

}

uint8_t rdcv(uint8_t reg,				// Controls which cell voltage register is read back.
                     uint8_t total_ic,			// the number of ICs in the system
                     cell_data_t cell_codes[][18]	// Array of the parsed cell codes
                    )
{

	const uint8_t NUM_RX_BYT = 8;
	const uint8_t BYT_IN_REG = 6;
	const uint8_t CELL_IN_REG = 3;

	uint8_t pec_error = 0;
	uint16_t parsed_cell;
	uint16_t received_pec;
	uint16_t data_pec;
	uint8_t data_counter=0;	//data counter
#if DYNAMIC_MEM
	uint8_t *cell_data;
	cell_data = (uint8_t *)malloc((NUM_RX_BYT*total_ic)*sizeof(uint8_t));
#else
	const uint8_t max_ic = 18;
	uint8_t cell_data[NUM_RX_BYT * max_ic];
#endif


	if (reg == 0)
	{
		for (uint8_t cell_reg = 1; cell_reg<7; cell_reg++)                    //executes once for each of the LTC6804 cell voltage registers/ LTC6813 changed cell_reg<5 to cell_reg<7
		{
			data_counter = 0;
			rdcv_reg(cell_reg, total_ic,cell_data );				 //Reads a single Cell voltage register

			for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++)      // executes for every LTC6804 in the daisy chain
			{
				// current_ic is used as the IC counter

				for (uint8_t current_cell = 0; current_cell<CELL_IN_REG; current_cell++)  // This loop parses the read back data into cell voltages, it
				{
					// loops once for each of the 3 cell voltage codes in the register

					parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);//Each cell code is received as two bytes and is combined to
					// create the parsed cell voltage code
					//cell_codes[current_ic][current_cell  + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;
					cell_codes[current_ic][current_cell  + ((cell_reg - 1) * CELL_IN_REG)].voltage = parsed_cell;
					data_counter = data_counter + 2;                       //Because cell voltage codes are two bytes the data counter
					//must increment by two for each parsed cell code
				}
				received_pec = (cell_data[data_counter] << 8) + cell_data[data_counter+1]; //The received PEC for the current_ic is transmitted as the 7th and 8th
				//after the 6 cell voltage data bytes
				data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT]);
				if (received_pec != data_pec)
				{
					pec_error = -1;                             //The pec_error variable is simply set negative if any PEC errors
					//are detected in the serial data
				}
			data_counter=data_counter+2;                        //Because the transmitted PEC code is 2 bytes long the data_counter
			//must be incremented by 2 bytes to point to the next ICs cell voltage data
			}
		}
	}

	else
	{
		rdcv_reg(reg, total_ic,cell_data);
		for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++)        // executes for every LTC6804 in the daisy chain
		{
			// current_ic is used as the IC counter
			for (uint8_t current_cell = 0; current_cell < CELL_IN_REG; current_cell++)  // This loop parses the read back data into cell voltages, it
			{
				// loops once for each of the 3 cell voltage codes in the register

				parsed_cell = cell_data[data_counter] + (cell_data[data_counter+1]<<8); //Each cell code is received as two bytes and is combined to
				// create the parsed cell voltage code

				//cell_codes[current_ic][current_cell + ((reg - 1) * CELL_IN_REG)] = 0x0000FFFF & parsed_cell;
				cell_codes[current_ic][current_cell + ((reg - 1) * CELL_IN_REG)].voltage = 0x0000FFFF & parsed_cell;
				data_counter= data_counter + 2;                       //Because cell voltage codes are two bytes the data counter
				//must increment by two for each parsed cell code
			}
			received_pec = (cell_data[data_counter] << 8 )+ cell_data[data_counter + 1]; //The received PEC for the current_ic is transmitted as the 7th and 8th
			//after the 6 cell voltage data bytes
			data_pec = pec15_calc(BYT_IN_REG, &cell_data[current_ic * NUM_RX_BYT]);
			if (received_pec != data_pec)
			{
				pec_error = -1;                             //The pec_error variable is simply set negative if any PEC errors
				//are detected in the serial data
			}
			data_counter= data_counter + 2;                       //Because the transmitted PEC code is 2 bytes long the data_counter
			//must be incremented by 2 bytes to point to the next ICs cell voltage data
		}
	}
#if DYNAMIC_MEM
	free(cell_data);
#endif
	return(pec_error);
}


/*!
	\brief Read the raw data from the LTC6804 cell voltage register.
*/
void rdcv_reg(uint8_t reg,			//Determines which cell voltage register is read back
                      uint8_t total_ic,		//the number of ICs in the
                      uint8_t *data			//An array of the unparsed cell codes
                     )
{
	const uint8_t REG_LEN = 8; //number of bytes in each ICs register + 2 bytes for the PEC
	uint8_t cmd[4];
	uint16_t cmd_pec;

	if (reg == 1)
	{
		cmd[1] = 0x04;
		cmd[0] = 0x00;
	}
	else if (reg == 2)
	{
		cmd[1] = 0x06;
		cmd[0] = 0x00;
	}
	else if (reg == 3)
	{
		cmd[1] = 0x08;
		cmd[0] = 0x00;
	}
	else if (reg == 4)
	{
		cmd[1] = 0x0A;
		cmd[0] = 0x00;
	}
	else if (reg == 5) // LTC6813 - Cell register E
	{
		cmd[1] = 0x09;
		cmd[0] = 0x00;
	}
	else if (reg == 6) // LTC6813 - Cell register F
	{
		cmd[1] = 0x0B;
		cmd[0] = 0x00;
	}

	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	WakeIdle();
	WakeIdle();
	spi_write_then_read_array_ltc(4, cmd, (REG_LEN*total_ic), data);
}


/*!
	\brief Reads and parses the LTC6804 auxiliary registers.
*/
int8_t rdaux(uint8_t reg,				//Determines which GPIO voltage register is read back.
                     uint8_t total_ic,			//the number of ICs in the system
                     temp_data_t aux_codes[][GPIO_NUM]	//A two dimensional array of the gpio voltage codes.
                    )
{
	const uint8_t NUM_RX_BYT = 8;
	const uint8_t BYT_IN_REG = 6;
	const uint8_t GPIO_IN_REG = 3;

	uint8_t data_counter = 0;
	int8_t pec_error = 0;
	uint16_t parsed_aux;
	uint16_t received_pec;
	uint16_t data_pec;
#if DYNAMIC_MEM
	uint8_t *data;
	data = (uint8_t *) malloc((NUM_RX_BYT*total_ic)*sizeof(uint8_t));
#else
	const uint8_t max_ic = 12;
	uint8_t data[NUM_RX_BYT * max_ic];
#endif

	if (reg == 0)
	{
		for (uint8_t gpio_reg = 1; gpio_reg<5; gpio_reg++)                //executes once for each of the LTC6804 aux voltage registers
		{
			data_counter = 0;
			rdaux_reg(gpio_reg, total_ic,data);                 //Reads the raw auxiliary register data into the data[] array

			for (uint8_t current_ic = 0 ; current_ic < total_ic; current_ic++)      // executes for every LTC6804 in the daisy chain
			{
				// current_ic is used as the IC counter

				for (uint8_t current_gpio = 0; current_gpio< GPIO_IN_REG; current_gpio++) // This loop parses the read back data into GPIO voltages, it
				{
					// loops once for each of the 3 gpio voltage codes in the register

					parsed_aux = data[data_counter] + (data[data_counter+1]<<8);              //Each gpio codes is received as two bytes and is combined to
					// create the parsed gpio voltage code

					//aux_codes[current_ic][current_gpio +((gpio_reg-1)*GPIO_IN_REG)] = parsed_aux;
					aux_codes[current_ic][current_gpio +((gpio_reg-1)*GPIO_IN_REG)].raw = parsed_aux;
					data_counter=data_counter+2;                        //Because gpio voltage codes are two bytes the data counter
					//must increment by two for each parsed gpio voltage code
				}
				received_pec = (data[data_counter]<<8)+ data[data_counter+1];          //The received PEC for the current_ic is transmitted as the 7th and 8th
				//after the 6 gpio voltage data bytes
				data_pec = pec15_calc(BYT_IN_REG, &data[current_ic*NUM_RX_BYT]);
				if (received_pec != data_pec)
				{
					pec_error = -1;                             //The pec_error variable is simply set negative if any PEC errors
					//are detected in the received serial data
				}

				data_counter=data_counter+2;                        //Because the transmitted PEC code is 2 bytes long the data_counter
				//must be incremented by 2 bytes to point to the next ICs gpio voltage data
			}


		}

	}
	else
	{
		rdaux_reg(reg, total_ic, data);
		for (int current_ic = 0 ; current_ic < total_ic; current_ic++)            // executes for every LTC6804 in the daisy chain
		{
			// current_ic is used as an IC counter

			for (int current_gpio = 0; current_gpio<GPIO_IN_REG; current_gpio++)    // This loop parses the read back data. Loops
			{
				// once for each aux voltage in the register

				parsed_aux = (data[data_counter] + (data[data_counter+1]<<8));        //Each gpio codes is received as two bytes and is combined to
				// create the parsed gpio voltage code
				//aux_codes[current_ic][current_gpio +((reg-1)*GPIO_IN_REG)] = parsed_aux;
				aux_codes[current_ic][current_gpio +((reg-1)*GPIO_IN_REG)].raw = parsed_aux;
				data_counter=data_counter+2;                      //Because gpio voltage codes are two bytes the data counter
				//must increment by two for each parsed gpio voltage code
			}
			received_pec = (data[data_counter]<<8) + data[data_counter+1];         //The received PEC for the current_ic is transmitted as the 7th and 8th
			//after the 6 gpio voltage data bytes
			data_pec = pec15_calc(BYT_IN_REG, &data[current_ic*NUM_RX_BYT]);
			if (received_pec != data_pec)
			{
				pec_error = -1;                               //The pec_error variable is simply set negative if any PEC errors
				//are detected in the received serial data
			}

			data_counter=data_counter+2;                        //Because the transmitted PEC code is 2 bytes long the data_counter
			//must be incremented by 2 bytes to point to the next ICs gpio voltage data
		}
	}

#if DYNAMIC_MEM
	free(data);
#endif
	return (pec_error);
}

adcv_delay(void){
	delay_u(T_REFUP_MAX + T_CYCLE_FAST_MAX);
}

void adax_delay(void)
{
	delay_u(T_REFUP_MAX + T_CYCLE_FAST_MAX);
}
/*!
	\brief Read the raw data from the LTC6804 auxiliary register.
*/
void rdaux_reg(uint8_t reg,			//Determines which GPIO voltage register is read back
                       uint8_t total_ic,	//The number of ICs in the system
                       uint8_t *data		//Array of the unparsed auxiliary codes
                      )
{
	const uint8_t REG_LEN = 8; // number of bytes in the register + 2 bytes for the PEC
	uint8_t cmd[4];
	uint16_t cmd_pec;

	if (reg == 1)     //Read back auxiliary group A
	{
		cmd[1] = 0x0C;
		cmd[0] = 0x00;
	}
	else if (reg == 2)  //Read back auxiliary group B
	{
		cmd[1] = 0x0E;
		cmd[0] = 0x00;
	}
	else if (reg == 3)  //Read back auxiliary group C
	{
		cmd[1] = 0x0D;
		cmd[0] = 0x00;
	}
	else if (reg == 4)  //Read back auxiliary group D
	{
		cmd[1] = 0x0F;
		cmd[0] = 0x00;
	}
	else          //Read back auxiliary group A
	{
		cmd[1] = 0x0C;
		cmd[0] = 0x00;
	}

	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	WakeIdle();
	spi_write_then_read_array_ltc(4, cmd, (REG_LEN*total_ic), data);
}


/*!
	\brief Clears the LTC6804 cell voltage registers.
*/
void clrcell(void)
{
	uint8_t cmd[4];
	uint16_t cmd_pec;


	cmd[0] = 0x07;
	cmd[1] = 0x11;


	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec );

	WakeIdle();
	spi_write_array(4, cmd);
}


/*!
	\brief Clears the LTC6804 Auxiliary registers.
*/
void clraux(void)
{
	uint8_t cmd[4];
	uint16_t cmd_pec;

	cmd[0] = 0x07;
	cmd[1] = 0x12;

	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	WakeIdle();
	spi_write_array(4, cmd);
}


/*!
	\brief Reads status registers A of a LTC6804 daisy chain.
*/
int8_t rdstata(uint8_t total_ic,		//Number of ICs in the system
					   uint8_t r_config[][8]	//A two dimensional array that the function stores the read configuration data.
                      )
{
	const uint8_t BYTES_IN_REG = 8;

	uint8_t cmd[4];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t received_pec;

#if DYNAMIC_MEM
	uint8_t *rx_data;
	rx_data = (uint8_t *) malloc((8*total_ic)*sizeof(uint8_t));
#else
	const uint8_t max_ic = 12;
	uint8_t rx_data[8 * max_ic];
#endif

	cmd[0] = 0x00;
	cmd[1] = 0x10;
	cmd[2] = 0xed;
	cmd[3] = 0x72;

	WakeIdle();
	spi_write_then_read_array_ltc(4, cmd, (BYTES_IN_REG*total_ic), rx_data);

	for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++)       //executes for each LTC6804 in the daisy chain and packs the data
	{
		//into the r_config array as well as check the received Config data
		//for any bit errors

		for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
		{
			r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic*BYTES_IN_REG)];
		}
		received_pec = (r_config[current_ic][6]<<8) + r_config[current_ic][7];
		data_pec = pec15_calc(6, &r_config[current_ic][0]);
		if (received_pec != data_pec)
		{
			pec_error = -1;
		}
	}

#if DYNAMIC_MEM
	free(rx_data);
#endif

	return(pec_error);
}


/*!
	\brief Reads status registers B of a LTC6804 daisy chain.
*/
int8_t rdstatb(uint8_t total_ic,		//Number of ICs in the system
					   uint8_t r_config[][8]	//A two dimensional array that the function stores the read configuration data.
                      )
{
	const uint8_t BYTES_IN_REG = 8;

	uint8_t cmd[4];
	int8_t pec_error = 0;
	uint16_t data_pec;
	uint16_t received_pec;

#if DYNAMIC_MEM
	uint8_t *rx_data;
	rx_data = (uint8_t *) malloc((8*total_ic)*sizeof(uint8_t));
#else
	const uint8_t max_ic = 12;
	uint8_t rx_data[8 * max_ic];
#endif

	cmd[0] = 0x00;
	cmd[1] = 0x12;
	cmd[2] = 0x70;
	cmd[3] = 0x24;

	WakeIdle();
	spi_write_then_read_array_ltc(4, cmd, (BYTES_IN_REG*total_ic), rx_data);

	for (uint8_t current_ic = 0; current_ic < total_ic; current_ic++)       //executes for each LTC6804 in the daisy chain and packs the data
	{
		//into the r_config array as well as check the received Config data
		//for any bit errors

		for (uint8_t current_byte = 0; current_byte < BYTES_IN_REG; current_byte++)
		{
			r_config[current_ic][current_byte] = rx_data[current_byte + (current_ic*BYTES_IN_REG)];
		}
		received_pec = (r_config[current_ic][6]<<8) + r_config[current_ic][7];
		data_pec = pec15_calc(6, &r_config[current_ic][0]);
		if (received_pec != data_pec)
		{
			pec_error = -1;
		}
	}

#if DYNAMIC_MEM
	free(rx_data);
#endif

	return(pec_error);
}


/*!
	\brief Starts cell voltage conversion.
*/
void adcv(void)
{

	uint8_t cmd[4];
	uint16_t cmd_pec;

	cmd[0] = ADCV[0];
	cmd[1] = ADCV[1];

	cmd_pec = pec15_calc(2, ADCV);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	WakeIdle();
	spi_write_array(4, cmd);
}


/*!
	\brief Start an GPIO Conversion.
*/
void adax(void)
{
	uint8_t cmd[4];
	uint16_t cmd_pec;

	cmd[0] = ADAX[0];
	cmd[1] = ADAX[1];
	cmd_pec = pec15_calc(2, ADAX);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	WakeIdle();
	spi_write_array(4, cmd);
}


/*!
	\brief Start an STATUS reg Conversion.
*/
void adstat(void)
{
	uint8_t cmd[4];
	uint16_t cmd_pec;

	cmd[0] = ADSTAT[0];
	cmd[1] = ADSTAT[1];
	cmd_pec = pec15_calc(2, ADSTAT);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	WakeIdle();
	spi_write_array(4, cmd);
}
