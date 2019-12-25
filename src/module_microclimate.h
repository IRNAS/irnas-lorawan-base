#ifndef MODULE_MICROCLIMATE_h
#define MODULE_MICROCLIMATE_h

#include <Arduino.h>
#include "project.h"
#include "project_utils.h"
#include "STM32L0.h"
#include "module.h"
#include <Wire.h> 
#include <Dps310.h>// Install through Manager
#include "HDC2080.h"

class MODULE_MICROCLIMATE
{
    public:
        // functions
        uint8_t configure(uint8_t * data, size_t * size);
        uint8_t get_settings_length();
        uint8_t set_downlink_data(uint8_t * data, size_t * size);
        module_flags_e scheduler(void);
        uint8_t initialize(void);
        uint8_t send(uint8_t * buffer, size_t * size);
        uint8_t read(void);
        uint8_t running(void);
        void event(event_e event);
        void print_data(void);

        // variables
        module_flags_e flags=M_ERROR;

        // parameters
        uint8_t param_a = 0;
        uint8_t param_b = 0;
        uint8_t param_c = 0;

    private:
        long calcPressureAltitude(long _PressureInPascal);
        // add
        struct module_settings_data_t
        {
            uint8_t  global_id;
            uint8_t  length;
            uint16_t read_interval; // in seconds
            uint16_t send_interval; // in minutes
        }__attribute__((packed));

        union module_settings_packet_t
        {
            module_settings_data_t data;
            uint8_t bytes[sizeof(module_settings_data_t)];
        };

        // we are sending 13 bytes anyhow as header, filling up the packet does not matter too much
        struct module_readings_data_t
        {
            uint16_t temperature_avg; // average of readings during a period, maximally once a day
            uint16_t temperature_min; // min of readings during a period, maximally once a day
            uint16_t temperature_max; // max of readings during a period, maximally once a day
            uint16_t pressure_avg; // average of readings during a period, maximally once a day
            uint16_t pressure_min; // min of readings during a period, maximally once a day
            uint16_t pressure_max; // max of readings during a period, maximally once a day
            uint16_t humidity_avg; // average of readings during a period, maximally once a day
            uint16_t humidity_min; // min of readings during a period, maximally once a day
            uint16_t humidity_max; // max of readings during a period, maximally once a day
        }__attribute__((packed));

        union module_readings_packet_t
        {
            module_readings_data_t data;
            uint8_t bytes[sizeof(module_readings_data_t)];
        };

        module_settings_packet_t settings_packet;
        module_readings_packet_t readings_packet;
        uint32_t read_timestamp;
        uint32_t send_timestamp;
        reading_structure_t r_temperature;
        reading_structure_t r_pressure;
        reading_structure_t r_humidity;
};

#endif /* MODULE_MICROCLIMATE_h */
