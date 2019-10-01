#include "project_utils.h"

/**
 * @brief Maps the variable with the given minimum and maximum value to the value with specified min, max
 * 
 * @param x - value
 * @param in_min - minimum of the value expected
 * @param in_max - maximum of the value expected
 * @param out_min - minimum value of the output
 * @param out_max - maximum value of the output
 * @return float
 */
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max){
 return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

/**
 * @brief Maps a specified value to a number with specified precision in number of bits between the given min and max
 * 
 * @param x 
 * @param min 
 * @param max 
 * @param precision 
 * @return uint32_t 
 */
uint32_t get_bits(float x, float min, float max, int precision){
  int range = max - min;
  if (x <= min) {
    x = 0;
  }
  else if (x >= max) {
    x = max - min;
  }
  else {
    x -= min;
  }
  double new_range = (pow(2, precision) - 1) / range;
  uint32_t out_x = x * new_range;
  return out_x;
}

// Implemented based on https://stackoverflow.com/questions/15638612/calculating-mean-and-standard-deviation-of-the-data-which-does-not-fit-in-memory

void push_value(float value,reading_structure_t *values){
  values->r_count++;
  float delta = value - values->r_mean;
  values->r_mean += delta/values->r_count;
  values->r_m2+= delta*(value - values->r_mean);
  values->r_min=min(values->r_min,value);
  values->r_max=max(values->r_max,value);
}

float get_variance(reading_structure_t *values){
  return values->r_m2/(values->r_count-1);
}

void clear_value(reading_structure_t *values){
  values->r_min=0;
  values->r_max=0;
  values->r_mean=0;
  values->r_count=0;
  values->r_m2=0;
}
