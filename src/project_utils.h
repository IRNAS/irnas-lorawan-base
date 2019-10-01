#ifndef PROJECT_UTILS_H_
#define PROJECT_UTILS_H_

#include "Arduino.h"

struct reading_structure_t
{
    float r_min;
    float r_max;
    float r_mean;
    float r_count;
    float r_m2;
};


float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
uint32_t get_bits(float x, float min, float max, int precision);
void push_value(float value,reading_structure_t *values);
float get_variance(reading_structure_t *values);
void clear_value(reading_structure_t *values);

#endif /* PROJECT_UTILS_H_ */