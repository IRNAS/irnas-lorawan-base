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

enum event_e : uint8_t
{
  EVENT_NONE,
  EVENT_HALL,
  EVENT_INPUT1,
  EVENT_INPUT2,
  EVENT_MOTION,
  EVENT_FREEFALL,
}; 


boolean check_i2c();
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max);
uint32_t get_bits(float x, float min, float max, int precision);
void push_value(float value,reading_structure_t * values);
float get_variance(reading_structure_t * values);
void clear_value(reading_structure_t * values);
uint16_t bcd2bin(unsigned char val);
char bin2bcd(uint16_t val);

#endif /* PROJECT_UTILS_H_ */
