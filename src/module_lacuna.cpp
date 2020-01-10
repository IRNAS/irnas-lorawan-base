#include "module_lacuna.h"

#define serial_debug Serial

#define NAME  "accelerometer"
extern event_e system_event;

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::configure(uint8_t * data, size_t * size)
{

}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::get_settings_length()
{

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
    serial_debug.println("Inside Lacuna scheduler:");
    serial_debug.println(ctime(&current_time));

    struct tm *timeinfo = localtime(&current_time);

    timeinfo->tm_hour = start_tx.hour;
    timeinfo->tm_min = start_tx.min;
    time_t start_window_tx = mktime(timeinfo);

    timeinfo->tm_hour = end_tx.hour;
    timeinfo->tm_min = end_tx.min;
    time_t end_window_tx = mktime(timeinfo);

    serial_debug.print("Window start: ");
    serial_debug.println(ctime(&start_window_tx));

    serial_debug.print("Window end: ");
    serial_debug.println(ctime(&end_window_tx));


    if(current_time >= start_window_tx && current_time < end_window_tx)
    {
        serial_debug.println("We are inside correct time window lets send!!!");
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
    flags = M_IDLE; // Needed for normal running  of modules

    //Our timewindow in which we want Lacuna to operate
    start_tx = { 19, 28 };
    end_tx = { 21, 31 };
    
    // lora_init_done should be false at the start to ensure that setup_lacuna
    // is called first time 
    lacuna_init_done = false;
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::send(uint8_t * buffer, size_t * size)
{
    serial_debug.println("WE GET INTO MODULE_LACUNA_SEND");
    if(!lacuna_init_done)
    {
        setup_lacuna();
        lacuna_init_done = true;
    }
    send_lacuna(); 
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
uint8_t MODULE_LACUNA::running(void)
{

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
    pinMode(PB6, OUTPUT);
    digitalWrite(PB6, HIGH);

    byte networkKey[] = {
        0xA4, 0xB5, 0x39, 0x6B, 0x95, 0xAE, 0xF1, 0xF5,
        0x7C, 0x43, 0x62, 0x46, 0x74, 0x39, 0x8D, 0x6D};

    byte appKey[] = {
        0x5D, 0x66, 0x6C, 0xBA, 0xEE, 0xF0, 0xAB, 0x29,
        0x6E, 0x59, 0x8A, 0xC3, 0xFE, 0x46, 0x7B, 0x4F};

    //TODO: Replace with your device address
    byte deviceAddress[] = {0x26, 0x01, 0x12, 0x16};

    // SX1262 configuration for lacuna LS200 board
    lsSX126xConfig cfg;

    cfg.nssPin = PB12;
    //cfg.resetPin = PA3; //Not needed
    cfg.antennaSwitchPin = PH1;
    cfg.busyPin = PB2;
    cfg.osc = lsSX126xOscTCXO;     // for Xtal: lsSX126xOscXtal
    cfg.type = lsSX126xTypeSX1262; // for SX1261: lsSX126xTypeSX1261

    // Initialize SX1262
    int result = lsInitSX126x(&cfg);
    serial_debug.print("E22/SX1262: ");
    serial_debug.println(lsErrorToString(result));

    // LoRaWAN session parameters
    lsCreateDefaultLoraWANParams(&loraWANParams, networkKey, appKey, deviceAddress);
    loraWANParams.txPort = 1;
    loraWANParams.rxEnable = true;

    // transmission parameters for Lacuna satellites
    lsCreateDefaultLoraSatTxParams(&SattxParams);

    // Override default Lacuna satellite parameters
    SattxParams.frequency = 863000000;
    SattxParams.grid = lsLoraEGrid_3_9_khz;
    SattxParams.codingRate = lsLoraECodingRate_1_3;
    SattxParams.bandwidth = lsLoraEBandwidth_187_5_khz;
    SattxParams.nbSync = 4;
    SattxParams.power = 14;
    SattxParams.hopping = 1;

    // transmission parameters for terrestrial LoRa
    lsCreateDefaultLoraTxParams(&txParams);
    
    // Override defult LoRa parameters
    txParams.power = 14;
    txParams.spreadingFactor = lsLoraSpreadingFactor_11;
    txParams.codingRate = lsLoraCodingRate_4_5;
    txParams.invertIq = false;
    txParams.frequency = 868300000;
    txParams.bandwidth = lsLoraBandwidth_125_khz;
    txParams.syncWord = LS_LORA_SYNCWORD_PUBLIC;
    txParams.preambleLength = 8;

    serial_debug.print("Terrestrial Uplink Frequency: ");
    serial_debug.println(txParams.frequency / 1e6);
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
void MODULE_LACUNA::send_lacuna(void)
{
    static char payload[255];

    // Sent LoRa message
    Serial.println("Sending LoRa message");

    const String lora_text = "Hello Lora";
    lora_text.toCharArray(payload, 255);
    int lora_result = lsSendLoraWAN(&loraWANParams, &txParams, (byte *)payload, sizeof lora_text);

    Serial.print("LoraWan result: ");
    Serial.println(lsErrorToString(lora_result));

    delay(1000);

    // Sent LoRaSat message
    Serial.println("Sending LoraSat message");
    const String satellite_text = "Hello Lacuna";
    satellite_text .toCharArray(payload, 255);
    int sat_result = lsSendLoraSatWAN(&loraWANParams, &SattxParams, (byte *)payload, sizeof satellite_text);

    Serial.print("LoraSatWan result: ");
    Serial.println(lsErrorToString(sat_result));

    delay(1000);
}