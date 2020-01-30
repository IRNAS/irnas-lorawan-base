#ifndef REGION
#define REGION R_EU868
#endif

#include "module_lacuna.h"

#define serial_debug Serial

#define NAME  "lacuna" 
 extern event_e system_event;

static char payload[255];
const String mytext = "Hello Lacuna!";

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::configure(uint8_t * data, size_t * size)
{
    // copy to buffer
    module_settings_packet_t settings_packet_downlink;
    memcpy(&settings_packet_downlink.bytes[0], data, sizeof(module_settings_data_t));

    // validate settings value range 
    settings_packet.data.global_id = settings_packet_downlink.data.global_id;

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
    serial_debug.println("Inside Lacuna scheduler:");
    serial_debug.println(ctime(&current_time));
#endif

    struct tm *timeinfo = localtime(&current_time);

    timeinfo->tm_hour = start_tx.hour;
    timeinfo->tm_min = start_tx.min;
    timeinfo->tm_sec =  0;
    time_t start_window_tx = mktime(timeinfo);

    timeinfo->tm_hour = end_tx.hour;
    timeinfo->tm_min = end_tx.min;
    timeinfo->tm_sec =  0;
    time_t end_window_tx = mktime(timeinfo);

    if(current_time >= start_window_tx && current_time < end_window_tx)
    {

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(":scheduler(");
    serial_debug.println("send_values)");
#endif
        flags = M_RUNNING;
        return flags;
    }
    flags = M_IDLE;
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
    start_tx = { 8, 10 };
    end_tx = { 13, 10 };
    
    // lora_init_done should be false at the start to ensure that setup_lacuna
    // is called first time 
    lacuna_init_done = false;

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
    setup_lacuna();
    send_lacuna(); 

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
    // Turn on power
    pinMode(PH0, OUTPUT);
    digitalWrite(PH0, HIGH);
    pinMode(MODULE_5V_EN, OUTPUT);
    digitalWrite(MODULE_5V_EN, HIGH);

    // Keys and device address are MSB
    byte networkKey[] = {
        0xA4, 0xB5, 0x39, 0x6B, 0x95, 0xAE, 0xF1, 0xF5,
        0x7C, 0x43, 0x62, 0x46, 0x74, 0x39, 0x8D, 0x6D};

    byte appKeyLacuna[] = {
        0x5D, 0x66, 0x6C, 0xBA, 0xEE, 0xF0, 0xAB, 0x29,
        0x6E, 0x59, 0x8A, 0xC3, 0xFE, 0x46, 0x7B, 0x4F};

    //TODO: Replace with your device address
    byte deviceAddress[] = {0x26, 0x01, 0x12, 0x16};


#ifdef serial_debug
    serial_debug.println("Initializing");

    serial_debug.print("Configured Region: ");
#if REGION == R_EU868
    serial_debug.println("Europe 862-870 Mhz");
#elif REGION == R_US915
    serial_debug.println("US 902-928 Mhz");
#elif REGION == R_AS923_changed
    serial_debug.println("Asia 923 Mhz");
#elif REGION == R_IN865
    serial_debug.println("India 865-867 Mhz");
#else
    serial_debug.println("Undefined");
#endif
#endif

    // SX1262 configuration for lacuna LS200 board
    lsSX126xConfig cfg;
    lsCreateDefaultSX126xConfig(&cfg);

    cfg.nssPin = PB12;
    cfg.resetPin = PA3; //Not needed
    cfg.antennaSwitchPin = PH1;
    cfg.busyPin = PB2;             // pin 2 for Lacuna shield, pin 3 for Semtch SX126x eval boards
    cfg.osc = lsSX126xOscTCXO;     // for Xtal: lsSX126xOscXtal
    cfg.type = lsSX126xTypeSX1262; // for SX1261: lsSX126xTypeSX1261

    // Initialize SX1262
    int result = lsInitSX126x(&cfg);

#ifdef serial_debug
    serial_debug.print("E22/SX1262: ");
    serial_debug.println(lsErrorToString(result));
#endif

    // LoRaWAN session parameters
    lsCreateDefaultLoraWANParams(&loraWANParams, networkKey, appKeyLacuna, deviceAddress);
    loraWANParams.txPort = 1;
    loraWANParams.rxEnable = true;

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
    txParams.frequency = 868300000;
    txParams.power = 21;
    txParams.codingRate = lsLoraCodingRate_4_5;
    txParams.bandwidth = lsLoraBandwidth_125_khz;
    txParams.spreadingFactor = lsLoraSpreadingFactor_11;
    txParams.invertIq = false;
    txParams.syncWord = LS_LORA_SYNCWORD_PUBLIC;
    txParams.preambleLength = 8;

#ifdef serial_debug
    serial_debug.print("Terrestrial Uplink Frequency: ");
    serial_debug.println(txParams.frequency / 1e6);
#endif
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
void MODULE_LACUNA::send_lacuna(void)
{
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
}

specific_public_data_t MODULE_LACUNA::getter()
{

}