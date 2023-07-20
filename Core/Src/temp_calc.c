#ifndef done

#include "temp_calc.h"
#include "math.h"
#include <stdint.h>






void temp_calc(uint8_t total_ic,  temp_data_t temp_data[][GPIO_NUM]){
    float v, r, vv , t;
    float B = 3605; // B-parameter from CSV file
    float R0 = 2700; // Reference resistance at 25 degrees Celsius
    float T0 = 298.15; // Reference temperature in Kelvin (25 degrees Celsius)
    float tolerance = 0.05; // Tolerance of the thermistor

    for(int i = 0; i < total_ic; i++){
        for(int j = 0; j < 5; j++){
            vv = temp_data[i][j].raw;
            v = vv/10000;
            r = (v*10000)/(3-v);
            r = r * (1 + tolerance); // Adjust resistance for tolerance
            t = log(r/R0);
            t = t / B;
            t = t + 1/T0;
            t = 1/t;
            t -= 273.15; // Convert from Kelvin to Celsius
            temp_data[i][j].temp = (int)t;
        }
    }
    for(int i = 0; i < total_ic; i++){
    	for(int j = 6; j < 9; j++){
    		vv = temp_data[i][j].raw;
            v = vv/10000;
            r = (v*10000)/(3-v);
            r = r * (1 + tolerance); // Adjust resistance for tolerance
            t = log(r/R0);
            t = t / B;
            t = t + 1/T0;
            t = 1/t;
            t -= 273.15; // Convert from Kelvin to Celsius
            temp_data[i][j].temp = (int)t;
    	}
    }
}

#endif
