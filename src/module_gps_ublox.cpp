#include "module_gps_ublox.h"

#define NAME  "gps"
#define serial_debug Serial

uint8_t MODULE_GPS_UBLOX::configure(uint8_t *data, size_t *size)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": configure(");
    serial_debug.println(")");;
#endif

    if (* size != sizeof(module_settings_data_t))
    {
        return 0;
    }
    // copy to buffer
    module_settings_packet_t settings_packet_downlink;
    memcpy(&settings_packet_downlink.bytes[0],data, sizeof(module_settings_data_t));
    // validate settings value range 
    settings_packet.data.global_id = settings_packet_downlink.data.global_id;
    settings_packet.data.length = settings_packet_downlink.data.length;

    settings_packet.data.send_interval              = constrain(settings_packet_downlink.data.send_interval, 0, 24 * 60);
    settings_packet.data.periodic_interval          = constrain(settings_packet_downlink.data.periodic_interval, 0, 24 * 60);
    settings_packet.data.triggered_interval         = constrain(settings_packet_downlink.data.triggered_interval, 0, 24 * 60);
    settings_packet.data.gps_triggered_threshold    = constrain(settings_packet_downlink.data.gps_triggered_threshold, 0, 0x3f);
    settings_packet.data.gps_triggered_duration     = constrain(settings_packet_downlink.data.gps_triggered_duration, 0, 0xff);
    settings_packet.data.gps_cold_fix_timeout       = constrain(settings_packet_downlink.data.gps_cold_fix_timeout, 0, 600);
    settings_packet.data.gps_hot_fix_timeout        = constrain(settings_packet_downlink.data.gps_hot_fix_timeout, 0, 600);
    settings_packet.data.gps_min_fix_time           = constrain(settings_packet_downlink.data.gps_min_fix_time, 0, 60);
    settings_packet.data.gps_min_ehpe               = constrain(settings_packet_downlink.data.gps_min_ehpe, 0, 100);
    settings_packet.data.gps_hot_fix_retry          = constrain(settings_packet_downlink.data.gps_hot_fix_retry, 0, 0xff);
    settings_packet.data.gps_cold_fix_retry         = constrain(settings_packet_downlink.data.gps_cold_fix_retry, 0, 0xff);
    settings_packet.data.gps_fail_retry             = constrain(settings_packet_downlink.data.gps_fail_retry, 0, 0xff); //must be 1 due to bug in GPS core
    settings_packet.data.gps_settings               = constrain(settings_packet_downlink.data.gps_settings, 0, 0xff);
    // write to main settings

}

uint8_t MODULE_GPS_UBLOX::get_settings_length()
{
    return sizeof(module_settings_data_t);
}

uint8_t MODULE_GPS_UBLOX::set_downlink_data(uint8_t * data, size_t * size)
{
    return 0;
}

module_flags_e MODULE_GPS_UBLOX::scheduler(void)
{
    uint32_t interval = 0;

    // do not schedule a GPS event if it has failed more then the specified amount of times
    if (gps_fail_count > settings_packet.data.gps_fail_retry)
    {
        return flags;
    }

    // if triggered gps is enabled and accelerometer trigger has ocurred
    if (settings_packet.data.periodic_interval > 0)
    {
        interval = settings_packet.data.periodic_interval;
    }

    // if triggered gps is enabled and accelerometer trigger has ocurred - overrides periodic interval
    if (settings_packet.data.triggered_interval > 0)
    {
        if (((millis() - gps_accelerometer_last) / 1000) < settings_packet.data.triggered_interval)
        {
            interval = settings_packet.data.triggered_interval;
        }
    }

    // linear backoff upon fail
    if (bitRead(settings_packet.data.gps_settings, 1))
    {
        interval = interval * (gps_fail_count + 1);
    }

    if ((interval != 0) && ((millis() - read_timestamp >= interval * 60 * 1000) ||  0 == read_timestamp))
    {
        if (M_IDLE == flags)
        {
            read_timestamp = millis();
            flags = M_READ;
#ifdef serial_debug
            serial_debug.print(NAME);
            serial_debug.print(":scheduler(");
            serial_debug.println("read_values)");
#endif
        }
        return flags;
    }

    uint32_t elapsed = millis() - send_timestamp;
    if ((settings_packet.data.send_interval != 0) & (elapsed >= (settings_packet.data.send_interval * 60 * 1000)))
    {
        if (M_IDLE == flags)
        {
            send_timestamp = millis();
            flags = M_SEND;
#ifdef serial_debug
            serial_debug.print(NAME);
            serial_debug.print("scheduler(");
            serial_debug.println("send_values)");
#endif
        }
        return flags;
    }
    return flags;
}

