#ifndef MODULE_ACCLEROMETER_h
#define MODULE_ACCLEROMETER_h

#include <Arduino.h>
#include "project.h"
#include "project_utils.h"
#include "STM32L0.h"
#include "module.h"
#include "LIS2DW12.h"

class MODULE_ACCELEROMETER 
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
        module_flags_e flags = M_ERROR;

        // parameters
        uint8_t param_a = 0;
        uint8_t param_b = 0;
        uint8_t param_c = 0;
    private:

        // add
        struct module_settings_data_t
        {
            uint8_t global_id;
            uint8_t length;
            uint16_t read_interval;
            uint16_t send_interval;
            uint8_t triggered_threshold;
            uint8_t triggered_duration;
            uint8_t free_fall;
            uint8_t dummy;
        }__attribute__((packed));

        union module_settings_packet_t
        {
            module_settings_data_t data;
            uint8_t bytes[sizeof(module_settings_data_t)];
        };

        // we are sending 13 bytes anyhow as header, filling up the packet does not matter too much
        struct module_readings_data_t
        {
            uint8_t accel_x_min;
            uint8_t accel_x_max;
            uint8_t accel_x_avg;
            uint8_t accel_y_min;
            uint8_t accel_y_max;
            uint8_t accel_y_avg;
            uint8_t accel_z_min;
            uint8_t accel_z_max;
            uint8_t accel_z_avg;
            uint8_t trig;
            uint8_t dummy;
        }__attribute__((packed));

        union module_readings_packet_t
        {
            module_readings_data_t data;
            uint8_t bytes[sizeof(module_readings_data_t)];
        };

        module_settings_packet_t settings_packet;
        module_readings_packet_t readings_packet;
        uint32_t read_timestamp;
        uint32_t  send_timestamp;
        reading_structure_t r_accel_x;
        reading_structure_t r_accel_y;
        reading_structure_t r_accel_z;

        LIS2DW12CLASS lis;
};

#endif /* MODULE_ACCLEROMETER_h */
