#ifndef done

#include "temp_calc.h"
#include "math.h"
#include <stdint.h>

void temp_calc(uint8_t total_ic,  temp_data_t temp_data[][GPIO_NUM]){ //fix arguments
	float v, r, vv , t;
	for(int i = 0; i < total_ic; i++){
		for(int j = 0; j < 10; j++){
			//vv =  ic[i].aux.a_codes[j];
			vv = temp_data[i][j].raw;
			v = vv/10000;
			r = (v*10000)/(3-v);
			t = log(r/10000);
			    t = t / 3660;
			    t = t + 1/298.15;
			    t = 1/t;

			    t -= 273.15;

		//	ic[i].aux.s_temp[j] = t;

		}
	}

}

#endif
