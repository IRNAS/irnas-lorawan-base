#include "project_utils.h"

// Define debug if required
#define serial_debug  Serial

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
float mapfloat(float x, float in_min, float in_max, float out_min, float out_max)
{
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
uint32_t get_bits(float x, float min, float max, int precision)
{
    int range = max - min;
    if (x <= min) 
    {
        x = 0;
    }
    else if (x >= max) 
    {
        x = max - min;
    }
    else 
    {
        x -= min;
    }
    uint32_t new_range = (pow(2, precision) - 1) / range;
    uint32_t out_x = x * new_range;
    return out_x;
}

// Implemented based on https://stackoverflow.com/questions/15638612/calculating-mean-and-standard-deviation-of-the-data-which-does-not-fit-in-memory

void push_value(float value, reading_structure_t * values)
{
    // special case if the array is empty, opulate both min and max
    if(0 == values->r_count)
    {
        values->r_min = value;
        values->r_max = value;
    }
    values->r_count++;

    float delta = value - values->r_mean;
    values->r_mean += delta/values->r_count;
    values->r_m2 += delta * (value - values->r_mean);
    values->r_min = min(values->r_min, value);
    values->r_max = max(values->r_max, value);
}

float get_variance(reading_structure_t * values)
{
    return values->r_m2 / (values->r_count - 1);
}

void clear_value(reading_structure_t * values)
{
    values->r_min = 0;
    values->r_max = 0;
    values->r_mean = 0;
    values->r_count = 0;
    values->r_m2 = 0;
}

/**
 * @brief Converts binary coded decimal to binary
 *
 * @param[in] val
 *
 * @return unsinged int
 */
uint16_t bcd2bin(unsigned char val)
{
    return (val & 0x0F) + (val >> 4) * 10;
}

/**
 * @brief Converts binary to binary coded decimal
 *
 * @param[in] val
 *
 * @return char
 */
char bin2bcd(uint16_t val)
{
    return ((val / 10) << 4) + val % 10;
}

/**
 * @brief function to check if i2c pull-up is present, call only once prior to init of Wire library
 * 
 * @return boolean 
 */
boolean check_i2c()
{
    boolean fail = false;
    pinMode(PIN_WIRE_SCL, INPUT);
    pinMode(PIN_WIRE_SDA, INPUT);

    if(LOW == digitalRead(PIN_WIRE_SCL))
    {
        //no I2C pull-up detected
#ifdef serial_debug
        serial_debug.print("i2c pull-up error(");
        serial_debug.println("scl)");
#endif
        fail = true;
    }
    if(LOW == digitalRead(PIN_WIRE_SDA))
    {
        //no I2C pull-up detected
#ifdef serial_debug
        serial_debug.print("i2c pull-up error(");
        serial_debug.println("sda)");
#endif
        fail = true;
    }
    return fail;
}
