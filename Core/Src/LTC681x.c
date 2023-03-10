#include <stdint.h>
#include "LTC681x.h"
#include "isoSpi.h"
#include <stdlib.h>


const uint16_t crc15Table[256] = {0x0,0xc599, 0xceab, 0xb32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,  // precomputed CRC15 Table
                                0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
                                0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
                                0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
                                0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
                                0x2544, 0x2be, 0xc727, 0xcc15, 0x98c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
                                0x3d6e, 0xf8f7,0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x7c2, 0xc25b, 0xc969, 0xcf0, 0xdf0d,
                                0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
                                0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
                                0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
                                0x4a88, 0x8f11, 0x57c, 0xc0e5, 0xcbd7, 0xe4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
                                0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
                                0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
                                0x85e9, 0xf84, 0xca1d, 0xc12f, 0x4b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a, 0x3528,
                                0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2, 0xe46b, 0xef59,
                                0x2ac0, 0xd3a, 0xc8a3, 0xc391, 0x608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7, 0x54aa, 0x9133, 0x9a01,
                                0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06, 0xab9f, 0x7862, 0xbdfb, 0xb6c9,
                                0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80, 0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a,
                                0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41, 0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25,
                                0x2fbc, 0x846, 0xcddf, 0xc6ed, 0x374, 0xd089, 0x1510, 0x1e22, 0xdbbb, 0xaf8, 0xcf61, 0xc453,
                                0x1ca, 0xd237, 0x17ae, 0x1c9c, 0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b,
                                0x2d02, 0xa76f, 0x62f6, 0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3,
                                0x585a, 0x8ba7, 0x4e3e, 0x450c, 0x8095
                               };

/* Calculates  and returns the CRC15 */
uint16_t pec15_calc(uint8_t len, //Number of bytes that will be used to calculate a PEC
                    uint8_t *data //Array of data that will be used to calculate  a PEC
                   )
{
	uint16_t remainder,addr;
	remainder = 16;//initialize the PEC

	for (uint8_t i = 0; i<len; i++) // loops for each byte in data array
	{
		addr = ((remainder>>7)^data[i])&0xff;//calculate PEC table address

		remainder = (remainder<<8)^crc15Table[addr];

	}

	return(remainder*2);//The CRC15 has a 0 in the LSB so the remainder must be multiplied by 2
}

/* Helper function that increments PEC counters */
void LTC681x_check_pec(uint8_t reg, //Type of Register
					   cell_asic *ic //A two dimensional array that stores the data
					   )
{
	switch (reg)
	{
		case CFGR:
		  for (int current_ic = 0 ; current_ic < ic_number; current_ic++)
		  {
			ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + ic[current_ic].config.rx_pec_match;
			ic[current_ic].crc_count.cfgr_pec = ic[current_ic].crc_count.cfgr_pec + ic[current_ic].config.rx_pec_match;
		  }
		break;

		case CFGRB:
		  for (int current_ic = 0 ; current_ic < ic_number; current_ic++)
		  {
			ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + ic[current_ic].configb.rx_pec_match;
			ic[current_ic].crc_count.cfgr_pec = ic[current_ic].crc_count.cfgr_pec + ic[current_ic].configb.rx_pec_match;
		  }
		break;
		case CELL:
		  for (int current_ic = 0 ; current_ic < ic_number; current_ic++)
		  {
			for (int i=0; i<ic[0].ic_reg.num_cv_reg; i++)
			{
			  ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + ic[current_ic].cells.pec_match[i];
			  ic[current_ic].crc_count.cell_pec[i] = ic[current_ic].crc_count.cell_pec[i] + ic[current_ic].cells.pec_match[i];
			}
		  }
		break;
		case AUX:
		  for (int current_ic = 0 ; current_ic < ic_number; current_ic++)
		  {
			for (int i=0; i<ic[0].ic_reg.num_gpio_reg; i++)
			{
			  ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + (ic[current_ic].aux.pec_match[i]);
			  ic[current_ic].crc_count.aux_pec[i] = ic[current_ic].crc_count.aux_pec[i] + (ic[current_ic].aux.pec_match[i]);
			}
		  }

		break;
		case STAT:
		  for (int current_ic = 0 ; current_ic < ic_number; current_ic++)
		  {

			for (int i=0; i<ic[0].ic_reg.num_stat_reg-1; i++)
			{
			  ic[current_ic].crc_count.pec_count = ic[current_ic].crc_count.pec_count + ic[current_ic].stat.pec_match[i];
			  ic[current_ic].crc_count.stat_pec[i] = ic[current_ic].crc_count.stat_pec[i] + ic[current_ic].stat.pec_match[i];
			}
		  }
		break;
		default:
		break;
	}
}


