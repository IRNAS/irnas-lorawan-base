#include "lorawan.h"

#define serial_debug  Serial

//#define LORAWAN_ABP
#define LORAWAN_OTAA

#ifdef LORAWAN_ABP
// LoraWAN ABP configuration
const char *devAddr = "260118C5";
const char *nwkSKey = "675F9B8777AB66EF3B967B3FC4FC7527";
const char *appSKey = "74A22F5287DAA0A6AA9639B32A721190";
#endif

#ifdef LORAWAN_OTAA
// LoraWAN ABP configuration
const char *appEui  = "70B3D57ED0022A92";
const char *appKey  = "6EAFE1A9D64C6FA2F627C09451A0CDA5";
const char *devEui  = "004E970288420117";
#endif

boolean lorawan_send_successful = false; // flags sending has been successful to the FSM

boolean lorawan_settings_new = false;
uint8_t lorawan_settings_buffer[256];
size_t lorawan_settings_length=0;

/**
 * @brief Initialize LoraWAN communication, returns fales if fails
 * 
 * @return boolean 
 */
boolean lorawan_init(void){
  if(LoRaWAN.begin(EU868)==0){
    return false;
  }
  LoRaWAN.addChannel(1, 868100000, 0, 5);
  LoRaWAN.addChannel(2, 868300000, 0, 5);
  LoRaWAN.addChannel(3, 868500000, 0, 5);
  LoRaWAN.addChannel(4, 867100000, 0, 5);
  LoRaWAN.addChannel(5, 867300000, 0, 5);
  LoRaWAN.addChannel(6, 867500000, 0, 5);
  LoRaWAN.addChannel(7, 867900000, 0, 5);
  LoRaWAN.addChannel(8, 867900000, 0, 5);
  LoRaWAN.setDutyCycle(false);
  // LoRaWAN.setAntennaGain(2.0);
  LoRaWAN.setTxPower(20);
  
  LoRaWAN.onJoin(lorawan_joinCallback);
  LoRaWAN.onLinkCheck(lorawan_checkCallback);
  LoRaWAN.onTransmit(lorawan_doneCallback);
  LoRaWAN.onReceive(lorawan_receiveCallback);

  #ifdef LORAWAN_OTAA
  //Get the device ID
  //LoRaWAN.getDevEui(devEui, 18);
  LoRaWAN.setSaveSession(true); // this will save the session for reboot, useful if reboot happens with in poor signal conditons
  //LoRaWAN.setLinkCheckLimit(48); // number of uplinks link check is sent, 5 for experimenting, 48 otherwise
  //LoRaWAN.setLinkCheckDelay(4); // number of uplinks waiting for an answer, 2 for experimenting, 4 otherwise
  //LoRaWAN.setLinkCheckThreshold(4); // number of times link check fails to assert link failed, 1 for experimenting, 4 otherwise
  LoRaWAN.joinOTAA(appEui, appKey, devEui);
  #endif

  #ifdef LORAWAN_ABP
  LoRaWAN.joinABP(devAddr, nwkSKey, appSKey);
  #endif

  //moving to main loop
  //lorawan_joinCallback(); // call join callback manually to execute all the steps, necessary for ABP or OTAA with saved session
  return true;
}

/**
 * @brief Sends the provided data buffer on the given port, returns false if failed
 * 
 * @param port 
 * @param buffer 
 * @param size 
 * @return boolean 
 */
