#ifndef REGION
#define REGION R_EU868
#endif

#include "module_lacuna.h"
#include "debug.h"

#ifdef MODULE_LACUNA_DEBUG
#define NAME  "lacuna" 
#define serial_debug  Serial
#endif

extern event_e system_event;

static char payload[255];
static byte relay_payload[255];

const String mytext = "Hello Lacuna!";

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
module_flags_e MODULE_LACUNA::scheduler(void)
{
    time_t current_time = rtc_time_read();

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": ");
    serial_debug.print(ctime(&current_time));
#endif

    struct tm *timeinfo = localtime(&current_time);

    timeinfo->tm_hour = settings_packet.data.start_hour;
    timeinfo->tm_min = settings_packet.data.start_min;
    timeinfo->tm_sec =  0;
    time_t start_window_tx = mktime(timeinfo);

    timeinfo->tm_hour = settings_packet.data.end_hour;
    timeinfo->tm_min = settings_packet.data.end_min;
    timeinfo->tm_sec =  0;
    time_t end_window_tx = mktime(timeinfo);

    if(current_time >= start_window_tx && current_time < end_window_tx)
    {
#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(": scheduler(");
        serial_debug.println("active)");
#endif
        flags = M_RUNNING;
        return flags;
    }
    else
    {
#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(": scheduler(");
        serial_debug.println("not active)");
#endif
        flags = M_IDLE;
        return flags;
    }
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
    // Turn on power
    // Not needed here as for hack the poacher we got power for lacuna modem 
    // directly by tapping 3v3 line
    
    //pinMode(MODULE_LACUNA_5V, OUTPUT);
    //digitalWrite(MODULE_LACUNA_5V, HIGH);
    //pinMode(MODULE_5V_EN, OUTPUT);
    //digitalWrite(MODULE_5V_EN, HIGH);
    
    setup_lacuna();
    send_lacuna(); 
    // Turn off only 5V to Lacuna, leave general 5v on, as Rpi might be working.
    //digitalWrite(MODULE_LACUNA_5V, LOW);
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
    // Keys where we will be transmiting
    uint8_t lacuna_networkKey[] = { 0xB0, 0xA6, 0xAF, 0x05, 0xE9, 0x38, 0xE6, 0xA2, 0x18, 0xBE, 0xC0, 0x2F, 0x41, 0x2B, 0x61, 0x6A };
    uint8_t lacuna_appKey[] = { 0xDB, 0x43, 0xA2, 0xD3, 0x8A, 0xAF, 0x67, 0xE7, 0x6F, 0x6C, 0x7F, 0xA9, 0xBF, 0x8F, 0x7F, 0x5E };
    uint8_t lacuna_deviceAddress[] =  { 0x26, 0x01, 0x19, 0x7C };

     Device to be relayed (receive)
    uint8_t relay_networkKey[] = { 0xB9, 0xDC, 0x31, 0x02, 0x26, 0x20, 0x2E, 0xD5, 0x4C, 0xB0, 0xFD, 0x2F, 0xFC, 0x71, 0xC5, 0xA9 };
    uint8_t relay_appKey[] = { 0xC9, 0xE3, 0xBB, 0x9E, 0x98, 0x99, 0x7F, 0xE0, 0x92, 0x6F, 0x96, 0x3C, 0x6B, 0x78, 0x4E, 0x03 };
    uint8_t relay_deviceAddress[] = { 0x26, 0x01, 0x1D, 0xDF };
    
    //uint8_t relay_networkKey[] = { 0x0A, 0x7E, 0x92, 0xD4, 0x4B, 0x36, 0x48, 0xE7, 0x62, 0xF2, 0x54, 0x57, 0xA0, 0x39, 0x7E, 0x8E };
    //uint8_t relay_appKey[] = { 0x9C, 0xD0, 0xF7, 0x34, 0x4C, 0x2A, 0x55, 0x18, 0x82, 0x20, 0x8D, 0x66, 0xE0, 0x78, 0xF6, 0xC2 };
    //uint8_t relay_deviceAddress[] = { 0x26, 0x01, 0x19, 0x20 };

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

    // LoRaWAN session parameters
    lsCreateDefaultLoraWANParams(&loraWANParams, 
                                 lacuna_networkKey, 
                                 lacuna_appKey, 
                                 lacuna_deviceAddress);
    loraWANParams.txPort = 1;
    loraWANParams.rxEnable = true;

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
void MODULE_LACUNA::send_lacuna(void)
{
    /*
    // Sent LoRa message
#ifdef serial_debug
    serial_debug.println("Sending LoRa message");
#endif

    mytext.toCharArray(payload, 255);
    int lora_result = lsSendLoraWAN(&loraWANParams, &txParams, (byte *)payload, sizeof mytext);

#ifdef serial_debug
    serial_debug.print("LoraWan result: ");
    serial_debug.println(lsErrorToString(lora_result));
#endif


    // Sent LoRaSat message
#ifdef serial_debug
    serial_debug.println("Sending LoraSat message");
#endif
    mytext.toCharArray(payload, 255);
    int sat_result = lsSendLoraSatWAN(&loraWANParams, &SattxParams, (byte *)payload, sizeof mytext);

#ifdef serial_debug
    serial_debug.print("LoraSatWan result: ");
    serial_debug.println(lsErrorToString(sat_result));
#endif


*/
    int32_t rxlength = lsReceiveLora(&relay_loraWANParams, 
                                     &relayParams, 
                                     relay_payload);

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
            serial_debug.print (relay_payload[n], HEX);
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
        int lora_result = lsSendLoraWAN(&loraWANParams, 
                &txParams, 
                (byte *)relay_payload, 
                rxlength);
#ifdef serial_debug
        if( LS_OK != lora_result)
        {
            serial_debug.print("Result: ");
            serial_debug.println(lsErrorToString(lora_result));
        }
#endif
        global_activate_pira += 1;
        global_pira_wakeup_reason = 1;
    }

}

specific_public_data_t MODULE_LACUNA::getter()
{
}
/*** end of file ***/