/* Generic function to write 68xx commands. Function calculates PEC for tx_cmd data. */
void cmd_68(uint8_t tx_cmd[2], SPI_HandleTypeDef *hspi) //The command to be transmitted
{
	uint8_t cmd[4];
	uint16_t cmd_pec;
	//uint8_t md_bits;

	cmd[0] = tx_cmd[0];
	cmd[1] =  tx_cmd[1];
	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);


	wakeup_idle(hspi);
	spi_write_array(4,cmd,hspi);

}


/* Wake isoSPI up from IDlE state and enters the READY state */
void wakeup_idle(SPI_HandleTypeDef *hspi) //Number of ICs in the system
{
	for (int i =0; i<ic_number; i++)
		{
		   cs_low();
		   delay_u(300); // Guarantees the LTC681x will be in standby ///if it doesnt work use delay_u(300)
		   cs_high();
		   delay_u(10); ///if it doesnt work use delay_u(10)
		}
}

/*!
 Wake the LTC681x from the sleep state
 @return void
 */
void wakeup_sleep(){
	for (int i =0; i<ic_number; i++)
			{
			   cs_low(CS_PIN);
			   delay_u(300); // Guarantees the LTC681x will be in standby ///if it doesnt work use delay_u(300)
			   cs_high(CS_PIN);
			   delay_u(10); ///if it doesnt work use delay_u(10)
			}
}

/* Helper function that parses voltage measurement registers */
int8_t parse_cells(uint8_t current_ic, // Current IC
					uint8_t cell_reg,  // Type of register
					uint8_t cell_data[], // Unparsed data
					uint16_t *cell_codes, // Parsed data
					uint8_t *ic_pec // PEC error
					//SPI_HandleTypeDef *hspi
					)
{
	const uint8_t BYT_IN_REG = 6;
	const uint8_t CELL_IN_REG = 3;
	int8_t pec_error = 0;
	uint16_t parsed_cell;
	uint16_t received_pec;
	uint16_t data_pec;
	uint8_t data_counter = current_ic*NUM_RX_BYT; //data counter


	for (uint8_t current_cell = 0; current_cell<CELL_IN_REG; current_cell++) // This loop parses the read back data into the register codes, it
	{																		// loops once for each of the 3 codes in the register

		parsed_cell = cell_data[data_counter] + (cell_data[data_counter + 1] << 8);//Each code is received as two bytes and is combined to
																				   // create the parsed code
		cell_codes[current_cell  + ((cell_reg - 1) * CELL_IN_REG)] = parsed_cell;

		data_counter = data_counter + 2;                       //Because the codes are two bytes, the data counter
															  //must increment by two for each parsed code
	}
	received_pec = (cell_data[data_counter] << 8) | cell_data[data_counter+1]; //The received PEC for the current_ic is transmitted as the 7th and 8th
																			   //after the 6 cell voltage data bytes
	data_pec = pec15_calc(BYT_IN_REG, &cell_data[(current_ic) * NUM_RX_BYT]);

	if (received_pec != data_pec)
	{
		pec_error = 1;                             //The pec_error variable is simply set negative if any PEC errors
		ic_pec[cell_reg-1]=1;
	}
	else
	{
		ic_pec[cell_reg-1]=0;
	}
	data_counter=data_counter+2;

	return(pec_error);
}

