#ifndef ADC_H
#define ADC_H

#define BIT_RESOLUTION 4095.0f // 2^12-1
#define UPPER_RESISTOR 219.4f  // in kOhms
#define LOWER_RESISTOR 99.4f   // in kOhms
#define CAL_VALUE      1.064f  // Calibration value that swe get by measurement

#include <STM32L0.h>

uint16_t get_voltage_in_mv(uint32_t analog_pin);

#endif