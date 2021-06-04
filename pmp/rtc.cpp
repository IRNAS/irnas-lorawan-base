#include "rtc.h"
#include "debug.h"

#ifdef  RTC_DEBUG
#define serial_debug  Serial
#endif

/*
 * @brief Intializes rtc and sets init time value
 *
 * @return none (void)
 */
void rtc_init()
{
#ifdef serial_debug
    serial_debug.println("rtc_init()");
#endif
    time_t t = TIME_INIT_VALUE;
    Wire.begin();

    //set the WRTC (Write RTC Enable Bit) bit to 1 to enable the RTC
    //only then the RTC start counting
    Wire.beginTransmission(ISL1208_ADDRESS);
    Wire.write(ISL1208_SR); //status register
    Wire.write(0x10); //enable WRTC 
    Wire.endTransmission();

    //Try to open the ISL1208, checks if the RTC is available on the I2C bus
    if (rtc_present())
    {
#ifdef serial_debug
        serial_debug.println("rtc_init(RTC detected!)");
#endif
        //Check if we need to reset the time
        uint8_t powerFailed = read8(ISL1208_ADDRESS, ISL1208_SR); 
        if (powerFailed & 0x01)
        {
#ifdef serial_debug
            //The time has been lost due to a power complete power failure
            serial_debug.println("rtc_init(RTC has lost power! Resetting time...)");
#endif
            //Set RTC time to Mon, 1 Jan 2018 00:00:00
            rtc_time_write(t);
        }
#ifdef serial_debug
        time_t time = rtc_time_read();
        serial_debug.print("Time in string: ");
        serial_debug.print(ctime(&time));
        serial_debug.print("Time in epoch: ");
        serial_debug.println(time);
#endif
    }
    else
    {
#ifdef serial_debug
        serial_debug.println("RTC not detected!");
#endif
    }
}

/**
 * @brief sync rtc time to external source if new time is newer then existing or forced
 * 
 * @param time_received 
 * @return bool 
 */
bool rtc_time_sync(time_t time_received, bool force)
{
    // if local time is greater then received time, reject unless forced
    if (rtc_time_read() > time_received && (false == force))
    {
        return false;
    }
    rtc_time_write(time_received);
#ifdef serial_debug
    time_t time = rtc_time_read();
    serial_debug.print("rtc_time_updated(");
    serial_debug.print(ctime(&time));
    serial_debug.print(" ");
    serial_debug.print(time);
    serial_debug.println(")");
#endif
    return true;
}

/**
 * @brief Reads time from RTC
 *
 * @return time_t
 */
time_t rtc_time_read()
{
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
void rtc_time_write(time_t t)
{
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
 * @brief check if RTC is present
 * 
 * @return true 
 * @return false 
 */
bool rtc_present() 
{
    Wire.beginTransmission(ISL1208_ADDRESS); //send the address
    byte error = Wire.endTransmission(); //read ACK

    if (error == 0) 
    { 
        //if RTC is available
        return true;
    }
    return false;
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
/*** end of file ***/
