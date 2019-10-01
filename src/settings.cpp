#include "settings.h"

//#define debug
//#define serial_debug  Serial
#define FORCE_DEFAULT_SETTINGS // TODO: remove for production

settingsPacket_t settings_packet;
settingsPacket_t settings_packet_downlink;

/**
 * @brief Setttings get lorawan settings port
 * 
 * @return uint8_t port
 */
uint8_t settings_get_packet_port(void){
    return settings_packet_port;
}

/**
 * @brief Settings init - read and configure settings upon boot or update
 * 
 * @details The settings are read from eeprom if implemented or hardcoded defaults are used here. See .h file for packet contents description.
 * 
 */
void settings_init(void){
    //default settings
    settings_packet.data.lorawan_adr=0;
    settings_packet.data.lorawan_datarate=3;
    settings_packet.data.lorawan_txp=20;

    //check if valid settings present in eeprom 
    uint8_t eeprom_settings_address = EEPROM_DATA_START_SETTINGS;
    #ifndef FORCE_DEFAULT_SETTINGS
    if(EEPROM.read(eeprom_settings_address)==0xab){
        for(int i=0;i<sizeof(settingsData_t);i++){
            settings_packet.bytes[i]=EEPROM.read(eeprom_settings_address+1+i);
        }
    }
    #endif

    #ifdef serial_debug
        serial_debug.println("lorawan_load_settings()");
    #endif
}

/**
 * @brief Settings from downlink parses the binary lorawan packet and performs range validation on all values and applies them. Sotres to eeprom if implemented.
 * 
 */
void settings_from_downlink(void)
{
    // perform validation
    // copy to main settings
    settings_packet.data.lorawan_adr=constrain(settings_packet_downlink.data.lorawan_adr, 0, 1);
    settings_packet.data.lorawan_datarate=constrain(settings_packet_downlink.data.lorawan_datarate, 0, 5);
    settings_packet.data.lorawan_txp=constrain(settings_packet_downlink.data.lorawan_txp, 0, 20);

    uint8_t eeprom_settings_address = EEPROM_DATA_START_SETTINGS;
    EEPROM.write(eeprom_settings_address,0xab);
    for(int i=0;i<sizeof(settingsData_t);i++){
        EEPROM.write(eeprom_settings_address+1+i,settings_packet.bytes[i]);
    }
    //EEPROM.put(eeprom_settings_address,settings_packet.bytes); // does not work on the byte array
}

/**
 * @brief Settings send via lorawan
 * 
 */
boolean settings_send(void){
    #ifdef serial_debug
        serial_debug.println("settings_send()");
    #endif
    return lorawan_send(settings_packet_port, &settings_packet.bytes[0], sizeof(settingsData_t));
}