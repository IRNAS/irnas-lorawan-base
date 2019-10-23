#ifndef BOARD_H_
#define BOARD_H_

/*
The board definition is defined per project and per module. Thus the pin names must be in format MODULE_NAME
 */

// MODULE_SYSTEM
#define MODULE_SYSTEM_BAN_MON_EN -1
#define MODULE_SYSTEM_BAN_MON_AN PA4
#define MODULE_GPS_EN PB5
#define MODULE_GPS_BCK -1
#define MODULE_GPS_SERIAL Serial2

#define MODULE_PIRA_SERIAL Serial1
#define MODULE_PIRA_5V PB6
#define MODULE_PIRA_STATUS PA11

#endif