uint8_t MODULE_GPS_UBLOX::initialize(void)
{
    settings_packet.data.periodic_interval = 1;
    settings_packet.data.triggered_interval = 0;
    settings_packet.data.send_interval = 0;
    settings_packet.data.gps_triggered_threshold = 0;
    settings_packet.data.gps_triggered_duration = 0;
    settings_packet.data.gps_cold_fix_timeout = 120;
    settings_packet.data.gps_hot_fix_timeout = 60;
    settings_packet.data.gps_min_fix_time = 5;
    settings_packet.data.gps_min_ehpe = 40;
    settings_packet.data.gps_hot_fix_retry = 3;
    settings_packet.data.gps_cold_fix_retry = 6;
    settings_packet.data.gps_fail_retry = 0;
    settings_packet.data.gps_settings = 0xff;
    flags = M_IDLE;
}

uint8_t MODULE_GPS_UBLOX::send(uint8_t  * data, size_t  * size)
{
    //form the readings_packet

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": send(");
    serial_debug.println(")");
#endif

    memcpy(data, &readings_packet.bytes[0], sizeof(module_readings_data_t));
    *size = sizeof(module_readings_data_t);
    flags = M_IDLE;
    return 1;
}

void MODULE_GPS_UBLOX::event(event_e event)
{

}

void MODULE_GPS_UBLOX::print_data(void)
{

}

uint8_t MODULE_GPS_UBLOX::read(void)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": read(");
    serial_debug.println(")");
#endif

    gps_start();
    return 1;
}




/**
  GPS links:
https://portal.u-blox.com/s/question/0D52p00008HKCskCAH/time-to-acquire-gps-position-when-almanac-and-ephemeris-data-be-cleared-
https://portal.u-blox.com/s/question/0D52p00008HKDCuCAP/guarantee-the-next-start-is-a-hot-start

assist offline is enabled by:
https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/blob/18fb1cc81c6bc91b25e3346595f820985f2267e5/libraries/GNSS/src/utility/gnss_core.c#L2741
see page 52 of above document
check UBX-NAV-AOPSTATUS and hten disable GPS
check if manually disabling backup does loose gps moduel settings, consider using the provided library to do this and test correct operation

*/

/**
 * @brief updates last motion activity detected time upon interrup from acelerometer
 * 
 */
void MODULE_GPS_UBLOX::gps_accelerometer_interrupt(void)
{
    /*#ifdef serial_debug
      serial_debug.print("gps_accelerometer_interrupt(");
      serial_debug.println(")");
#endif*/
    gps_accelerometer_last = millis();
}

/**
 * @brief Checks if GPS is busy and times out after the specified time
 * 
 * @param timeout 
 * @return bool 
 */
bool MODULE_GPS_UBLOX::gps_busy_timeout(uint16_t timeout)
{
    for(uint16_t i = 0; i < timeout; i++)
    {
        if (!GNSS.busy())
        {
            return false;
        }
        delay(1);
    }
    return true;
}

/**
 * @brief Controls GPS pin for main power
 * 
 * @param enable 
 */
void MODULE_GPS_UBLOX::gps_power(bool enable)
{
    if (enable)
    {
        pinMode(MODULE_GPS_EN, OUTPUT);
        digitalWrite(MODULE_GPS_EN, HIGH);
    }
    else
    {
        digitalWrite(MODULE_GPS_EN, LOW);
        delay(100);
        pinMode(MODULE_GPS_EN, INPUT_PULLDOWN);
    }
}

/**
 * @brief COntrols GPS pin for backup power
 * 
 * @param enable 
 */
void MODULE_GPS_UBLOX::gps_backup(bool enable)
{
    if (enable)
    {
        pinMode(MODULE_GPS_BCK, OUTPUT);
        digitalWrite(MODULE_GPS_BCK, HIGH);
    }
    else
    {
        digitalWrite(MODULE_GPS_BCK, LOW);
        pinMode(MODULE_GPS_BCK, INPUT_PULLDOWN);
    }
}

/**
 * @brief Initializes GPS - to be called upon boot or if no backup power is available
 * 
 * @return bool - true if successful
 */
