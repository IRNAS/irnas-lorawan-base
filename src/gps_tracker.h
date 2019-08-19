#ifndef GPS_TRACKER_H_
#define GPS_TRACKER_H_

#include "Arduino.h"
#include <TimerMillis.h>
#include <STM32L0.h>
#include "lorawan.h"
#include "settings.h"
#include "status.h"
#include "GNSS.h"
#include "board.h"
#include "Wire.h"

#define LIS2DH12_ADDR 0x19
#define LIS2DW12_WHO_AM_I                    0x0FU
#define LIS2DW12_CTRL1                       0x20U
#define LIS2DW12_CTRL2                       0x21U
#define LIS2DW12_CTRL4_INT1_PAD_CTRL         0x23U
#define LIS2DW12_CTRL6                       0x25U
#define LIS2DW12_WAKE_UP_THS                 0x34U   
#define LIS2DW12_WAKE_UP_DUR                 0x35U
#define LIS2DW12_CTRL7                       0x3FU

/**
 * @brief LoraWAN gps packet setup - port 1
 * 
 */
struct gpsData_t{
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
  uint8_t lux;
  uint8_t motion;
}__attribute__((packed));

union gpsPacket_t{
  gpsData_t data;
  byte bytes[sizeof(gpsData_t)];
};

static const uint8_t gps_packet_port = 1;
extern gpsPacket_t gps_packet;

extern boolean gps_send_flag;
extern boolean gps_done;

void gps_accelerometer_interrupt(void);
void gps_scheduler(void);
boolean gps_busy_timeout(uint16_t timeout);
void gps_power(boolean enable);
void gps_backup(boolean enable);
boolean gps_begin(void);
boolean gps_start(void);
void gps_acquiring_callback(void);
void gps_stop(void);
void gps_end(void);
void accelerometer_init(void);
boolean gps_send(void);
void writeReg(uint8_t reg, uint8_t val);

#endif