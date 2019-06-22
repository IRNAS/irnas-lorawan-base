#ifndef STATUS_H_
#define STATUS_H_

#include "Arduino.h"
#include "TimerMillis.h"
#include <STM32L0.h>
#include "lorawan.h"
#include "project_utils.h"
#include "settings.h"

extern boolean status_send_flag;

/**
 * @brief LoraWAN status packet setup - port 2
 * 
 * @details Packet reporting system staus
 * resetCause - details source of last reset, see decoder.js for meaning
 * mode - what operaiton mode is the device in
 * battery - battery status in %
 * temperature - temperature
 * vbus - voltage
 * system_functions_errors - errors of different modules, only applicable if respective module is enabled
 *    bit 0 - gps periodic error
 *    bit 1 - gps triggered error
 *    bit 2 - gps fix error
 *    bit 3 - accelerometer error
 *    bit 4 - light sensor error
 *    bit 5 - temperature error
 *    bit 6 - humidity sensor error
 *    bit 7 - pressure sensor error
 */
struct statusData_t{
  uint8_t resetCause;
  uint8_t mode;
  uint8_t battery;
  uint8_t temperature;
  uint8_t vbus;
  uint8_t system_functions_errors;
}__attribute__((packed));

union statusPacket_t{
  statusData_t data;
  byte bytes[sizeof(statusData_t)];
};

static const uint8_t status_packet_port = 2;
extern statusPacket_t status_packet;

void status_scheduler(void);
void status_init(void);
boolean status_send(void);

#endif