bool MODULE_GPS_UBLOX::gps_begin(void)
{
#ifdef serial_debug
    serial_debug.print("gps: begin(");
    serial_debug.println("started)");
    serial_debug.print("gps: settings(");
    serial_debug.print("per: ");
    serial_debug.print(settings_packet.data.periodic_interval);
    serial_debug.print(" trig: ");
    serial_debug.print(settings_packet.data.triggered_interval);
    serial_debug.print(" send: ");
    serial_debug.print(settings_packet.data.send_interval);
    serial_debug.print(" trig_th: ");
    serial_debug.print(settings_packet.data.gps_triggered_threshold);
    serial_debug.print(" trig_dur: ");
    serial_debug.print(settings_packet.data.gps_triggered_duration);
    serial_debug.print(" cold_t: ");
    serial_debug.print(settings_packet.data.gps_cold_fix_timeout);
    serial_debug.print(" hot_t: ");
    serial_debug.print(settings_packet.data.gps_hot_fix_timeout);
    serial_debug.print(" min_fix: ");
    serial_debug.print(settings_packet.data.gps_min_fix_time);
    serial_debug.print(" min_ehpe: ");
    serial_debug.print(settings_packet.data.gps_min_ehpe);
    serial_debug.print(" hot_ret: ");
    serial_debug.print(settings_packet.data.gps_hot_fix_retry);
    serial_debug.print(" cold_ret: ");
    serial_debug.print(settings_packet.data.gps_cold_fix_retry);
    serial_debug.print(" fail_ret: ");
    serial_debug.print(settings_packet.data.gps_fail_retry);
    serial_debug.print(" set: ");
    serial_debug.print(settings_packet.data.gps_settings);
    serial_debug.println(")");
#endif

    // check if more fails have occured then allowed
    if (gps_fail_count > settings_packet.data.gps_fail_retry)
    {
        return false;
    }

    // Step 1: power up the GPS and backup power
    gps_power(true);
    gps_backup(true);
    delay(1000); // wait for boot and power stabilization

    // Step 2: initialize GPS
    // Note: https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/issues/86
    // Note: https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/issues/90
    gps_busy_timeout(1000);
    GNSS.begin(MODULE_GPS_SERIAL, GNSS.MODE_UBLOX, GNSS.RATE_1HZ);
    gps_begin_happened = true;
    if (gps_busy_timeout(3000))
    {
        gps_end();
        gps_fail_count++;
#ifdef serial_debug
        serial_debug.println("fail after begin");
#endif
        return false;
    }

    // Step 3: configure GPS
    // see default config https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/blob/18fb1cc81c6bc91b25e3346595f820985f2267e5/libraries/GNSS/src/utility/gnss_core.c#L2904
    bool error = false;
    error |= gps_busy_timeout(1000);

    GNSS.setConstellation(GNSS.CONSTELLATION_GPS_AND_GLONASS);
    error |= gps_busy_timeout(1000);

    GNSS.disableWakeup();
    error |= gps_busy_timeout(1000);

    GNSS.setAutonomous(true);
    error |= gps_busy_timeout(1000);

    GNSS.setAntenna(GNSS.ANTENNA_INTERNAL);
    error |= gps_busy_timeout(1000);

    GNSS.setPlatform(GNSS.PLATFORM_PORTABLE);
    error |= gps_busy_timeout(1000);

    //GNSS.onLocation(gps_acquiring_callback);
    //GNSS.onLocation(Callback(&MODULE_GPS_UBLOX::running, this));
    error |= gps_busy_timeout(1000);

    if (error)
    {
        gps_end();
        gps_fail_count++;
#ifdef serial_debug
        serial_debug.println("fail at config");
#endif
        return false;
    }

    // GPS periodic and triggered error and fix
    bitClear(readings_packet.data.status, 0);
    bitClear(readings_packet.data.status, 1);
    bitSet(readings_packet.data.status, 2);

#ifdef serial_debug
    serial_debug.print("gps: begin(");
    serial_debug.println("done)");
#endif

    return true;
}

/**
 * @brief Starts the GPS acquisition and sets up timeouts
 * 
 * @return bool 
 */
