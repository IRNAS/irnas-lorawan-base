#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Arduino.h"
#include <STM32L0.h>
#include "lorawan.h"
#include "EEPROM.h"
#include "stm32l0_eeprom.h"

#define EEPROM_DATA_START_SETTINGS 0

extern boolean settings_updated;

/**
 * @brief LoraWAN settings packet setup - port 3
 * 
 * @details Packet variables have the following information contained:
 * system_status_interval -  send interval in minutes, range 1 - 1440, default 60
 * system_functions - enable/disable certain features
 *    bit 0 - 
 *    bit 1 - 
 *    bit 2 - 
 *    bit 3 - accelerometer enabled
 *    bit 4 - light sensor enabled
 *    bit 5 - temperature sensor enabled
 *    bit 6 - humidity sensor enabled
 *    bit 7 - pressure sensor enabled
 * lorawan_datarate_adr - lorawan fix reporting datarate, range 0 - 5 (SF7-SF12), upper nibble is adr, lower nibble is datarate
 * gps_periodic_interval - gps periodic fix interval in minutes
 * gps_triggered_interval - gps triggered interval in minutes
 * gps_triggered_threshold - threshold of accelerometer to trigger a fix
 * gps_triggered_duration - number of accelerometer samples for activity
 * gps_cold_fix_timeout - cold fix timeout in seconds
 * gps_hot_fix_timeout - hot fix timeout in seconds
 * gps_min_fix_time - minimal fix time
 * gps_min_ehpe - minimal ehpe to be achieved
 * gps_hot_fix_retry - number of times a hot fix is retried before failing to cold-fix
 * gps_cold_fix_retry - number of time a cold fix is retried before failing the gps module
 * gps_fail_retry - number of times gps system is retried before putting it in failed state
 * gps_settings -
 *  bit 0 - 3d fix enabled
 *  bit 1 - linear backoff upon fail (based on interval time)
 *  bit 2 - hot fix enabled
 *  bit 3 - fully resolved required
 */
struct settingsData_t{
  uint16_t  system_status_interval;
  uint8_t   system_functions;
  uint8_t   lorawan_datarate_adr;
  uint16_t  gps_periodic_interval;
  uint16_t  gps_triggered_interval;
  uint8_t   gps_triggered_threshold;
  uint8_t   gps_triggered_duration;
  uint16_t  gps_cold_fix_timeout;
  uint16_t  gps_hot_fix_timeout;
  uint8_t   gps_min_fix_time;
  uint8_t   gps_min_ehpe;
  uint8_t   gps_hot_fix_retry;
  uint8_t   gps_cold_fix_retry;
  uint8_t   gps_fail_retry;
  uint8_t   gps_settings;
}__attribute__((packed));

union settingsPacket_t{
  settingsData_t data;
  byte bytes[sizeof(settingsData_t)];
};

static const uint8_t settings_packet_port = 3;
extern settingsPacket_t settings_packet;
extern settingsPacket_t settings_packet_downlink;

uint8_t settings_get_packet_port(void);
void settings_init(void);
void settings_from_downlink(void);
boolean settings_send(void);

#endif