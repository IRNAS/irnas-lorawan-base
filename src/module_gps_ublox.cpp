#include "module_gps_ublox.h"

#define serial_debug Serial
uint8_t MODULE_GPS_UBLOX::set_settings(uint16_t *data, uint16_t length){

}

uint8_t MODULE_GPS_UBLOX::set_downlink_data(uint16_t *data, uint16_t length){

}

module_flags_e MODULE_GPS_UBLOX::scheduler(void){
  unsigned long interval=0;

  // do not schedule a GPS event if it has failed more then the specified amount of times
  if(gps_fail_count>settings_packet.data.gps_fail_retry){
    return flags;
  }

  // if triggered gps is enabled and accelerometer trigger has ocurred
  if(settings_packet.data.periodic_interval>0){
    interval=settings_packet.data.periodic_interval;
  }

  // if triggered gps is enabled and accelerometer trigger has ocurred - overrides periodic interval
  if(settings_packet.data.triggered_interval>0){
    if(((millis()-gps_accelerometer_last)/1000)<settings_packet.data.triggered_interval){
      interval=settings_packet.data.triggered_interval;
    }
  }

  // linear backoff upon fail
  if(bitRead(settings_packet.data.gps_settings,1)){
    interval=interval*(gps_fail_count+1);
  }

  if((interval!=0) & (millis()-gps_event_last>=interval*60*1000|gps_event_last==0)){
    if (flags==M_IDLE){
        read_timestamp=millis();
        flags=M_READ;
    }
    #ifdef serial_debug
      serial_debug.print("scheduler(");
      serial_debug.println("_read_values)");
    #endif
  }

  unsigned long elapsed = millis()-send_timestamp;
  if((settings_packet.data.send_interval!=0) & (elapsed>=(settings_packet.data.send_interval*60*1000))){
    if (flags==M_IDLE){
        send_timestamp=millis();
        flags=M_SEND;
    }
    #ifdef serial_debug
      serial_debug.print("scheduler(");
      serial_debug.println("send)");
    #endif
  }
  return flags;
}

uint8_t MODULE_GPS_UBLOX::initialize(void){
    settings_packet.data.periodic_interval=10;
    settings_packet.data.triggered_interval=0;
    settings_packet.data.send_interval=0;
    flags=M_IDLE;
}

uint8_t MODULE_GPS_UBLOX::send(uint8_t *data, size_t *size){
    //form the readings_packet

    #ifdef serial_debug
        serial_debug.print(name);
        serial_debug.print(": send(");
        serial_debug.println(")");
    #endif

    memcpy(data,&readings_packet.bytes[0], sizeof(module_readings_data_t));
    *size=sizeof(module_readings_data_t);
    flags=M_IDLE;
    return 1;
}

void MODULE_GPS_UBLOX::print_data(void){

}

