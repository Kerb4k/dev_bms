/*
 * LTC681X.h
 *
 *  Created on: Apr 11, 2023
 *      Author: Dovlat Ibragimov
 */
#include "conf.h"

#ifndef INC_LTC681X_H_
#define INC_LTC681X_H_

#define DYNAMIC_MEM	0

/*
	Number of LTC6804s on daisy-chain
*/
//#define IC_NUM		12			// This define can be found in shared.h

/*
	Timing of states (in microseconds)
*/
#define T_WAKE_MAX		400
#define T_READY			10
#define T_IDLE_MIN		4300
#define T_REFUP_MAX		4400
/*
	Measurement + Calibration Cycle Time When
	Starting from the REFUP State in Fast Mode
*/
#define T_CYCLE_FAST_MAX	1185	// Measure 12 Cells


/*
	Conversion mode
*/
#define MD_FAST 1
#define MD_NORMAL 2
#define MD_FILTERED 3


/*
	Conversion channels
*/
#define CELL_CH_ALL 0
/*
	Conversion channels
*/
#define AUX_CH_ALL 0
/*
	Conversion channels
*/
#define STS_CH_ALL 0
#define STS_CH_SOC 1
#define STS_CH_ITMP 2
#define STS_CH_VA 3
#define STS_CH_VD 4

//uint8_t CHG = 0; //!< aux channels to be converted
/*!
\brief Controls if Discharging transistors are enabled
or disabled during Cell conversions.
*/
#define DCP_DISABLED 0
#define DCP_ENABLED 1

void WakeIdle(void);
void WakeUp(void);
void initialize(void);
void init(uint8_t MD, uint8_t DCP, uint8_t CH, uint8_t CHG, uint8_t CHST);
uint16_t pec15_calc(uint8_t len, uint8_t *data);
void wrcfg(uint8_t total_ic, uint8_t config[][6]);
int8_t rdcfg(uint8_t total_ic, uint8_t r_config[][8]);
uint8_t rdcv(uint8_t reg, uint8_t total_ic, cell_data_t cell_codes[][CELL_NUM]);
void rdcv_reg(uint8_t reg, uint8_t total_ic, uint8_t *data);
int8_t rdaux(uint8_t reg, uint8_t total_ic, temp_data_t aux_codes[][GPIO_NUM]);
void rdaux_reg(uint8_t reg, uint8_t total_ic, uint8_t *data);
void clrcell(void);
void clraux(void);
int8_t rdstata(uint8_t total_ic, uint8_t r_config[][8]);
int8_t rdstatb(uint8_t total_ic, uint8_t r_config[][8]);
void adcv(void);
void adax(void);
void adstat(void);

static const unsigned int crc15Table[256] = {
	0x0000, 0xc599, 0xceab, 0x0b32, 0xd8cf, 0x1d56, 0x1664, 0xd3fd, 0xf407, 0x319e, 0x3aac,
	0xff35, 0x2cc8, 0xe951, 0xe263, 0x27fa, 0xad97, 0x680e, 0x633c, 0xa6a5, 0x7558, 0xb0c1,
	0xbbf3, 0x7e6a, 0x5990, 0x9c09, 0x973b, 0x52a2, 0x815f, 0x44c6, 0x4ff4, 0x8a6d, 0x5b2e,
	0x9eb7, 0x9585, 0x501c, 0x83e1, 0x4678, 0x4d4a, 0x88d3, 0xaf29, 0x6ab0, 0x6182, 0xa41b,
	0x77e6, 0xb27f, 0xb94d, 0x7cd4, 0xf6b9, 0x3320, 0x3812, 0xfd8b, 0x2e76, 0xebef, 0xe0dd,
	0x2544, 0x02be, 0xc727, 0xcc15, 0x098c, 0xda71, 0x1fe8, 0x14da, 0xd143, 0xf3c5, 0x365c,
	0x3d6e, 0xf8f7, 0x2b0a, 0xee93, 0xe5a1, 0x2038, 0x07c2, 0xc25b, 0xc969, 0x0cf0, 0xdf0d,
	0x1a94, 0x11a6, 0xd43f, 0x5e52, 0x9bcb, 0x90f9, 0x5560, 0x869d, 0x4304, 0x4836, 0x8daf,
	0xaa55, 0x6fcc, 0x64fe, 0xa167, 0x729a, 0xb703, 0xbc31, 0x79a8, 0xa8eb, 0x6d72, 0x6640,
	0xa3d9, 0x7024, 0xb5bd, 0xbe8f, 0x7b16, 0x5cec, 0x9975, 0x9247, 0x57de, 0x8423, 0x41ba,
	0x4a88, 0x8f11, 0x057c, 0xc0e5, 0xcbd7, 0x0e4e, 0xddb3, 0x182a, 0x1318, 0xd681, 0xf17b,
	0x34e2, 0x3fd0, 0xfa49, 0x29b4, 0xec2d, 0xe71f, 0x2286, 0xa213, 0x678a, 0x6cb8, 0xa921,
	0x7adc, 0xbf45, 0xb477, 0x71ee, 0x5614, 0x938d, 0x98bf, 0x5d26, 0x8edb, 0x4b42, 0x4070,
	0x85e9, 0x0f84, 0xca1d, 0xc12f, 0x04b6, 0xd74b, 0x12d2, 0x19e0, 0xdc79, 0xfb83, 0x3e1a,
	0x3528, 0xf0b1, 0x234c, 0xe6d5, 0xede7, 0x287e, 0xf93d, 0x3ca4, 0x3796, 0xf20f, 0x21f2,
	0xe46b, 0xef59, 0x2ac0, 0x0d3a, 0xc8a3, 0xc391, 0x0608, 0xd5f5, 0x106c, 0x1b5e, 0xdec7,
	0x54aa, 0x9133, 0x9a01, 0x5f98, 0x8c65, 0x49fc, 0x42ce, 0x8757, 0xa0ad, 0x6534, 0x6e06,
	0xab9f, 0x7862, 0xbdfb, 0xb6c9, 0x7350, 0x51d6, 0x944f, 0x9f7d, 0x5ae4, 0x8919, 0x4c80,
	0x47b2, 0x822b, 0xa5d1, 0x6048, 0x6b7a, 0xaee3, 0x7d1e, 0xb887, 0xb3b5, 0x762c, 0xfc41,
	0x39d8, 0x32ea, 0xf773, 0x248e, 0xe117, 0xea25, 0x2fbc, 0x0846, 0xcddf, 0xc6ed, 0x0374,
	0xd089, 0x1510, 0x1e22, 0xdbbb, 0x0af8, 0xcf61, 0xc453, 0x01ca, 0xd237, 0x17ae, 0x1c9c,
	0xd905, 0xfeff, 0x3b66, 0x3054, 0xf5cd, 0x2630, 0xe3a9, 0xe89b, 0x2d02, 0xa76f, 0x62f6,
	0x69c4, 0xac5d, 0x7fa0, 0xba39, 0xb10b, 0x7492, 0x5368, 0x96f1, 0x9dc3, 0x585a, 0x8ba7,
	0x4e3e, 0x450c, 0x8095
};

#endif /* INC_LTC681X_H_ */
