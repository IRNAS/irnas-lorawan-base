#include "module_system.h"
#include "debug.h"

#ifdef  MODULE_SYSTEM_DEBUG
#define NAME  "system"
#define serial_debug  Serial
#endif

uint8_t MODULE_SYSTEM::configure(uint8_t * data, size_t * size)
{

    // copy to buffer
    module_settings_packet_t settings_packet_downlink;
    memcpy(&settings_packet_downlink.bytes[0], data, sizeof(module_settings_data_t));

    // validate settings value range 
    settings_packet.data.global_id = settings_packet_downlink.data.global_id;
    settings_packet.data.length = settings_packet_downlink.data.length;
    settings_packet.data.send_interval = constrain(settings_packet_downlink.data.send_interval, 0, 24 * 60);
    settings_packet.data.read_interval = constrain(settings_packet_downlink.data.read_interval, 0, 3600);

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": configure(");
    serial_debug.print("s:");
    serial_debug.print(settings_packet.data.send_interval);
    serial_debug.print(" r:");
    serial_debug.print(settings_packet.data.read_interval);
    serial_debug.println(")");
#endif
    return 0;
}

uint8_t MODULE_SYSTEM::get_settings_length()
{
    return sizeof(module_settings_data_t);
}


uint8_t MODULE_SYSTEM::set_downlink_data(uint8_t *data, size_t *size)
{

}

module_flags_e MODULE_SYSTEM::scheduler(void)
{
    return flags;
}

uint8_t MODULE_SYSTEM::initialize(void)
{
    settings_packet.data.read_interval = 10;
    settings_packet.data.send_interval = 1;
    readings_packet.data.time_alive =  0;
    flags = M_IDLE;
}

uint8_t MODULE_SYSTEM::send(uint8_t *data, size_t *size)
{
    flags = M_IDLE;
    return 1;
}

void MODULE_SYSTEM::event(event_e event)
{

}

void MODULE_SYSTEM::print_data(void)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": print_data(");
    serial_debug.println(")");
#endif
}

uint8_t MODULE_SYSTEM::read(void)
{
    flags = M_IDLE;
    return 1;
}

uint8_t MODULE_SYSTEM::running(void)
{
}

specific_public_data_t  MODULE_SYSTEM::getter()
{

}
/*** end of file ***/