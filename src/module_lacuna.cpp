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
        Serial.println(ctime(&current_time));
        Serial.println(current_time);
        return M_IDLE;
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
    /*
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
    */
}

/*!
 * @brief 
 * @param[in] 
 * @param[in]
 * @return 
 */
uint8_t MODULE_LACUNA::send(uint8_t * buffer, size_t * size)
{
    /*
    // Sent LoRa message
    Serial.println("Sending LoRa message");

    const String mytext = "Hello Lora";
    mytext.toCharArray(payload, 255);
    int lora_result = lsSendLoraWAN(&loraWANParams, &txParams, (byte *)payload, sizeof mytext);

    Serial.print("LoraWan result: ");
    Serial.println(lsErrorToString(lora_result));

    delay(1000);

    // Sent LoRaSat message
    Serial.println("Sending LoraSat message");
    const String mytext = "Hello Lacuna";
    mytext.toCharArray(payload, 255);
    int sat_result = lsSendLoraSatWAN(&loraWANParams, &SattxParams, (byte *)payload, sizeof mytext);

    Serial.print("LoraSatWan result: ");
    Serial.println(lsErrorToString(sat_result));

    delay(1000);
   */
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

