#ifndef PROJECT_H_
#define PROJECT_H_

/*
The board definition is defined per project and per module. Thus the pin names must be in format MODULE_NAME
 */

//include modules
//#include "module.h"
#include "module_system.h"
#include "module_gps_ublox.h"
#include "module_pira.h"
#include "module_accelerometer.h"

#define N_MODULES 3
#define N_MODULES_TOTAL N_MODULES+1

extern module *s_SYSTEM;
extern module *s_GPS;
extern module *s_PIRA;
extern module *modules[];

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

#define MODULE_ACCELEROMETER_INT1 -1

#define BOARD_LED PH0




#endif