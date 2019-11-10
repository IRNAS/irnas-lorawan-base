#include "rtc.h"

#define serial_debug Serial

/**
 * @brief RTC object, used to read and write time to rtc chip 
 */
ISL1208_RTC rtc; 

/*
 * @brief Intializes rtc and sets init time value
 *
 * @return none (void)
 */
void rtc_init(){
    #ifdef serial_debug
        serial_debug.println("rtc_init()");
    #endif
    time_t t = TIME_INIT_VALUE;
    Wire.begin();
    rtc.begin();

    //Try to open the ISL1208,checks if the RTC is available on the I2C bus
    if(rtc.isRtcActive()){
        #ifdef serial_debug
        serial_debug.println("RTC detected!");
        #endif
        //Check if we need to reset the time
        uint8_t powerFailed = read8(ISL1208_ADDRESS, ISL1208_SR); 
        if(powerFailed & 0x01){
            #ifdef serial_debug
            //The time has been lost due to a power complete power failure
            serial_debug.println("RTC has lost power! Resetting time...");
            #endif
            //Set RTC time to Mon, 1 Jan 2018 00:00:00
            rtc_time_write(t);
        }
    }
    else{
        #ifdef serial_debug
        serial_debug.println("RTC not detected!");
        #endif
    }
}

/**
 * @brief Reads time from RTC
 *
 * @return time_t
 */
time_t rtc_time_read(){
    //Setup a tm structure based on the RTC
    struct tm timeinfo;
    timeinfo.tm_sec = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_SC));
    timeinfo.tm_min = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_MN));

    //Make sure we get the proper hour regardless of the mode
    char hours = read8(ISL1208_ADDRESS, ISL1208_HR);
    if (hours & (1 << 7))
    {
        //RTC is in 24-hour mode
        timeinfo.tm_hour = bcd2bin(hours & 0x3F);
    }
    else
    {
        //RTC is in 12-hour mode
        timeinfo.tm_hour = bcd2bin(hours & 0x1F);

        //Check for the PM flag
        if (hours & (1 << 5))
        {
            timeinfo.tm_hour += 12;
        }
    }

    //Continue reading the registers
    timeinfo.tm_mday = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_DT));
    timeinfo.tm_mon  = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_MO)) - 1;
    timeinfo.tm_year = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_YR)) + 100;
    timeinfo.tm_wday = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_DW));

    //Return as a timestamp
    return mktime(&timeinfo);
}

/**
 * @brief Writes time to RTC
 *
 * @return none (void)
 */
void rtc_time_write(time_t t){
    //Convert the time to a tm
    struct tm *timeinfo = localtime(&t);

    /* The clock has an 8 bit wide bcd-coded register (they never learn)
     * for the year. tm_year is an offset from 1900 and we are interested
     * in the 2000-2099 range, so any value less than 100 is invalid.
     */
    if (timeinfo->tm_year < 100)
    {
        return;
    }

    //Read the old SR register value
    char sr = read8(ISL1208_ADDRESS, ISL1208_SR);

    //Enable RTC writing
    write8(ISL1208_ADDRESS, ISL1208_SR, sr | (1 << 4));

    //Write the current time
    write8(ISL1208_ADDRESS, ISL1208_SC, bin2bcd(timeinfo->tm_sec));
    write8(ISL1208_ADDRESS, ISL1208_MN, bin2bcd(timeinfo->tm_min));
    write8(ISL1208_ADDRESS, ISL1208_HR, bin2bcd(timeinfo->tm_hour) | (1 << 7));
    write8(ISL1208_ADDRESS, ISL1208_DT, bin2bcd(timeinfo->tm_mday));
    write8(ISL1208_ADDRESS, ISL1208_MO, bin2bcd(timeinfo->tm_mon + 1));
    write8(ISL1208_ADDRESS, ISL1208_YR, bin2bcd(timeinfo->tm_year - 100));
    write8(ISL1208_ADDRESS, ISL1208_DW, bin2bcd(timeinfo->tm_wday & 7));

    //Disable RTC writing
    write8(ISL1208_ADDRESS, ISL1208_SR, sr);
}

/**
 * @brief Reads a register over I2C
 *
 * @param[in] addr
 * @param[in] reg
 *
 * @return char
 */
char read8(char addr, char reg)
{
    //Select the register
    Wire.beginTransmission(addr); //send I2C address of RTC
    Wire.write(reg); //status register
    Wire.endTransmission();

    //Read the 8-bit register
    Wire.requestFrom(addr, 1); // now get the bytes of data...

    //Return the byte
    return Wire.read();
}


/**
 * @brief Writes to a register over I2C
 *
 * @param[in] addr
 * @param[in] reg
 * @param[in] data
 *
 * @return none (void)
 */
void write8(char addr, char reg, char data)
{
    //Select the register
    Wire.beginTransmission(addr); //send I2C address of RTC
    Wire.write(reg); //status register

    //Write to register
    Wire.write(data);
    Wire.endTransmission();
}