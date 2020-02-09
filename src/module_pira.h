#ifndef MODULE_PIRA_H
#define MODULE_PIRA_H

#include <Arduino.h>
#include "project.h"
#include "project_utils.h"
#include <STM32L0.h>
#include "module.h"
#include "Wire.h"
#include "rtc.h"

#define RX_BUFFER_SIZE              (7)             // Size in B, do not change, comunication protocol between Pira and RPi depends on this

class MODULE_PIRA 
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

        specific_public_data_t getter();

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
            uint8_t  global_id;
            uint8_t  length;
            uint16_t read_interval; // in seconds
            uint16_t send_interval; // in minutes
            uint32_t safety_power_period;
            uint32_t safety_sleep_period;
            uint32_t safety_reboot;
            uint32_t operational_wakeup;
        }__attribute__((packed));

        union module_settings_packet_t
        {
            module_settings_data_t data;
            uint8_t bytes[sizeof(module_settings_data_t)];
        };

        // we are sending 13 bytes anyhow as header, filling up the packet does not matter too much
        struct module_readings_data_t
        {
            uint16_t empty_space;       // RPi disk space value
            uint16_t photo_count;       // RPi photo count
            uint32_t status_time;       // system time, only 4 bytes are needed, time_t is 4 bytes long
            uint16_t next_wakeup;       // When will next wake up happen in seconds
            uint16_t cycle_duration;    // How long is rpi power pin held high in milliseconds
            uint16_t error_values;
        }__attribute__((packed));

        union module_readings_packet_t
        {
            module_readings_data_t data;
            uint8_t bytes[sizeof(module_readings_data_t)];
        };

        module_settings_packet_t settings_packet;
        module_readings_packet_t readings_packet;
        specific_public_data_t public_data;

        uint32_t read_timestamp;
        uint32_t send_timestamp;

        // Pira enumerated state variable
        enum state_pira_e
        {
            IDLE_PIRA,
            START_PIRA,
            WAIT_STATUS_ON,
            WAKEUP,
            REBOOT_DETECTION,
            STOP_PIRA,
        };

        /**
         * @brief Variables concerning the state of the program
         * @detail
         *      status_pira_state_machine
         *          It keeps the current state of state machine
         *      state_prev
         *          It keeps the previous state of state machine
         *      state_goto_timeout
         *          It keeps state that should be entered in case of time out.
         *          It is set everytime when we enter state.
         *      pira_elapsed
         *          It keeps how much time in ms pira_elapsed since we entered a state
         *      rpi_turned_off_timestamp
         *          Used to keep track of when pira module turns off RPI, at beginning set to 0 to start pira fsm at boot
         *      stateTimeoutDuration
         *          If pira_elapsed is larger than stateTimeoutStart then state timeouted.
         *          It is set everytime when we enter state.
         *      stateTimeoutStart
         *          Set everytime we call pira_state_transition funtion.
         *      rpi_power_pin_pulled_high
         *          Used to calculate cycle_duration
         *      rpi_power_pin_pulled_low
         *          Used to calculate cycle_duration
         */
        state_pira_e status_pira_state_machine;
        state_pira_e state_prev;
        state_pira_e state_goto_timeout;
        uint32_t pira_elapsed;
        uint32_t rpi_turned_off_timestamp;
        uint32_t stateTimeoutDuration;
        uint32_t stateTimeoutStart;
        uint32_t rpi_power_pin_pulled_high;
        uint32_t rpi_power_pin_pulled_low;

        // Uart related functions
        void uart_command_parse(uint8_t * rxBuffer);
        void uart_command_send(char command, uint32_t data);
        void uart_command_receive(void);
        void send_status_values(void);
        void print_status_values(void);
        uint32_t get_overview_value(void);
        void pira_state_transition(state_pira_e next);
        bool pira_state_check_timeout(void);
        char * return_state(state_pira_e state);
        void pira_state_machine();
        char * decode_flag(module_flags_e flag);
};

#endif /* MODULE_PIRA_H */
/*** end of file ***/