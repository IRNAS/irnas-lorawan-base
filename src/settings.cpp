#include "settings.h"

#define serial_debug  Serial
//#define FORCE_DEFAULT_SETTINGS // TODO: remove for production

static const uint8_t settings_packet_port = 100;
settingsPacket_t settings_packet;
settingsPacket_t settings_packet_downlink;
uint16_t settings_buffer_total_length = 0;
uint8_t settings_buffer_tlv_type[N_MODULES_TOTAL]; // all modules + basic settings
size_t settings_buffer_tlv_length[N_MODULES_TOTAL]; // all modules + basic settings
uint8_t *settings_buffer_tlv_ptr[N_MODULES_TOTAL]; // all modules + basic settings

static uint8_t settings_buffer[1024]; // Note that at 0x1400 EEPROm LoraWAN things start, so must not run into that

/**
 * @brief Setttings get lorawan settings port
 * 
 * @return uint8_t port
 */
uint8_t settings_get_packet_port(void)
{
    return settings_packet_port;
}

/**
 * @brief Set the basic settings of the device
 * 
 * @param data 
 * @param length 
 * @return uint8_t 
 */
uint8_t settings_set_settings(uint8_t * data, size_t * size)
{

    // copy to buffer
    settingsPacket_t settings_packet_downlink;
    memcpy(&settings_packet_downlink.bytes[0],data, sizeof(settingsData_t));

    // validate settings value range 
    settings_packet.data.global_id = settings_packet_downlink.data.global_id;
    settings_packet.data.length = settings_packet_downlink.data.length;

    settings_packet.data.lorawan_datarate = constrain(settings_packet_downlink.data.lorawan_datarate, 0, 5);    // DR0/SF12 to DR5/SF7
    settings_packet.data.lorawan_adr =      constrain(settings_packet_downlink.data.lorawan_adr, 0, 1);         // ADR enable/disable
    settings_packet.data.lorawan_txp =      constrain(settings_packet_downlink.data.lorawan_txp, 0, 30);        // Configure TX power
    settings_packet.data.lorawan_reg =      constrain(settings_packet_downlink.data.lorawan_reg, 0, 1);         // Enable the regulatory limit
    settings_packet.data.resend_delay =     constrain(settings_packet_downlink.data.resend_delay, 0, 24*60);    // Resend delay in minutes
    settings_packet.data.resend_count =     constrain(settings_packet_downlink.data.resend_count, 0, 10);       // Number of times for this to be resent

    // Default packet value: 01 07 05 00 0F 00 00 00

#ifdef serial_debug
    serial_debug.print("basic");
    serial_debug.print(":configure(");
    serial_debug.print("dr:");
    serial_debug.print(settings_packet.data.lorawan_datarate);
    serial_debug.print(" adr:");
    serial_debug.print(settings_packet.data.lorawan_adr);
    serial_debug.print(" txp:");
    serial_debug.print(settings_packet.data.lorawan_txp);
    serial_debug.print(" res_d:");
    serial_debug.print(settings_packet.data.resend_delay);
    serial_debug.print(" res_c:");
    serial_debug.print(settings_packet.data.resend_count);
    serial_debug.println(")");;
#endif
    return 0;
}

/**
 * @brief Settings init - read and configure settings upon boot or update
 * 
 * @details The settings are read from eeprom if implemented or hardcoded defaults are used here. See .h file for packet contents description.
 * 
 */
