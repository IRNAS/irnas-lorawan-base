#ifndef MODULE_PIRA_h
#define MODULE_PIRA_h

#include <Arduino.h>
#include "board.h"
#include "project_utils.h"
#include "STM32L0.h"
#include "module.h"
#include "Wire.h"
#include "rtc.h"

#define ON_PERIOD_INIT_VALUE_s      (7200)
#define OFF_PERIOD_INIT_VALUE_s     (7200)
#define RX_BUFFER_SIZE              (7)             // Size in B, do not change, comunication protocol between Pira and RPi depends on this
#define WATCHDOG_RESET_VALUE_s      (15000)
#define REBOOT_TIMEOUT_s            (60)

class MODULE_PIRA 
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

        void print_data(void);

        // variables
        String name = "pira";
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
            uint16_t status_battery;
            uint32_t safety_power_period;
            uint32_t safety_sleep_period;
            uint32_t safety_reboot;
            uint32_t operational_wakeup;
        }__attribute__((packed));

        union module_settings_packet_t{
            module_settings_data_t data;
            uint8_t bytes[sizeof(module_settings_data_t)];
        };

        // we are sending 13 bytes anyhow as header, filling up the packet does not matter too much
        struct module_readings_data_t{
            uint16_t empty_space; // RPi disk space value
            uint16_t photo_count; // RPi photo count
            uint64_t status_time; // system time
            uint16_t error_values;
        }__attribute__((packed));

        union module_readings_packet_t{
            module_readings_data_t data;
            uint8_t bytes[sizeof(module_readings_data_t)];
        };

        module_settings_packet_t settings_packet;
        module_readings_packet_t readings_packet;
        unsigned long read_timestamp;
        unsigned long send_timestamp;

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
         *      state
         *          It keeps the current state of state machine
         *      state_prev
         *          It keeps the previous state of state machine
         *      state_goto_timeout
         *          It keeps state that should be entered in case of time out.
         *          It is set everytime when we enter state.
         *      pira_elapsed
         *          It keeps how much time in ms pira_elapsed since we entered a state
         *      stateTimeoutDuration
         *          If pira_elapsed is larger than stateTimeoutStart then state timeouted.
         *          It is set everytime when we enter state.
         *      stateTimeoutStart
         *          Set everytime we call pira_state_transition funtion.
         */
        state_pira_e status_pira_state_machine;
        state_pira_e state_prev;
        state_pira_e state_goto_timeout;
        uint32_t pira_elapsed;
        uint32_t stateTimeoutDuration;
        uint32_t stateTimeoutStart;

        // Uart related functions
        void uart_receive();
        void uart_command_parse(uint8_t *rxBuffer);
        void uart_command_send(char command, uint32_t data);
        void uart_command_receive(void);
        void send_status_values(void);
        void print_status_values(void);
        uint32_t get_overview_value(void);
        void pira_state_transition(state_pira_e next);
        bool pira_state_check_timeout(void);
        char* return_state(state_pira_e state);
        void pira_state_machine();
};

#endif /* MODULE_PIRA_h */
