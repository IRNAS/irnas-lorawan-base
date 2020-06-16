#ifndef PROJECT_H
#define PROJECT_H

/*
   The board definition is defined per project and per module. Thus the pin names must be in format MODULE_NAME
   */

//include modules
#include "module_system.h"
#include "module_gps_ublox.h"
#include "module_pira.h"
#include "module_accelerometer.h"
#include "module_lacuna.h"

#define N_MODULES 4
#define N_MODULES_TOTAL N_MODULES+1

extern module *s_SYSTEM;
extern module *s_GPS;
extern module *s_PIRA;
extern module *s_LACUNA;
extern module *modules[];

// Set here the project
#define  PMP_v1

#ifdef RHINO_v2_4
#define MODULE_SYSTEM_BAN_MON_EN PH1
#define MODULE_SYSTEM_BAN_MON_AN PA4
#define MODULE_SYSTEM_LIGHT_EN PB14

#define MODULE_GPS_EN PB6
#define MODULE_GPS_BCK PA8
#define MODULE_GPS_SERIAL Serial1

#define MODULE_5V_EN -1
#define MODULE_PIRA_SERIAL Serial1
#define MODULE_PIRA_5V -1
#define MODULE_PIRA_STATUS -1
#define MODULE_PIRA_UNDERVOLTAGE_THRESHOLD 3100 // In mV

#define MODULE_ACCELEROMETER_INT1 PB2
#define MODULE_ACCELEROMETER_INT2 PB7

#define MODULE_VSWR_ADC PA3
#define MODULE_VSWR_EN PA5

#define BOARD_LED PA0
#define BOARD_REED PH0
#endif

#ifdef PMP_v1
// MODULE_SYSTEM
#define MODULE_SYSTEM_BAN_MON_EN -1
#define MODULE_SYSTEM_BAN_MON_AN PA4
#define MODULE_GPS_EN PB5
#define MODULE_GPS_BCK -1
#define MODULE_GPS_SERIAL Serial2
#define MODULE_5V_EN PB6

#define MODULE_PIRA_SERIAL Serial1
#define MODULE_PIRA_5V PA8
#define MODULE_PIRA_STATUS PA11
#define MODULE_PIRA_UNDERVOLTAGE_THRESHOLD 3100 // In mV

#define MODULE_ACCELEROMETER_INT1 -1

#define MODULE_ULTRASONIC_OLED_3V PB7

#define MODULE_LACUNA_5V PH0

#define BOARD_LED PH0   //Do not use, as it is connected to lacuna power
#define BOARD_BUTTON PA5 
extern uint16_t global_activate_pira;
#endif


#ifdef LION_v2_3
#define MODULE_SYSTEM_BAN_MON_EN PH1
#define MODULE_SYSTEM_BAN_MON_AN PA4
#define MODULE_SYSTEM_LIGHT_EN PB14

#define MODULE_GPS_EN PB6
#define MODULE_GPS_BCK PA8
#define MODULE_GPS_SERIAL Serial1

#define MODULE_5V_EN -1
#define MODULE_PIRA_SERIAL Serial1
#define MODULE_PIRA_5V -1
#define MODULE_PIRA_STATUS -1
#define MODULE_PIRA_UNDERVOLTAGE_THRESHOLD 3100 // In mV

#define MODULE_ACCELEROMETER_INT1 PB2
#define MODULE_ACCELEROMETER_INT2 PB7

#define MODULE_VSWR_ADC PA3
#define MODULE_VSWR_EN PA5

#define BOARD_LED PA0
#define BOARD_REED PH0
#define BOARD_CHG_DISABLE PA11
#endif

#endif /* PROJECT_H */
/*** end of file ***/
