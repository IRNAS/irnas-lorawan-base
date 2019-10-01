#ifndef SETTINGS_H_
#define SETTINGS_H_

#include "Arduino.h"
#include <STM32L0.h>
#include "lorawan.h"
#include "EEPROM.h"
#include "stm32l0_eeprom.h"

#define EEPROM_DATA_START_SETTINGS 0

extern boolean settings_updated;

/*
storage of settings is somewhat complex in this case, to allow certain parts of settings to be sent independently
settings port can receive information in the following configuration:
TLV
T - 1 byte type - corresponds to port
L - 1 byte length value - corresponds to the number of bytes in the settings
V - array of values of given length, mathing the settings struct/union length of a particular module
 */

/**
 * @brief LoraWAN settings packet setup - port 100
 */
struct settingsData_t{
  uint8_t   lorawan_datarate;
  uint8_t   lorawan_adr;
  uint8_t   lorawan_txp;
  uint8_t   resend_delay; // in minutes, 0 disables it
  uint8_t   resend_count; // in times to be resent, 0 disables it
}__attribute__((packed));

union settingsPacket_t{
  settingsData_t data;
  byte bytes[sizeof(settingsData_t)];
};

static const uint8_t settings_packet_port = 100;
extern settingsPacket_t settings_packet;
extern settingsPacket_t settings_packet_downlink;

uint8_t settings_get_packet_port(void);
void settings_init(void);
void settings_from_downlink(void);
boolean settings_send(void);

#endif