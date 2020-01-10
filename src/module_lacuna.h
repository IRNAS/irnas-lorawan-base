#ifndef MODULE_LACUNA_h
#define MODULE_LACUNA_h

#include <Arduino.h>
#include <LibLacuna.h>
#include "project.h"
#include "project_utils.h"
#include "STM32L0.h"
#include "module.h"

class MODULE_LACUNA
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
        }__attribute__((packed));

        union module_settings_packet_t
        {
            module_settings_data_t data;
            uint8_t bytes[sizeof(module_settings_data_t)];
        };

        // We probably don't need this
        struct module_readings_data_t
        {
        }__attribute__((packed));

        union module_readings_packet_t
        {
            module_readings_data_t data;
            uint8_t bytes[sizeof(module_readings_data_t)];
        };

        module_settings_packet_t settings_packet;
        module_readings_packet_t readings_packet;

        lsLoraWANParams loraWANParams;
        lsLoraSatTxParams SattxParams;
        lsLoraTxParams txParams;
};

#endif /* MODULE_LACUNA_h */
