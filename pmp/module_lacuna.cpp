#ifndef REGION
#define REGION R_EU868
#endif

#include "module_lacuna.h"
#include "debug.h"
#include "secrets.h"

#ifdef MODULE_LACUNA_DEBUG
#define NAME  "lacuna" 
#define serial_debug  Serial
#endif

extern event_e system_event;

static char payload[255];

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::configure(uint8_t * data, size_t * size)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": configure(");
    serial_debug.println(")");
#endif

    // Do not accept settings if size does not match
    if (*size != sizeof(module_settings_data_t))
    {
        return 0;
    }

    // copy to buffer
    module_settings_packet_t settings_packet_downlink;
    memcpy(&settings_packet_downlink.bytes[0], data, sizeof(module_settings_data_t));

    // validate settings value range 
    settings_packet.data.global_id = settings_packet_downlink.data.global_id;
    settings_packet.data.start_hour = settings_packet_downlink.data.start_hour;
    settings_packet.data.start_min = settings_packet_downlink.data.start_min;
    settings_packet.data.end_hour = settings_packet_downlink.data.end_hour;
    settings_packet.data.end_min = settings_packet_downlink.data.end_min;  
    settings_packet.data.min_rssi = settings_packet_downlink.data.min_rssi;  
    return 0;
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::get_settings_length()
{
    return sizeof(module_settings_data_t);
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::set_downlink_data(uint8_t * data, size_t * size)
{

}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
module_flags_e MODULE_LACUNA::scheduler(void) {
#ifdef serial_debug
  serial_debug.print(NAME);
  serial_debug.print(": scheduler(");
  serial_debug.println("active)");
#endif
  flags = M_RUNNING;
  return flags;
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::initialize(void)
{
    //Our timewindow in which we want Lacuna to operate
    settings_packet.data.start_hour = 0;
    settings_packet.data.start_min = 0;
    settings_packet.data.end_hour = 23;
    settings_packet.data.end_min = 59;
    settings_packet.data.min_rssi = 98;

    flags = M_IDLE; // Needed for normal running  of modules
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::send(uint8_t * buffer, size_t * size)
{
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::read(void)
{

}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
void MODULE_LACUNA::running(void)
{
    // Turn on power for lacuna modem
    pinMode(MODULE_LACUNA_5V, OUTPUT);
    digitalWrite(MODULE_LACUNA_5V, HIGH);
    
    setup_lacuna();
    receive_lacuna(); 

    // Turn off power for lacuna modem
    digitalWrite(MODULE_LACUNA_5V, LOW);
    flags = M_IDLE; 
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
void MODULE_LACUNA::event(event_e event)
{

}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
void MODULE_LACUNA::print_data(void)
{

}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
void MODULE_LACUNA::setup_lacuna(void)
{    
    // SX1262 configuration for lacuna LS200 board
    lsSX126xConfig cfg;
    lsCreateDefaultSX126xConfig(&cfg);

    cfg.nssPin = PB12;
    cfg.resetPin = -1;           // Not needed
    cfg.antennaSwitchPin = PB2;
    cfg.busyPin = PH1;             
    cfg.dio1Pin = PB7;             
    cfg.osc = lsSX126xOscTCXO;      // for Xtal: lsSX126xOscXtal
    cfg.type = lsSX126xTypeSX1262;  // for SX1261: lsSX126xTypeSX1261

    // Initialize SX1262
    int result = lsInitSX126x(&cfg);

    if (!result)
    {
#ifdef serial_debug
        serial_debug.print("E22/SX1262: ");
        serial_debug.println(lsErrorToString(result));
#endif
    }

    uint8_t relay_networkKey[] = RELAY_NETWORKKEY;
    uint8_t relay_appKey[] = RELAY_APPKEY;
    uint8_t relay_deviceAddress[] = RELAY_DEVICEADDRESS;

    // LoRaWAN parameters for relay
    lsCreateDefaultLoraWANParams(&relay_loraWANParams, 
                                 relay_networkKey, 
                                 relay_appKey, 
                                 relay_deviceAddress);

    // transmission parameters for Lacuna satellites
    lsCreateDefaultLoraSatTxParams(&SattxParams);

    // Override default Lacuna satellite parameters
    SattxParams.frequency = 862750000;
    SattxParams.power = 21;
    SattxParams.grid = lsLoraEGrid_3_9_khz;
    SattxParams.codingRate = lsLoraECodingRate_1_3;
    SattxParams.bandwidth = lsLoraEBandwidth_335_9_khz;;
    SattxParams.nbSync = 4;
    SattxParams.hopping = 1;

    // transmission parameters for terrestrial LoRa
    lsCreateDefaultLoraTxParams(&txParams);
    
    // Override defult LoRa parameters
    txParams.spreadingFactor = lsLoraSpreadingFactor_11;
    txParams.power = 21;
    txParams.codingRate = lsLoraCodingRate_4_5;
    txParams.invertIq = false;
    txParams.frequency = 867300000;
    txParams.bandwidth = lsLoraBandwidth_125_khz;
    txParams.syncWord = LS_LORA_SYNCWORD_PUBLIC;
    txParams.preambleLength = 8;

    // LoRa parameters for relay (receive) 
    lsCreateDefaultLoraTxParams(&relayParams);

    // Make sure that SF and bandwith match with device that we will relay.
    relayParams.spreadingFactor = lsLoraSpreadingFactor_9;
    relayParams.invertIq = false;
    relayParams.frequency = 868500000;
    relayParams.bandwidth = lsLoraBandwidth_125_khz;
    relayParams.syncWord = LS_LORA_SYNCWORD_PUBLIC;
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
void MODULE_LACUNA::receive_lacuna(void)
{
    int32_t rxlength = lsReceiveLora(&relay_loraWANParams, 
                                     &relayParams, 
                                     global_relay_payload);

    if (rxlength) 
    { 
        /* valid relay data received */
#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(": message(");
        serial_debug.println("Valid uplink received!)");
        serial_debug.print("        SNR: ");
        serial_debug.println(relayParams.snr);
        serial_debug.print("        RSSI: ");
        serial_debug.println(relayParams.rssi);
        serial_debug.print("        SignalRSSI: ");
        serial_debug.println(relayParams.signalrssi);
        serial_debug.print("        Payload in hex: ");

        for (uint8_t n = 0; n < rxlength; n++)
        {
            serial_debug.print (global_relay_payload[n], HEX);
            serial_debug.write (" ");
        }

        uint8_t * device_id = getDeviceId();

        serial_debug.print("\n        Device ID in hex: ");
        for (uint8_t n = 0; n < 4; n++)
        {
            serial_debug.print (device_id[n], HEX);
            serial_debug.write (" ");
        }
        serial_debug.println();
#endif

        if (settings_packet.data.min_rssi == 0 || relayParams.rssi > -settings_packet.data.min_rssi) {
            global_activate_pira += 1;
            global_pira_wakeup_reason = 1;
        }
    }

}

specific_public_data_t MODULE_LACUNA::getter()
{
}
/*** end of file ***/
