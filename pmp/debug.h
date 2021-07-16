#ifndef DEBUG_H
#define DEBUG_H

/*
This header file is supposed to be included in any c, cpp file that uses debugging output 
through UART. 
Uncommenting a define will turn on debugging output in that particular file. 
*/

#define MAIN_DEBUG

#define MODULE_ACCELEROMETER_DEBUG
#define MODULE_GPS_UBLOX_DEBUG
#define MODULE_LACUNA_DEBUG
#define MODULE_PIRA_DEBUG
#define MODULE_SYSTEM_DEBUG

#define PROJECT_UTILS_DEBUG
#define RTC_DEBUG
#define SETTINGS_DEBUG
#define LORAWAN_DEBUG


#endif  /* DEBUG_H */
/*** end of file ***/
