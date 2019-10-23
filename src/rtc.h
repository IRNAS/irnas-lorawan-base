#ifndef RTC_PROJECT_H
#define RTC_PROJECT_H

#include "ISL1208_RTC.h"
#include <stdint.h>
#include <time.h>
#include "project_utils.h"

#define TIME_INIT_VALUE             (1514764800UL)  // Initial Time is Mon, 1 Jan 2018 00:00:00

void rtc_init();
time_t rtc_time_read();
void rtc_time_write(time_t t);
char read8(char addr, char reg);
void write8(char addr, char reg, char data);

#endif