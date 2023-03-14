#ifndef LTC681x_h
#define LTC681x_h

#include <stdint.h>
#include <stdbool.h>
#include "stm32g4xx.h"

#define ic_number 2

//time constants
#define T_CYCLE_FAST_MAX	1191	// Measure 18 Cells
#define T_refup_max 4400
#define T_wake_max 400

///////////////////////////////////////////////////////

#define cell_n 18
#define IC_LTC6813

#define MD_422HZ_1KHZ 0
#define MD_27KHZ_14KHZ 1
#define MD_7KHZ_3KHZ 2
#define MD_26HZ_2KHZ 3

#define ADC_OPT_ENABLED 1
#define ADC_OPT_DISABLED 0

#define CELL_CH_ALL 0
#define CELL_CH_1and7 1
#define CELL_CH_2and8 2
#define CELL_CH_3and9 3
#define CELL_CH_4and10 4
#define CELL_CH_5and11 5
#define CELL_CH_6and12 6

#define SELFTEST_1 1
#define SELFTEST_2 2

#define AUX_CH_ALL 0
#define AUX_CH_GPIO1 1
#define AUX_CH_GPIO2 2
#define AUX_CH_GPIO3 3
#define AUX_CH_GPIO4 4
#define AUX_CH_GPIO5 5
#define AUX_CH_VREF2 6

#define STAT_CH_ALL 0
#define STAT_CH_SOC 1
#define STAT_CH_ITEMP 2
#define STAT_CH_VREGA 3
#define STAT_CH_VREGD 4

#define REG_ALL 0
#define REG_1 1
#define REG_2 2
#define REG_3 3
#define REG_4 4
#define REG_5 5
#define REG_6 6

#define DCP_DISABLED 0
#define DCP_ENABLED 1

#define PULL_UP_CURRENT 1
#define PULL_DOWN_CURRENT 0

#define NUM_RX_BYT 8
#define CELL 1
#define AUX 2
#define STAT 3
#define CFGR 0
#define CFGRB 4
#define CS_PIN 10

/*! Cell Voltage data structure. */
typedef struct
{
  uint16_t c_codes[18]; //!< Cell Voltage Codes
  uint8_t pec_match[6]; //!< If a PEC error was detected during most recent read cmd
} cv;

/*! AUX Reg Voltage Data structure */
typedef struct
{
  uint16_t a_codes[12]; //!< Aux Voltage Codes
  uint8_t pec_match[4]; //!< If a PEC error was detected during most recent read cmd
  double s_temp[10];

} ax;

/*! Status Reg data structure. */
typedef struct
{
  uint16_t stat_codes[4]; //!< Status codes.
  uint8_t flags[3]; //!< Byte array that contains the uv/ov flag data
  uint8_t mux_fail[1]; //!< Mux self test status flag
  uint8_t thsd[1]; //!< Thermal shutdown status
  uint8_t pec_match[2]; //!< If a PEC error was detected during most recent read cmd
} st;

/*! IC register structure. */
typedef struct
{
  uint8_t tx_data[6];  //!< Stores data to be transmitted
  uint8_t rx_data[8];  //!< Stores received data
  uint8_t rx_pec_match; //!< If a PEC error was detected during most recent read cmd
} ic_register;

/*! PEC error counter structure. */
typedef struct
{
  uint16_t pec_count; //!< Overall PEC error count
  uint16_t cfgr_pec;  //!< Configuration register data PEC error count
  uint16_t cell_pec[6]; //!< Cell voltage register data PEC error count
  uint16_t aux_pec[4];  //!< Aux register data PEC error count
  uint16_t stat_pec[2]; //!< Status register data PEC error count
} pec_counter;

/*! Register configuration structure */
typedef struct
{
  uint8_t cell_channels; //!< Number of Cell channels
  uint8_t stat_channels; //!< Number of Stat channels
  uint8_t aux_channels;  //!< Number of Aux channels
  uint8_t num_cv_reg;    //!< Number of Cell voltage register
  uint8_t num_gpio_reg;  //!< Number of Aux register
  uint8_t num_stat_reg;  //!< Number of  Status register
} register_cfg;