bool MODULE_GPS_UBLOX::gps_start(void)
{
    flags = M_ERROR;
    bitSet(readings_packet.data.status, 2);

    // Step 0: Initialize GPS
    if (false == gps_begin_happened)
    {
        if (gps_begin() == false)
        {
            return false;
        }
    }

    //re-initialize upon fail
    else if (gps_fail_count > 0)
    {
        // This does not work currently due to a bug
        if (gps_begin() == false)
        {
            return false;
        }
    }

    // Step 1: Enable power
    gps_power(true);
    delay(100);

    // Step 2: Resume GPS
    gps_busy_timeout(1000);
    if (false == GNSS.resume())
    {
#ifdef serial_debug
        serial_debug.print("gps(resume failed ");
        serial_debug.println(")");
#endif
        gps_fail_count++;
        return false;
    }

    // Step 3: Start acquiring location
    gps_fix_start_time = millis();
    if (gps_hot_fix)
    {
        gps_timeout = settings_packet.data.gps_hot_fix_timeout * 1000;
    }
    else
    {
        gps_timeout = settings_packet.data.gps_cold_fix_timeout * 1000;
    }

    // Schedule automatic stop after timeout
    //gps_timer_timeout.start(gps_stop,gps_timeout+1000);
    // Schedule automatic stop after timeout if no messages received
    //gps_timer_response_timeout.start(gps_acquiring_callback,1000);
    gps_response_fail_count = 0;
    gps_done = false;
    flags = M_RUNNING;
    return true;
}

/**
 * @brief Callback triggered by the GPS module upon receiving a message from it
 * 
 */
void MODULE_GPS_UBLOX::running(void)
{
    // Restart automatic stop after timeout if no messages received
    //gps_timer_response_timeout.start(gps_acquiring_callback,1000);
    if ((millis() - gps_fix_start_time) > gps_timeout)
    {
        gps_stop();
    }
    else if(GNSS.location(gps_location))
    {
        float ehpe = gps_location.ehpe();
        bool gps_3D_fix_required = bitRead(settings_packet.data.gps_settings,0);
        bool gps_fully_resolved_required = bitRead(settings_packet.data.gps_settings,3);
        gps_response_fail_count = 0;

#ifdef serial_debug
        serial_debug.print("gps( ehpe ");
        serial_debug.print(ehpe);
        serial_debug.print(" sat ");
        serial_debug.print(gps_location.satellites());
        serial_debug.print(" d ");
        serial_debug.print(gps_location.fixType());
        serial_debug.print(" res ");
        serial_debug.print(gps_location.fullyResolved());
        serial_debug.print(" to ");
        serial_debug.print((millis() - gps_fix_start_time)/1000);
        serial_debug.print("/");
        serial_debug.print(gps_timeout/1000);
        serial_debug.println(" )");
#endif

        gps_time_to_fix = millis() - gps_fix_start_time; 

        if ((gps_location.fixType() >= GNSSLocation::TYPE_2D) 
                &&(gps_time_to_fix >= (settings_packet.data.gps_min_fix_time * 1000)))
        {
            // clear GPS fix error
            bitClear(readings_packet.data.status, 2);

            // wait until fix conditions are met
            if ((ehpe <= settings_packet.data.gps_min_ehpe) 
                    && (gps_location.fullyResolved() || false == gps_fully_resolved_required)
                    && ((gps_location.fixType() == GNSSLocation::TYPE_3D) || false == gps_3D_fix_required)
                    && (gps_time_to_fix > settings_packet.data.gps_min_fix_time))
            {
                if (bitRead(settings_packet.data.gps_settings, 2))
                {
                    // enable hot-fix immediately
                    gps_hot_fix = true;
                }
                gps_stop();
                flags = M_SEND; // flag the data to be redy for sending
            }
        }
    }
    else
    {
        gps_response_fail_count++;

        if (gps_response_fail_count > 10)
        {
            //raise error and stop
            gps_fail_count++;
#ifdef serial_debug
            serial_debug.print("gps_running_reponse(");
            serial_debug.println("fail)");
#endif
            gps_stop();
        }
    }
}

/**
 * @brief Reads the data from GPS and performs shutdown, called by either the timeout function or the acquiring callback upon fix
 * 
 */
