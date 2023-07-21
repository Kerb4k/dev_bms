#ifndef done

#include "temp_calc.h"
#include "math.h"
#include <stdint.h>






void temp_calc(uint8_t total_ic,  temp_data_t temp_data[][GPIO_NUM]){
    float v, r, t;
    float B = 3730; // B-parameter from CSV file
    float R0 = 2700; // Reference resistance at 25 degrees Celsius
    float T0 = 298.15; // Reference temperature in Kelvin (25 degrees Celsius)

    float R1 = 2700; // Resistance of the voltage divider in ohms

    for(int i = 0; i < total_ic; i++){
        for(int j = 0; j < 6; j++){
        	float Vs = (float)temp_data[i][5].raw / 10000; // Source voltage in volts Vref2
            v = (float)temp_data[i][j].raw / 10000; // Convert raw reading to volts
            r = (v * R1) / (Vs - v); // Calculate resistance of the thermistor
            t = log(r/R0);
            t = t / B;
            t = t + 1/T0;
            t = 1/t;
            t -= 273.15; // Convert from Kelvin to Celsius
            temp_data[i][j].temp = (int)t;
        }
    }

    for(int i = 0; i < total_ic; i++){
    	for(int j = 6; j < 10; j++){
    		float Vs = (float)temp_data[i][5].raw / 10000; // Source voltage in volts Vref2
    		v = (float)temp_data[i][j].raw / 10000; // Convert raw reading to volts
    		r = (v * R1) / (Vs - v); // Calculate resistance of the thermistor
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
