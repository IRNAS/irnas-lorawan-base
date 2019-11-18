#include "module_system.h"

// Before this was declared as Arduino String
#define NAME  "gps"
#define serial_debug Serial

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
    serial_debug.print(":configure(");
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
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(":get_settings_length(");
    serial_debug.println(")");
#endif
    return sizeof(module_settings_data_t);
}


uint8_t MODULE_SYSTEM::set_downlink_data(uint8_t *data, size_t *size)
{

}

module_flags_e MODULE_SYSTEM::scheduler(void)
{
    uint32_t elapsed = millis() - send_timestamp;
    if ((settings_packet.data.send_interval != 0)  &&  (elapsed >= (settings_packet.data.send_interval * 60 * 1000)))
    {
        if (M_IDLE == flags)
        {
            send_timestamp = millis();
            flags = M_SEND;
        }
#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(":scheduler(");
        serial_debug.println("send)");
#endif
        return flags;
    }

    elapsed = millis() - read_timestamp;
    if ((settings_packet.data.read_interval != 0)
            &&  (elapsed >= (settings_packet.data.read_interval * 1000)))
    {
        if (M_IDLE == flags)
        {
            read_timestamp = millis();
            flags = M_READ;
        }
#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(":scheduler(");
        serial_debug.println("_read_values)");
#endif
        return flags;
    }
    return flags;
}

uint8_t MODULE_SYSTEM::initialize(void)
{
    settings_packet.data.read_interval = 10;
    settings_packet.data.send_interval = 1;
    flags = M_IDLE;
}

uint8_t MODULE_SYSTEM::send(uint8_t *data, size_t *size)
{
    //from the readings_packet

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": send(");
    serial_debug.println(")");
#endif

    readings_packet.data.reset_cause = STM32L0.resetCause();

    readings_packet.data.vbus = (uint8_t) get_bits(STM32L0.getVDDA(), 0, 3.6, 8);

    readings_packet.data.battery_avg = (uint8_t) get_bits(r_battery.r_mean, 0, 4000, 8);
    readings_packet.data.battery_min = (uint8_t) get_bits(r_battery.r_min, 0, 4000, 8);
    readings_packet.data.battery_max = (uint8_t) get_bits(r_battery.r_max, 0, 4000, 8);
    clear_value(&r_battery);

    readings_packet.data.input_analog_avg = (uint8_t) get_bits(r_analog_input.r_mean, 0, 32000, 8);
    readings_packet.data.input_analog_min = (uint8_t) get_bits(r_analog_input.r_min, 0, 32000, 8);
    readings_packet.data.input_analog_max = (uint8_t) get_bits(r_analog_input.r_max, 0, 32000, 8);
    clear_value(&r_analog_input);

    readings_packet.data.temperature_avg = (uint8_t) get_bits(r_temperature.r_mean ,-20, 80, 8);
    readings_packet.data.temperature_min = (uint8_t) get_bits(r_temperature.r_min, -20, 80, 8);
    readings_packet.data.temperature_max = (uint8_t) get_bits(r_temperature.r_max, -20, 80, 8);
    clear_value(&r_temperature);

    memcpy(data,&readings_packet.bytes[0], sizeof(module_readings_data_t));
    *size = sizeof(module_readings_data_t);
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
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": read(");
    serial_debug.println(")");
#endif
    // temperature
    push_value(STM32L0.getTemperature(),&r_temperature);

    // battery voltage
    float voltage = 0;
#ifdef MODULE_SYSTEM_BAN_MON_AN
    pinMode(MODULE_SYSTEM_BAN_MON_EN, OUTPUT);
    digitalWrite(MODULE_SYSTEM_BAN_MON_EN, HIGH);
    delay(10);
    for (uint8_t i = 0; i < 16; i++)
    {
        voltage += analogRead(MODULE_SYSTEM_BAN_MON_AN);
        delay(1);
    }

    voltage = voltage / 16; // TODO: calibrate
    digitalWrite(MODULE_SYSTEM_BAN_MON_EN, LOW);
    pinMode(MODULE_SYSTEM_BAN_MON_EN, INPUT_PULLDOWN);

    push_value(voltage, &r_battery);
#endif /* MODULE_SYSTEM_BAN_MON_AN */

    float analog_input = 0;

    // analog input
#ifdef MODULE_SYSTEM_AINPUT_AN
    pinMode(MODULE_SYSTEM_AINPUT_EN, OUTPUT);
    digitalWrite(MODULE_SYSTEM_AINPUT_EN, HIGH);
    delay(10);
    for (uint8_t i = 0; i < 16; i++)
    {
        analog_input += analogRead(MODULE_SYSTEM_AINPUT_AN);
        delay(1);
    }
    analog_input = analog_input / 16; // TODO: calibrate
    digitalWrite(MODULE_SYSTEM_AINPUT_EN, LOW);
    pinMode(MODULE_SYSTEM_AINPUT_EN, INPUT_PULLDOWN);
    push_value(analog_input, &r_analog_input);
#endif /*MODULE_SYSTEM_AINPUT_AN */

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": read(");
    serial_debug.print("temperature_mean: ");
    serial_debug.print(r_temperature.r_mean);
    serial_debug.println(")");
#endif

    flags = M_IDLE;
    return 1;
}

uint8_t MODULE_SYSTEM::running(void)
{
}