/* Writes the command and reads the raw cell voltage register data */
void LTC681x_rdcv_reg(uint8_t reg, //Determines which cell voltage register is read back
                      uint8_t *data, //An array of the unparsed cell codes
					  SPI_HandleTypeDef *hspi
                     )
{
	const uint8_t REG_LEN = 8; //Number of bytes in each ICs register + 2 bytes for the PEC
	uint8_t cmd[4];
	uint16_t cmd_pec;

	if (reg == 1)     //1: RDCVA
	{
		cmd[1] = 0x04;
		cmd[0] = 0x00;
	}
	else if (reg == 2) //2: RDCVB
	{
		cmd[1] = 0x06;
		cmd[0] = 0x00;
	}
	else if (reg == 3) //3: RDCVC
	{
		cmd[1] = 0x08;
		cmd[0] = 0x00;
	}
	else if (reg == 4) //4: RDCVD
	{
		cmd[1] = 0x0A;
		cmd[0] = 0x00;
	}
	else if (reg == 5) //4: RDCVE
	{
		cmd[1] = 0x09;
		cmd[0] = 0x00;
	}
	else if (reg == 6) //4: RDCVF
	{
		cmd[1] = 0x0B;
		cmd[0] = 0x00;
	}

	cmd_pec = pec15_calc(2, cmd);
	cmd[2] = (uint8_t)(cmd_pec >> 8);
	cmd[3] = (uint8_t)(cmd_pec);

	wakeup_idle(hspi);
	spi_write_read(cmd,4,data,(REG_LEN*ic_number),hspi);

}

uint8_t rdcv(        cell_asic *ic, //!< Array of the parsed cell codes
					 SPI_HandleTypeDef *hspi
                    ){


		int8_t pec_error = 0;
	    uint8_t cell_data[NUM_RX_BYT * ic_number];
	    uint8_t c_ic = 0;

	    // Executes once for each of the LTC6811 cell voltage registers
	    for (uint8_t cell_reg = 1; cell_reg < 7; cell_reg++) {

	    	LTC681x_rdcv_reg(cell_reg,cell_data,hspi );
	    				for (int current_ic = 0; current_ic<ic_number; current_ic++)
	    				{
	    				if (ic->isospi_reverse == false)
	    				{
	    				  c_ic = current_ic;
	    				}
	    				else
	    				{
	    				  c_ic = ic_number - current_ic - 1;
	    				}
	    				pec_error = pec_error + parse_cells(current_ic,cell_reg, cell_data,
	    													&ic[c_ic].cells.c_codes[0],
	    													&ic[c_ic].cells.pec_match[0]);
	    				}

	    }
	    LTC681x_check_pec(CELL, ic);

	  return(pec_error);
}

void clrcell(SPI_HandleTypeDef *hspi){

	uint8_t cmd[2]= {0x07 , 0x11};
		cmd_68(cmd,hspi);
}


/* Starts ADC conversion for cell voltage */
void adcv( uint8_t MD, //ADC Mode
				   uint8_t DCP, //Discharge Permit
				   uint8_t CH, //Cell Channels to be measured
				   SPI_HandleTypeDef *hspi
                 )
{
	uint8_t cmd[2];
	uint8_t md_bits;

	md_bits = (MD & 0x02) >> 1;
	cmd[0] = md_bits + 0x02;
	md_bits = (MD & 0x01) << 7;
	cmd[1] =  md_bits + 0x60 + (DCP<<4) + CH;

	cmd_68(cmd, hspi);
}

int cell_voltage(cell_asic *ic ,SPI_HandleTypeDef *hspi){
	uint8_t pec;
	wakeup_sleep();
	adcv(1, 0, 0, hspi);
	//delay_u(T_refup_max+T_CYCLE_FAST_MAX); //waiting for conversion
	delay_m(10);

	pec = rdcv(ic, hspi);
	clrcell(hspi);
}