void settings_init(void)
{
    // Array of type - global ids and associated lengths
#ifdef serial_debug
    serial_debug.print("settings_init(");
    serial_debug.println(")");
#endif

    // Basic settings element - global id 1
    settings_buffer_tlv_type[0] = 1;
    settings_buffer_tlv_length[0] = sizeof(settingsData_t);
    settings_buffer_tlv_ptr[0] = &settings_buffer[0];
    settings_buffer_total_length += settings_buffer_tlv_length[0];

#ifdef serial_debug
    serial_debug.print("settings_init(basic length:");
    serial_debug.print(settings_buffer_total_length);
    serial_debug.println(")");
#endif

    // Modules
    for (size_t count = 0; count < N_MODULES; count++)
    {
        delay(100);
        settings_buffer_tlv_type[count + 1] = modules[count]->get_global_id();
        settings_buffer_tlv_length[count + 1] = modules[count]->get_settings_length();

        // create pointers
        settings_buffer_tlv_ptr[count + 1]= &settings_buffer[settings_buffer_total_length];

        // calculate total length
        settings_buffer_total_length += settings_buffer_tlv_length[count + 1];

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

    if (settings_buffer_total_length > sizeof(settings_buffer))
    {
        settings_buffer_total_length = sizeof(settings_buffer);

#ifdef serial_debug
        serial_debug.print("settings_init(buffer too small");
        serial_debug.println(")");
#endif
    }

    // populate the array from eeprom
    // check if valid settings present in eeprom 
    uint8_t eeprom_settings_address = EEPROM_DATA_START_SETTINGS;

    if (0xac == EEPROM.read(eeprom_settings_address))
    {
        for (uint16_t count = 0; count < settings_buffer_total_length; count++)
        {
            settings_buffer[count] = EEPROM.read(eeprom_settings_address + 1 + count);
        }
    }

#ifndef FORCE_DEFAULT_SETTINGS
    // basic settings
    settings_set_settings(settings_buffer_tlv_ptr[0], &settings_buffer_tlv_length[0]);
    // modules
    for (size_t count = 0; count < N_MODULES; count++)
    {
        modules[count]->configure(settings_buffer_tlv_ptr[count + 1], &settings_buffer_tlv_length[count + 1]);
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
void settings_from_downlink(uint8_t * data, size_t length)
{
    size_t buffer_processed_length = 0;

    while(buffer_processed_length < length)
    {
        // check the data for header
        uint8_t in_global_id = data[buffer_processed_length];
        uint8_t in_length = data[buffer_processed_length + 1];

#ifdef serial_debug
        serial_debug.print("settings_from_downlink(id:");
        serial_debug.print(in_global_id);
        serial_debug.print(" len:");
        serial_debug.print(in_length);
        serial_debug.println(")");
#endif 

        if (0 == in_length)
        {
            return;
        }

        // find id and check length in array, then copy to main setting buffer
        for (size_t count = 0; count < sizeof(settings_buffer_tlv_type); count++)
        {
#ifdef serial_debug
            serial_debug.print("settings_received(mod:");
            serial_debug.print(settings_buffer_tlv_type[count]);
            serial_debug.print(" len:");
            serial_debug.print(settings_buffer_tlv_length[count]);
            serial_debug.print(" count:");
            serial_debug.print(count);
            serial_debug.print(" data:");

            for (uint16_t i = 0; i < settings_buffer_tlv_length[count];i++)
            {
                serial_debug.print(" 0x");
                serial_debug.print(data[buffer_processed_length+i],HEX);
            }
            serial_debug.println(")");
#endif 
            if (settings_buffer_tlv_type[count] == in_global_id)
            {
                if (settings_buffer_tlv_length[count] == in_length)
                {
                    // copy to main array first
                    memcpy(settings_buffer_tlv_ptr[count], &data[buffer_processed_length], settings_buffer_tlv_length[count]);

                    // basic settings
                    if (1 == settings_buffer_tlv_type[count])
                    {
                        settings_set_settings(settings_buffer_tlv_ptr[0], &settings_buffer_tlv_length[0]);
#ifdef serial_debug
                        serial_debug.print("settings_received(0");
                        serial_debug.println(")");
#endif 
                    }
                    // module settings
                    else
                    {
                        //this does not seem to be called correctly
                        modules[count-1]->configure(settings_buffer_tlv_ptr[count], &settings_buffer_tlv_length[count]);
                        modules[count-1]->get_settings_length();
#ifdef serial_debug
                        serial_debug.print("settings_received(");
                        serial_debug.print(modules[count-1]->get_global_id());
                        serial_debug.println(")");
#endif
                    }       
                }
                break;
            }
        }
        // this skips the received TLV if no matching modules found
        buffer_processed_length += in_length;
    }

    // Store to EEPROM
    uint8_t eeprom_settings_address = EEPROM_DATA_START_SETTINGS;
    EEPROM.write(eeprom_settings_address, 0xac);
    for (int i = 0; i<settings_buffer_total_length; i++)
    {
        EEPROM.write(eeprom_settings_address + 1 + i , settings_buffer[i]);
    }
#ifdef serial_debug
    serial_debug.print("settings_received(");
    serial_debug.println("eeprom stored)");
#endif
}

/**
 * @brief Settings send via lorawan
 * 
 */
boolean settings_send(void)
{
#ifdef serial_debug
    serial_debug.println("settings_send()");
#endif

    uint16_t lenght = settings_buffer_total_length;
    if (settings_buffer_total_length > LoRaWAN.getMaxPayloadSize())
    {
        lenght = LoRaWAN.getMaxPayloadSize();
#ifdef serial_debug
        serial_debug.print("settings larger then packet size, truncating");
        serial_debug.println("");
#endif
    }
    return lorawan_send(settings_packet_port, &settings_buffer[0], lenght);
}