boolean lorawan_send(uint8_t port, const uint8_t *buffer, size_t size){
  #ifdef serial_debug
    serial_debug.println("lorawan_send() init");
  #endif
  int response = 0; 
  if (!LoRaWAN.joined()) {
    #ifdef serial_debug
      serial_debug.println("lorawan_send() not joined");
    #endif
    return false;
  }

  if (LoRaWAN.busy()) {
    #ifdef serial_debug
      serial_debug.println("lorawan_send() busy");
    #endif
    return false;
  }

  LoRaWAN.setDutyCycle(settings_packet.data.lorawan_reg);
  LoRaWAN.setTxPower(settings_packet.data.lorawan_txp);
  LoRaWAN.setADR(settings_packet.data.lorawan_adr);
  LoRaWAN.setDataRate(settings_packet.data.lorawan_datarate);
  #ifdef serial_debug
    serial_debug.print("lorawan_send( ");
    serial_debug.print("TimeOnAir: ");
    serial_debug.print(LoRaWAN.getTimeOnAir());
    serial_debug.print(", NextTxTime: ");
    serial_debug.print(LoRaWAN.getNextTxTime());
    serial_debug.print(", MaxPayloadSize: ");
    serial_debug.print(LoRaWAN.getMaxPayloadSize());
    serial_debug.print(", DR: ");
    serial_debug.print(LoRaWAN.getDataRate());
    serial_debug.print(", TxPower: ");
    serial_debug.print(LoRaWAN.getTxPower(), 1);
    serial_debug.print("dbm, UpLinkCounter: ");
    serial_debug.print(LoRaWAN.getUpLinkCounter());
    serial_debug.print(", DownLinkCounter: ");
    serial_debug.print(LoRaWAN.getDownLinkCounter());
    serial_debug.print(", Port: ");
    serial_debug.print(port);
    serial_debug.print(", Size: ");
    serial_debug.print(size);
    serial_debug.println(" )");
  #endif
  // int sendPacket(uint8_t port, const uint8_t *buffer, size_t size, bool confirmed = false);
  response = LoRaWAN.sendPacket(port, buffer, size, false);
  if(response>0){
    #ifdef serial_debug
      serial_debug.println("lorawan_send() sendPacket");
    #endif
    lorawan_send_successful = false;
    return true;
  }
  else{
    #ifdef serial_debug
      serial_debug.print("lorawan_send failed (");
      serial_debug.print(response);
      serial_debug.println(")");
    #endif
  }
  return false;
}

/**
 * @brief Callback ocurring when join has been successful
 * 
 */
void lorawan_joinCallback(void)
{
    if (LoRaWAN.joined())
    {
      #ifdef serial_debug
        serial_debug.println("JOINED");
      #endif
      LoRaWAN.setRX2Channel(869525000, 3); // SF12 - 0 for join, then SF 9 - 3, see https://github.com/TheThingsNetwork/ttn/issues/155
    }
    else
    {
      #ifdef serial_debug
        serial_debug.println("REJOIN( )");
      #endif
      LoRaWAN.rejoinOTAA();
    }
}

/**
 * @brief returns the boolean status of the LoraWAN join
 * 
 * @return boolean 
 */
boolean lorawan_joined(void)
{
  return LoRaWAN.joined();
}

/**
 * @brief Callback occurs when link check packet has been received
 * 
 */
void lorawan_checkCallback(void)
{
  #ifdef serial_debug
    serial_debug.print("CHECK( ");
    serial_debug.print("RSSI: ");
    serial_debug.print(LoRaWAN.lastRSSI());
    serial_debug.print(", SNR: ");
    serial_debug.print(LoRaWAN.lastSNR());
    serial_debug.print(", Margin: ");
    serial_debug.print(LoRaWAN.linkMargin());
    serial_debug.print(", Gateways: ");
    serial_debug.print(LoRaWAN.linkGateways());
    serial_debug.println(" )");
  #endif
}

/**
 * @brief Callback when the data is received via LoraWAN - procecssing various content
 * 
 */
void lorawan_receiveCallback(void)
{
  #ifdef serial_debug
    serial_debug.print("RECEIVE( ");
    serial_debug.print("RSSI: ");
    serial_debug.print(LoRaWAN.lastRSSI());
    serial_debug.print(", SNR: ");
    serial_debug.print(LoRaWAN.lastSNR());
  #endif
  if (LoRaWAN.parsePacket())
  {
    uint32_t size;
    uint8_t data[256];

    size = LoRaWAN.read(&data[0], sizeof(data));

    if (size)
    {
      data[size] = '\0';
      #ifdef serial_debug
          serial_debug.print(", PORT: ");
          serial_debug.print(LoRaWAN.remotePort());
          serial_debug.print(", SIZE: \"");
          serial_debug.print(size);
          serial_debug.print("\"");
          serial_debug.println(" )");
      #endif

      //handle settings
      if(LoRaWAN.remotePort()==settings_get_packet_port()){
        memcpy(&lorawan_settings_buffer[0],&data[0], size);
        lorawan_settings_length=size;
        lorawan_settings_new=true;
      }
    }
  }
}

/**
 * @brief Callback on transmission done - signals successful TX with a flag
 * 
 */
void lorawan_doneCallback(void)
{
  #ifdef serial_debug
    serial_debug.println("DONE()");
  #endif

  if (!LoRaWAN.linkGateways())
  {
    #ifdef serial_debug
      serial_debug.println("DISCONNECTED");
    #endif
  }
  else{
    lorawan_send_successful = true;
  }
}