/*! Cell variable structure */
typedef struct
{
  ic_register config;
  ic_register configb;
  cv  cells;
  ax  aux;
  st  stat;
  ic_register com;
  ic_register pwm;
  ic_register pwmb;
  ic_register sctrl;
  ic_register sctrlb;
  uint8_t sid[6];
  bool isospi_reverse;
  pec_counter crc_count;
  register_cfg ic_reg;
  long system_open_wire;
} cell_asic;


/* Calculates  and returns the CRC15 */
uint16_t pec15_calc(uint8_t len, //Number of bytes that will be used to calculate a PEC
                    uint8_t *data //Array of data that will be used to calculate  a PEC
                   );
/* Helper function that increments PEC counters */
void check_pec(uint8_t reg, //Type of Register
					   cell_asic *ic //A two dimensional array that stores the data
					   );

/*!
 Wake isoSPI up from IDlE state and enters the READY state
 @return void
 */
void wakeup_idle( SPI_HandleTypeDef *hspi);//!< Number of ICs in the daisy chain

/*!
 Wake the LTC681x from the sleep state
 @return void
 */
void cmd_68(uint8_t tx_cmd[2], SPI_HandleTypeDef *hspi);

void wakeup_sleep(); //!< Number of ICs in the daisy chain
/* Helper function that parses voltage measurement registers */
int8_t parse_cells(uint8_t current_ic, // Current IC
					uint8_t cell_reg,  // Type of register
					uint8_t cell_data[], // Unparsed data
					uint16_t *cell_codes, // Parsed data
					uint8_t *ic_pec // PEC error
					//SPI_HandleTypeDef *hspi
					);
/* Writes the command and reads the raw cell voltage register data */
void rdcv_reg(uint8_t reg, //Determines which cell voltage register is read back
                      uint8_t *data, //An array of the unparsed cell codes
					  SPI_HandleTypeDef *hspi
                     );
/*!
 Reads and parses the LTC681x cell voltage registers.
 The function is used to read the cell codes of the LTC681x.
 This function will send the requested read commands parse the data and store the cell voltages in the cell_asic structure.
 @return uint8_t, PEC Status.
  0: No PEC error detected
 -1: PEC error detected, retry read
 */
uint8_t rdcv(cell_asic *ic, //!< Array of the parsed cell codes
			 SPI_HandleTypeDef *hspi);



/*!
 Starts cell voltage conversion
 Starts ADC conversions of the LTC681x Cpin inputs.
 The type of ADC conversion executed can be changed by setting the following parameters:
 @return void
 */
void adcv(uint8_t MD, //!< ADC conversion Mode
                  uint8_t DCP, //!< Controls if Discharge is permitted during conversion
                  uint8_t CH, //!< Sets which Cell channels are converted
				  SPI_HandleTypeDef *hspi
                 );
//clears cell voltages
void clrcell(SPI_HandleTypeDef *hspi);

void cell_voltage(cell_asic *ic ,SPI_HandleTypeDef *hspi);


void rdaux_reg(uint8_t reg, //Determines which GPIO voltage register is read back
                       uint8_t *data, //Array of the unparsed auxiliary codes
					   SPI_HandleTypeDef *hspi
                      );

uint8_t rdaux(cell_asic *ic,//!<  Array of the parsed aux codes
		 SPI_HandleTypeDef *hspi);

void adax( uint8_t MD, //!< ADC Conversion Mode
				  uint8_t CHG, //!< Sets which GPIO channels are converted
				  SPI_HandleTypeDef *hspi
				);

void clraux(SPI_HandleTypeDef *hspi);

void stack_temp(cell_asic *ic ,SPI_HandleTypeDef *hspi);

#endif
