#include "settings.h"

#define serial_debug  Serial
//#define FORCE_DEFAULT_SETTINGS // TODO: remove for production

static const uint8_t settings_packet_port = 100;
settingsPacket_t settings_packet;
settingsPacket_t settings_packet_downlink;
uint16_t settings_buffer_total_length=0;
uint8_t settings_buffer_tlv_type[N_MODULES_TOTAL]; // all modules + basic settings
uint8_t settings_buffer_tlv_length[N_MODULES_TOTAL]; // all modules + basic settings
uint8_t *settings_buffer_tlv_ptr[N_MODULES_TOTAL]; // all modules + basic settings

static uint8_t settings_buffer[1024]; // Note that at 0x1400 EEPROm LoraWAN things start, so must not run into that

/**
 * @brief Setttings get lorawan settings port
 * 
 * @return uint8_t port
 */
uint8_t settings_get_packet_port(void){
    return settings_packet_port;
}

/**
 * @brief Set the basic settings of the device
 * 
 * @param data 
 * @param length 
 * @return uint8_t 
 */
uint8_t settings_set_settings(uint8_t *data, uint16_t length){

if(length != sizeof(settingsData_t)){
  return 0;
}

memcpy(&settings_packet_downlink.bytes[0],data, sizeof(settingsData_t));
// validate settings value range 

}

/**
 * @brief Settings init - read and configure settings upon boot or update
 * 
 * @details The settings are read from eeprom if implemented or hardcoded defaults are used here. See .h file for packet contents description.
 * 
 */
void settings_init(void){
    // Array of type - global ids and associated lengths
    #ifdef serial_debug
        serial_debug.print("settings_init(");
        serial_debug.println(")");
    #endif

    // Basic settings element - global id 1
    settings_buffer_tlv_type[0]=1;
    settings_buffer_tlv_length[0]=sizeof(settingsData_t);
    settings_buffer_tlv_ptr[0]=&settings_buffer[0];
    settings_buffer_total_length+=settings_buffer_tlv_length[0];
    #ifdef serial_debug
        serial_debug.print("settings_init(basic length:");
        serial_debug.print(settings_buffer_total_length);
        serial_debug.println(")");
    #endif

    // Modules
    for (size_t count = 0; count < N_MODULES; count++){
        delay(100);
        settings_buffer_tlv_type[count+1]=1;
        settings_buffer_tlv_length[count+1]=modules[count]->get_settings_length();
        // create pointers
        settings_buffer_tlv_ptr[count+1]=&settings_buffer[settings_buffer_total_length];
        // calculate total length
        settings_buffer_total_length+=settings_buffer_tlv_length[count+1];
        #ifdef serial_debug
            serial_debug.print("settings_init(id:");
            serial_debug.print(modules[count]->get_global_id());
            serial_debug.print(" len:");
            serial_debug.print(modules[count]->get_settings_length());
            serial_debug.println(")");
        #endif
    }

    #ifdef serial_debug
        serial_debug.print("settings_init(length:");
        serial_debug.print(settings_buffer_total_length);
        serial_debug.println(")");
    #endif

    if (settings_buffer_total_length>sizeof(settings_buffer)){
        settings_buffer_total_length=sizeof(settings_buffer);
        #ifdef serial_debug
            serial_debug.print("settings_init(buffer too small");
            serial_debug.println(")");
        #endif
    }

    // populate the array from eeprom
    // check if valid settings present in eeprom 
    uint8_t eeprom_settings_address = EEPROM_DATA_START_SETTINGS;

    if(EEPROM.read(eeprom_settings_address)==0xac){
        for(int count=0;count<settings_buffer_total_length;count++){
            settings_buffer[count]=EEPROM.read(eeprom_settings_address+1+count);
        }
    }

    #ifndef FORCE_DEFAULT_SETTINGS

    // basic settings
    memcpy(&settings_packet.bytes[0],settings_buffer_tlv_ptr[0], (size_t)settings_buffer_tlv_length[0]);
    // modules
    for (size_t count = 0; count < N_MODULES; count++){
        modules[count]->set_settings(settings_buffer_tlv_ptr[count+1],settings_buffer_tlv_length[count+1]);
    }

    #endif

    #ifdef serial_debug
        serial_debug.println("lorawan_load_settings()");
    #endif
}

/**
 * @brief Receive values from Lorawan and write them to the general buffer, then apply to each module
 * 
 */
void settings_from_downlink(uint8_t* data, size_t length){
    size_t buffer_processed_length = 0;

    while(buffer_processed_length<length){
        // check the data for header
        uint8_t in_global_id=data[buffer_processed_length++];
        uint8_t in_length=data[buffer_processed_length++];
        #ifdef serial_debug
            serial_debug.print("settings_from_downlink(id:");
            serial_debug.print(in_global_id);
            serial_debug.print(" len:");
            serial_debug.print(in_length);
            serial_debug.println(")");
        #endif 
        // find id and check length in array, then copy to main setting buffer
        for (size_t count = 0; count < sizeof(settings_buffer_tlv_type); count++){
            if(settings_buffer_tlv_type[count]==in_global_id){
                if(settings_buffer_tlv_length[count]==in_length){
                    // copy to main array first
                    memcpy(settings_buffer_tlv_ptr[count],&data[buffer_processed_length], settings_buffer_tlv_length[count]);
                    buffer_processed_length+=settings_buffer_tlv_length[count];
                    // basic settings
                    if(settings_buffer_tlv_type[count]==0){
                        settings_set_settings(settings_buffer_tlv_ptr[count+1],settings_buffer_tlv_length[count+1]);
                       #ifdef serial_debug
                            serial_debug.print("settings_received(0");
                            serial_debug.println(")");
                        #endif 
                    }
                    // module settings
                    else{
                        modules[count]->set_settings(settings_buffer_tlv_ptr[count+1],settings_buffer_tlv_length[count+1]);
                        #ifdef serial_debug
                            serial_debug.print("settings_received(");
                            serial_debug.print(modules[count]->get_global_id());
                            serial_debug.println(")");
                        #endif
                    }       
                }
                break;
            }
        }
    }

    // Store to EEPROM
    uint8_t eeprom_settings_address = EEPROM_DATA_START_SETTINGS;
    EEPROM.write(eeprom_settings_address,0xac);
    for(int i=0;i<settings_buffer_total_length;i++){
        EEPROM.write(eeprom_settings_address+1+i,settings_buffer[i]);
    }
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