#ifndef RTC_PROJECT_H
#define RTC_PROJECT_H

#include <stdint.h>
#include <time.h>
#include "project_utils.h"

#define TIME_INIT_VALUE             (1577836800UL)  // Initial Time is Mon, 1 Jan 2020 00:00:00

#define ISL1208_ADDRESS   0x6F  //I2C slave addess of RTC IC

#define ISL1208_SC     0x00  //seconds register
#define ISL1208_MN     0x01  //minutes register
#define ISL1208_HR     0x02  //hours register
#define ISL1208_DT     0x03  //date register
#define ISL1208_MO     0x04  //month register
#define ISL1208_YR     0x05  //year register
#define ISL1208_DW     0x06  //day of the week register

#define ISL1208_SR     0x07  //status register
#define ISL1208_INT    0x08  //interrupt register
#define ISL1208_ATR    0x0A  //analog trimming register
#define ISL1208_DTR    0x0B  //digital trimming register

#define ISL1208_SCA    0x0C  //alarm seconds register
#define ISL1208_MNA    0x0D  //alarm minutes register
#define ISL1208_HRA    0x0E  //alarm hours register
#define ISL1208_DTA    0x0F  //alarm date register
#define ISL1208_MOA    0x10  //alarm month register
#define ISL1208_DWA    0x11  //alarm day of the week register

#define ISL1208_USR1    0x12  //user memory 1
#define ISL1208_USR2    0x13  //user memory 2

void rtc_init();
bool rtc_time_sync(time_t time_received, bool force);
time_t rtc_time_read();
void rtc_time_write(time_t t);
bool rtc_present();
char read8(char addr, char reg);
void write8(char addr, char reg, char data);

#endif /* RTC_PROJECT_H */
/*** end of file ***/