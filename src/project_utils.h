#ifndef PROJECT_UTILS_H_
#define PROJECT_UTILS_H_

#include "Arduino.h"
#include "LIS2DW12.h"

struct reading_structure_t
{
    float r_min;
    float r_max;
    float r_mean;
    float r_count;
    float r_m2;
};

enum input_event_e : uint16_t{
  HALL,
  INPUT1,
  INPUT2
}; 

struct main_share_t{
  uint64_t time;
  uint16_t voltage;
  input_event_e input_event;
  lis2dw_event_t accelerometer_event_e ;
};




float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
uint32_t get_bits(float x, float min, float max, int precision);
void push_value(float value,reading_structure_t *values);
float get_variance(reading_structure_t *values);
void clear_value(reading_structure_t *values);
unsigned int bcd2bin(unsigned char val);
char bin2bcd(unsigned int val);

#endif /* PROJECT_UTILS_H_ */