uint8_t MODULE_GPS_UBLOX::read(void){
    #ifdef serial_debug
        serial_debug.print(name);
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
void MODULE_GPS_UBLOX::gps_accelerometer_interrupt(void){
  /*#ifdef serial_debug
    serial_debug.print("gps_accelerometer_interrupt(");
    serial_debug.println(")");
  #endif*/
  gps_accelerometer_last=millis();
}

/**
 * @brief Checks if GPS is busy and times out after the specified time
 * 
 * @param timeout 
 * @return boolean 
 */
boolean MODULE_GPS_UBLOX::gps_busy_timeout(uint16_t timeout){
  for(uint16_t i=0;i<timeout;i++){
    if(!GNSS.busy()){
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
void MODULE_GPS_UBLOX::gps_power(boolean enable){
  if(enable){
    pinMode(MODULE_GPS_EN,OUTPUT);
    digitalWrite(MODULE_GPS_EN,HIGH);
  }
  else{
    digitalWrite(MODULE_GPS_EN,LOW);
    delay(100);
    pinMode(MODULE_GPS_EN,INPUT_PULLDOWN);
  }
}

/**
 * @brief COntrols GPS pin for backup power
 * 
 * @param enable 
 */
void MODULE_GPS_UBLOX::gps_backup(boolean enable){
  if(enable){
    pinMode(MODULE_GPS_BCK,OUTPUT);
    digitalWrite(MODULE_GPS_BCK,HIGH);
  }
  else{
    digitalWrite(MODULE_GPS_BCK,LOW);
    pinMode(MODULE_GPS_BCK,INPUT_PULLDOWN);
  }
}

/**
 * @brief Initializes GPS - to be called upon boot or if no backup power is available
 * 
 * @return boolean - true if successful
 */
boolean MODULE_GPS_UBLOX::gps_begin(void){
  #ifdef serial_debug
    serial_debug.print("gps_begin(");
    serial_debug.println(")");
  #endif

  // check if more fails have occured then allowed
  if(gps_fail_count>settings_packet.data.gps_fail_retry){
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
  GNSS.begin(Serial1, GNSS.MODE_UBLOX, GNSS.RATE_1HZ);
  gps_begin_happened=true;
  if(gps_busy_timeout(3000)){
    gps_end();
    gps_fail_count++;
    #ifdef serial_debug
      serial_debug.println("fail after begin");
    #endif
    return false;
  }
  // Step 3: configure GPS
  // see default config https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/blob/18fb1cc81c6bc91b25e3346595f820985f2267e5/libraries/GNSS/src/utility/gnss_core.c#L2904
  boolean error=false;
  error|=gps_busy_timeout(1000);

  GNSS.setConstellation(GNSS.CONSTELLATION_GPS_AND_GLONASS);
  error|=gps_busy_timeout(1000);

  GNSS.disableWakeup();
  error|=gps_busy_timeout(1000);

  GNSS.setAutonomous(true);
  error|=gps_busy_timeout(1000);

  GNSS.setAntenna(GNSS.ANTENNA_INTERNAL);
  error|=gps_busy_timeout(1000);

  GNSS.setPlatform(GNSS.PLATFORM_PORTABLE);
  error|=gps_busy_timeout(1000);

  //GNSS.onLocation(gps_acquiring_callback);
  GNSS.onLocation(Callback(&MODULE_GPS_UBLOX::running, this));
  error|=gps_busy_timeout(1000);
  
  if(error){
    gps_end();
    gps_fail_count++;
    #ifdef serial_debug
      serial_debug.println("fail at config");
    #endif
    return false;
  }

  // GPS periodic and triggered error and fix
  bitClear(readings_packet.data.status,0);
  bitClear(readings_packet.data.status,1);
  bitSet(readings_packet.data.status,2);

  #ifdef serial_debug
    serial_debug.print("gps_begin(");
    serial_debug.println("done)");
  #endif

  return true;
}

/**
 * @brief Starts the GPS acquisition and sets up timeouts
 * 
 * @return boolean 
 */
boolean MODULE_GPS_UBLOX::gps_start(void){
  flags=M_ERROR;
  bitSet(readings_packet.data.status,2);
  // Step 0: Initialize GPS
  if(gps_begin_happened==false){
    if(gps_begin()==false){
      return false;
    }
  }
  //re-initialize upon fail
  else if(gps_fail_count>0){
    // This does not work currently due to a bug
    if(gps_begin()==false){
      return false;
    }
  }
  // Step 1: Enable power
  gps_power(true);
  delay(100);
  // Step 2: Resume GPS
  gps_busy_timeout(1000);
  if(GNSS.resume()==false){
    #ifdef serial_debug
      serial_debug.print("gps(resume failed ");
      serial_debug.println(")");
    #endif
    gps_fail_count++;
    return false;
  }

  // Step 3: Start acquiring location
  gps_fix_start_time=millis();
  if(gps_hot_fix){
    gps_timeout = settings_packet.data.gps_hot_fix_timeout*1000;
  }
  else{
    gps_timeout = settings_packet.data.gps_cold_fix_timeout*1000;
  }

  // Schedule automatic stop after timeout
  //gps_timer_timeout.start(gps_stop,gps_timeout+1000);
  // Schedule automatic stop after timeout if no messages received
  //gps_timer_response_timeout.start(gps_acquiring_callback,1000);
  gps_response_fail_count = 0;
  gps_done = false;
  flags=M_RUNNING;
  return true;
}

/**
 * @brief Callback triggered by the GPS module upon receiving amessage from it
 * 
 */
void MODULE_GPS_UBLOX::running(void){
  // Restart automatic stop after timeout if no messages received
  //gps_timer_response_timeout.start(gps_acquiring_callback,1000);
  if(millis()-gps_fix_start_time>gps_timeout){
    gps_stop();
  }
  else if(GNSS.location(gps_location)){
    float ehpe = gps_location.ehpe();
    boolean gps_3D_fix_required = bitRead(settings_packet.data.gps_settings,0);
    boolean gps_fully_resolved_required = bitRead(settings_packet.data.gps_settings,3);

    #ifdef serial_debug
      serial_debug.print("gps( ehpe ");
      serial_debug.print(ehpe);
      serial_debug.print(" sat ");
      serial_debug.print(gps_location.satellites());
      //serial_debug.print(" aopcfg ");
      //serial_debug.print(gps_location.aopCfgStatus());
      //serial_debug.print(" aop ");
      //serial_debug.print(gps_location.aopStatus());
      serial_debug.print(" d ");
      serial_debug.print(gps_location.fixType());
      serial_debug.print(" res ");
      serial_debug.print(gps_location.fullyResolved());
      serial_debug.println(" )");
    #endif

    gps_time_to_fix = (millis()-gps_fix_start_time); 
    if((gps_location.fixType()>= GNSSLocation::TYPE_2D)&(gps_time_to_fix>=(settings_packet.data.gps_min_fix_time*1000))){
      // clear GPS fix error
      bitClear(readings_packet.data.status,2);
      // wait until fix conditions are met
      if(
        (ehpe <= settings_packet.data.gps_min_ehpe)&
        (gps_location.fullyResolved()|gps_fully_resolved_required==false)&
        ((gps_location.fixType() == GNSSLocation::TYPE_3D)|gps_3D_fix_required==false)&
        (gps_time_to_fix>settings_packet.data.gps_min_fix_time)){
         
        /*#ifdef serial_debug
          serial_debug.print("gps(fix");
          serial_debug.print(" ttf ");
          serial_debug.print(gps_time_to_fix);
          serial_debug.print(" min ");
          serial_debug.print(settings_packet.data.gps_min_fix_time);
          serial_debug.println(")");
        #endif */
        if(bitRead(settings_packet.data.gps_settings,2)){
          // enable hot-fix immediately
          gps_hot_fix=true;
        }
        gps_stop();
        flags=M_SEND; // flag the data to be redy for sending
        //gps_timer_timeout.start(gps_stop,1);
      }
    }
  }
  else{
    gps_response_fail_count++;
    if(gps_response_fail_count>10){
      //raise error and stop
      gps_fail_count++;
      gps_stop();
      //gps_timer_timeout.start(gps_stop,1);
    }
  }
}

/**
 * @brief Reads the data from GPS and performs shutdown, called by either the timeout function or the acquiring callback upon fix
 * 
 * @param good_fix - to indicate stopping with good fix acquired
 */
void MODULE_GPS_UBLOX::gps_stop(void){
  //gps_timer_timeout.stop();
  //gps_timer_response_timeout.stop();
  gps_time_to_fix = (millis()-gps_fix_start_time);
  // Power off GPS
  gps_power(false);
  if(!bitRead(settings_packet.data.gps_settings,2)){
    // Disable GPS if hot-fix mechanism is not used
    gps_hot_fix=false;
  }

  // if GPS fix error
  if(bitRead(readings_packet.data.status,2)){
    gps_fail_fix_count++;
    if((gps_hot_fix==true)&(gps_fail_fix_count>=settings_packet.data.gps_hot_fix_retry)){
      //disable hot-fix and rest fail counter
      gps_hot_fix=false;
      gps_fail_fix_count=0;
    }
    else if(gps_fail_fix_count>=settings_packet.data.gps_cold_fix_retry){
      gps_fail_count++;
    }
  }
  else{
    gps_fail_fix_count=0;
    gps_fail_count=0;
  }
  gps_done = true;

  if(gps_fail_count>0){
    gps_hot_fix=false;
    gps_end();
    return; // continuing fails otherwise by reading invalid data locaitons
  }

  GNSS.satellites(gps_satellites);
  uint8_t max_snr = 0;
  for (unsigned int index = 0; index < gps_satellites.count(); index++){
    if(max_snr<gps_satellites.snr(index)){
      max_snr=gps_satellites.snr(index);
    }
  }

  float latitude, longitude, hdop, epe, satellites, altitude = 0;
  latitude = gps_location.latitude();
  longitude = gps_location.longitude();
  altitude = gps_location.altitude();
  hdop = gps_location.hdop();
  epe = gps_location.ehpe();
  satellites = gps_location.satellites();

  // 3 of 4 bytes of the variable are populated with data
  uint32_t lat_packed = (uint32_t)(((latitude + 90) / 180.0) * 16777215);
  uint32_t lon_packed = (uint32_t)(((longitude + 180) / 360.0) * 16777215);
  readings_packet.data.lat1 = lat_packed >> 16;
  readings_packet.data.lat2 = lat_packed >> 8;
  readings_packet.data.lat3 = lat_packed;
  readings_packet.data.lon1 = lon_packed >> 16;
  readings_packet.data.lon2 = lon_packed >> 8;
  readings_packet.data.lon3 = lon_packed;
  readings_packet.data.alt = (uint16_t)altitude;
  readings_packet.data.satellites_hdop = (((uint8_t)satellites)<<4)|(((uint8_t)hdop)&0x0f);
  readings_packet.data.time_to_fix = (uint8_t)(gps_time_to_fix/1000);
  readings_packet.data.epe = (uint8_t)epe;
  readings_packet.data.snr = (uint8_t)max_snr;
    
  #ifdef serial_debug
    serial_debug.print("gps(");
    serial_debug.print(" "); serial_debug.print(latitude,7);
    serial_debug.print(" "); serial_debug.print(longitude,7);
    serial_debug.print(" "); serial_debug.print(altitude,3);
    serial_debug.print(" h: "); serial_debug.print(hdop,2);     
    serial_debug.print(" e: "); serial_debug.print(epe,2);
    serial_debug.print(" s: "); serial_debug.print(satellites,0); 
    serial_debug.print(" t: "); serial_debug.print(gps_time_to_fix); 
    serial_debug.print(" f: "); serial_debug.print(gps_fail_fix_count); 
    serial_debug.print(" d: "); serial_debug.print(gps_location.fixType());
    serial_debug.print(" h: "); serial_debug.print(gps_hot_fix);
    serial_debug.print(" c: "); serial_debug.print(gps_fail_count);
    serial_debug.println(")");
    serial_debug.flush();
  #endif
}

void MODULE_GPS_UBLOX::gps_end(void){
  // Note: https://github.com/GrumpyOldPizza/ArduinoCore-stm32l0/issues/90
  //GNSS.end();
  gps_begin_happened==false;
  gps_power(false);
  gps_backup(false);
  // GPS periodic and triggered error and fix
  bitSet(readings_packet.data.status,0);
  bitSet(readings_packet.data.status,1);
  bitSet(readings_packet.data.status,2);
  flags=M_ERROR; // flag the data to be redy for sending
  // Self-disable
  #ifdef serial_debug
    serial_debug.print("gps_end(");
    serial_debug.println(")");
  #endif
}