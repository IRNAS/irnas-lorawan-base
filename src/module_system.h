#ifndef MODULE_SYSTEM_h
#define MODULE_SYSTEM_h

#include <Arduino.h>
#include "project.h"
#include "project_utils.h"
#include "STM32L0.h"
#include "module.h"

class MODULE_SYSTEM 
{
    public:

        // functions
        uint8_t configure(uint8_t *data, size_t *size);
        uint8_t get_settings_length();
        uint8_t set_downlink_data(uint8_t *data, size_t *size);
        module_flags_e scheduler(void);
        uint8_t initialize(void);
        uint8_t send(uint8_t *buffer, size_t *size);
        uint8_t read(void);
        uint8_t running(void);
        void event(event_e event);
        void print_data(void);

        // variables
        String name = "system";
        module_flags_e flags=M_ERROR;

        // parameters
        uint8_t param_a = 0;
        uint8_t param_b = 0;
        uint8_t param_c = 0;
    private:

        // add
        struct module_settings_data_t{
            uint8_t  global_id;
            uint8_t  length;
            uint16_t read_interval; // in seconds
            uint16_t send_interval; // in minutes
        }__attribute__((packed));

        union module_settings_packet_t{
            module_settings_data_t data;
            uint8_t bytes[sizeof(module_settings_data_t)];
        };

        // we are sending 13 bytes anyhow as header, filling up the packet does not matter too much
        struct module_readings_data_t{
            uint8_t reset_cause; // simple value expected to never change unless watchdog event occurs
            uint8_t vbus; // should be always the same, we can drop it
            uint8_t battery_avg; // average of readings during a period, maximally once a day
            uint8_t battery_min; // min of readings during a period, maximally once a day
            uint8_t battery_max; // max of readings during a period, maximally once a day
            uint8_t input_analog_avg; // average of readings during a period, maximally once a day
            uint8_t input_analog_min; // min of readings during a period, maximally once a day
            uint8_t input_analog_max; // max of readings during a period, maximally once a day
            uint8_t temperature_avg; // average of readings during a period, maximally once a day
            uint8_t temperature_min; // min of readings during a period, maximally once a day
            uint8_t temperature_max; // max of readings during a period, maximally once a day
            uint8_t errors[10];
        }__attribute__((packed));

        union module_readings_packet_t{
            module_readings_data_t data;
            uint8_t bytes[sizeof(module_readings_data_t)];
        };

        module_settings_packet_t settings_packet;
        module_readings_packet_t readings_packet;
        unsigned long read_timestamp;
        unsigned long send_timestamp;
        reading_structure_t r_battery;
        reading_structure_t r_analog_input;
        reading_structure_t r_temperature;
};

#endif /* MODULE_SYSTEM_h */
