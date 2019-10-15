#ifndef MODULE_GPS_UBLOX_h
#define MODULE_GPS_UBLOX_h

#include <Arduino.h>
#include "board.h"
#include "project_utils.h"
#include "STM32L0.h"
#include "GNSS.h"
#include "module.h"

class MODULE_GPS_UBLOX
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
        void running(void);

        void print_data(void);

        // variables
        String name = "gps";
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
            uint16_t send_interval;         // in minutes
            uint16_t periodic_interval;    // in seconds
            uint16_t triggered_interval;    // in seconds
            uint8_t  gps_triggered_threshold;
            uint8_t  gps_triggered_duration;
            uint16_t gps_cold_fix_timeout;
            uint16_t gps_hot_fix_timeout;
            uint8_t  gps_min_fix_time;
            uint8_t  gps_min_ehpe;
            uint8_t  gps_hot_fix_retry;
            uint8_t  gps_cold_fix_retry;
            uint8_t  gps_fail_retry;
            uint8_t  gps_settings;
        }__attribute__((packed));

        union module_settings_packet_t{
            module_settings_data_t data;
            uint8_t bytes[sizeof(module_settings_data_t)];
        };

        // we are sending 13 bytes anyhow as header, filling up the packet does not matter too much
        struct module_readings_data_t{
            uint8_t lat1;
            uint8_t lat2;
            uint8_t lat3;
            uint8_t lon1;
            uint8_t lon2;
            uint8_t lon3;
            uint16_t alt;
            uint8_t satellites_hdop;
            uint8_t time_to_fix;
            uint8_t epe;
            uint8_t snr;
            uint8_t status;
            uint8_t motion;
        }__attribute__((packed));

        union module_readings_packet_t{
            module_readings_data_t data;
            uint8_t bytes[sizeof(module_readings_data_t)];
        };
        module_settings_packet_t settings_packet;
        module_readings_packet_t readings_packet;
        unsigned long read_timestamp;
        unsigned long send_timestamp;

        boolean gps_send_flag = false; // extern
        boolean gps_done = false; // extern
        boolean gps_begin_happened = false;
        uint8_t gps_fail_count = 0;
        uint8_t gps_fail_fix_count = 0;
        uint8_t gps_response_fail_count = 0;
        boolean gps_hot_fix = false;
        unsigned long gps_event_last = 0;
        unsigned long gps_accelerometer_last = 0;
        unsigned long gps_fix_start_time = 0;
        unsigned long gps_timeout = 0;
        unsigned long gps_time_to_fix;

        //TimerMillis gps_timer_timeout;
        //TimerMillis gps_timer_response_timeout;

        GNSSLocation gps_location;
        GNSSSatellites gps_satellites;

        void gps_accelerometer_interrupt(void);
        boolean gps_busy_timeout(uint16_t timeout);
        void gps_power(boolean enable);
        void gps_backup(boolean enable);
        boolean gps_begin(void);
        boolean gps_start(void);
        static void gps_acquiring_callback(void);
        void gps_stop(void);
        void gps_end(void);
};

#endif /* MODULE_GPS_UBLOX_h */