void MODULE_GPS_UBLOX::gps_stop(void)
{
#ifdef serial_debug
    serial_debug.print("gps: stop(");
    serial_debug.println(")");
#endif

    flags = M_IDLE; // provisional flag

    gps_time_to_fix = millis() - gps_fix_start_time;

    // Power off GPS
    gps_power(false);
    if (!bitRead(settings_packet.data.gps_settings, 2))
    {
        // Disable GPS if hot-fix mechanism is not used
        gps_hot_fix = false;
    }

    // if GPS fix error
    if (bitRead(readings_packet.data.status, 2))
    {
        gps_fail_fix_count++;
        if ((true == gps_hot_fix) 
                && (gps_fail_fix_count >= settings_packet.data.gps_hot_fix_retry))
        {
            //disable hot-fix and rest fail counter
            gps_hot_fix = false;
            gps_fail_fix_count = 0;
        }
        else if ((gps_fail_fix_count >= settings_packet.data.gps_cold_fix_retry)
                && (255 != settings_packet.data.gps_cold_fix_retry))
        {
            gps_fail_count++;
        }
    }
    else
    {
        gps_fail_fix_count = 0;
        gps_fail_count = 0;
    }
    gps_done = true;

    if (gps_fail_count > 0)
    {
        gps_hot_fix = false;
        gps_end();
        return; // continuing fails otherwise by reading invalid data locaitons
    }

    GNSS.satellites(gps_satellites);
    uint8_t max_snr = 0;
    for (unsigned int index = 0; index < gps_satellites.count(); index++)
    {
        if (max_snr < gps_satellites.snr(index))
        {
            max_snr = gps_satellites.snr(index);
        }
    }

    struct tm timeinfo;
    timeinfo.tm_sec = gps_location.seconds();
    timeinfo.tm_min = gps_location.minutes();
    timeinfo.tm_hour = gps_location.hours();
    timeinfo.tm_mday = gps_location.day();
    timeinfo.tm_mon  = gps_location.month() - 1;
    timeinfo.tm_year = gps_location.year() - 1900;
    time_t time = mktime(&timeinfo);

    // make sure to sync onliy valid time fix
    if (gps_location.fixType() >= GNSSLocation::TYPE_TIME)
    {
        rtc_time_sync(time, true);
    }

    float latitude, longitude, hdop, epe, satellites, altitude = 0;
    latitude = gps_location.latitude();
    longitude = gps_location.longitude();
    altitude = gps_location.altitude();
    hdop = gps_location.hdop();
    epe = gps_location.ehpe();
    satellites = gps_location.satellites();

    // 3 of 4 bytes of the variable are populated with data
    uint32_t lat_packed = (uint32_t) (((latitude + 90) / 180.0) * 16777215);
    uint32_t lon_packed = (uint32_t) (((longitude + 180) / 360.0) * 16777215);

    readings_packet.data.lat1 = lat_packed >> 16;
    readings_packet.data.lat2 = lat_packed >> 8;
    readings_packet.data.lat3 = lat_packed;
    readings_packet.data.lon1 = lon_packed >> 16;
    readings_packet.data.lon2 = lon_packed >> 8;
    readings_packet.data.lon3 = lon_packed;

    readings_packet.data.alt = (uint16_t) altitude;
    readings_packet.data.satellites_hdop = (((uint8_t) satellites) << 4) | (((uint8_t) hdop) & 0x0f);
    readings_packet.data.time_to_fix = (uint8_t) (gps_time_to_fix / 1000);
    readings_packet.data.epe = (uint8_t) epe;
    readings_packet.data.snr = (uint8_t) max_snr;

#ifdef serial_debug
    serial_debug.print("gps(");
    serial_debug.print(" "); serial_debug.print(latitude, 7);
    serial_debug.print(" "); serial_debug.print(longitude, 7);
    serial_debug.print(" "); serial_debug.print(altitude, 3);
    serial_debug.print(" h: "); serial_debug.print(hdop, 2);     
    serial_debug.print(" e: "); serial_debug.print(epe, 2);
    serial_debug.print(" s: "); serial_debug.print(satellites, 0); 
    serial_debug.print(" t: "); serial_debug.print(gps_time_to_fix); 
    serial_debug.print(" f: "); serial_debug.print(gps_fail_fix_count); 
    serial_debug.print(" d: "); serial_debug.print(gps_location.fixType());
    serial_debug.print(" h: "); serial_debug.print(gps_hot_fix);
    serial_debug.print(" c: "); serial_debug.print(gps_fail_count);
    serial_debug.println(")");
    serial_debug.flush();
#endif
    flags = M_SEND; // send flag
}

void MODULE_GPS_UBLOX::gps_end(void)
{
    // Note: https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/issues/90
    gps_begin_happened = false;
    gps_power(false);
    gps_backup(false);

    // GPS periodic and triggered error and fix
    bitSet(readings_packet.data.status, 0);
    bitSet(readings_packet.data.status, 1);
    bitSet(readings_packet.data.status, 2);

    flags = M_ERROR; // flag the data to be redy for sending
    // Self-disable
#ifdef serial_debug
    serial_debug.print("gps: end(");
    serial_debug.println(")");
#endif
}

specific_public_data_t  MODULE_GPS_UBLOX::getter()
{

}