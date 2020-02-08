#include "adc.h"

/**
 * @brief Returns averaged voltage in mV on analog pin.
 *
 * @param analog_pin that we want to read
 *
 * @return intiger voltage in mV
 */
uint16_t get_voltage_in_mv(uint32_t analog_pin)
{
    float voltage;
    // Needed to set the resolution of adc
    analogReadResolution(12);

    // Averaging
    for (uint8_t i = 0; i < 16; i++)
    {
        voltage += analogRead(analog_pin);
        delay(1);
    }

    voltage = voltage / 16; 

    float v_ref = STM32L0.getVDDA();

    voltage *= (v_ref/BIT_RESOLUTION);
    voltage *= ((UPPER_RESISTOR + LOWER_RESISTOR) / LOWER_RESISTOR);
    voltage *= CAL_VALUE; // Calibration value 
    voltage *= 1000; // Move from V to mV

    return (uint16_t) voltage;
}
/*** end of file ***/