// This program will:
// Send Lacuna a message in the loop to check RSSI 
// Send Lora a message in the loop to check RSSI

#include "LoRaWAN.h"
#include <LibLacuna.h>
const char *lora_devAddr = "26013006";
const char *lora_nwkSKey = "C9290F8F24335E8B0B938F65D6723EC5";
const char *lora_appSKey = "BADCBCE21B50FF81F106CEC3EC41CF66";

uint8_t lora_payload[4]   = {0xde, 0xad, 0xbe, 0xef };
uint8_t lacuna_payload[4] = {0xba, 0xdc, 0x0f, 0xfe };

lsLoraWANParams loraWANParams;
lsLoraTxParams txParams;

void setup_lacuna(void)
{
    // Keys where we will be transmiting
    uint8_t lacuna_networkKey[] = { 0x44, 0xC6, 0x13, 0xDC, 0x93, 0x6D, 0xC7, 0x5B, 0x30, 0xC3, 0x73, 0x60, 0xD1, 0xD1, 0xE4, 0x62 };
    uint8_t lacuna_appKey[] = { 0x52, 0x9E, 0x70, 0x45, 0xCC, 0x4B, 0x13, 0xE5, 0x5B, 0xE1, 0xFD, 0x15, 0xD7, 0x71, 0xF5, 0x48 };
    uint8_t lacuna_deviceAddress[] =  { 0x26, 0x01, 0x3B, 0x3D };

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
        Serial.print("E22/SX1262: ");
        Serial.println(lsErrorToString(result));
    }

    // LoRaWAN session parameters
    lsCreateDefaultLoraWANParams(&loraWANParams, 
                                 lacuna_networkKey, 
                                 lacuna_appKey, 
                                 lacuna_deviceAddress);
    loraWANParams.txPort = 1;
    loraWANParams.rxEnable = true;

    // transmission parameters for terrestrial LoRa
    lsCreateDefaultLoraTxParams(&txParams);
}

void send_lacuna()
{
    Serial.println("Sending Lacuna message");

    int lora_result = lsSendLoraWAN(&loraWANParams, &txParams, (byte *)lacuna_payload, 4);
}

void setup()
{
    Serial.begin(115200);

    LoRaWAN.begin(EU868);
    LoRaWAN.addChannel(1, 868300000, 0, 6);
    LoRaWAN.addChannel(3, 867100000, 0, 5);
    LoRaWAN.addChannel(4, 867300000, 0, 5);
    LoRaWAN.addChannel(5, 867500000, 0, 5);
    LoRaWAN.addChannel(6, 867700000, 0, 5);
    LoRaWAN.addChannel(7, 867900000, 0, 5);
    LoRaWAN.addChannel(8, 868800000, 7, 7);
    LoRaWAN.setRX2Channel(869525000, 3);
    
    LoRaWAN.setDutyCycle(false);
    LoRaWAN.joinABP(lora_devAddr, lora_nwkSKey, lora_appSKey);

    pinMode(PH0, OUTPUT);
    digitalWrite(PH0, HIGH);

    setup_lacuna();
}

void loop()
{
    Serial.println("Sending Lora message");
    LoRaWAN.sendPacket(4, lora_payload, 4, false);
    delay(1000);
    send_lacuna();
    Serial.println("Wait 5 sec");
    delay(5000);
}
