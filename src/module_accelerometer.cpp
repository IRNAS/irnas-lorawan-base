#include "module_accelerometer.h"

#define serial_debug Serial

// Before this was declared as Arduino String
#define NAME  "accelerometer"

extern event_e system_event;

/**
 * @brief called upon pin change
 * 
 */
void accelerometer_callback(void)
{
    system_event = EVENT_MOTION;
    STM32L0.wakeup();
}

uint8_t MODULE_ACCELEROMETER::configure(uint8_t * data, size_t * size)
{
    // copy to buffer
    module_settings_packet_t settings_packet_downlink;
    memcpy(&settings_packet_downlink.bytes[0], data, sizeof(module_settings_data_t));

    // validate settings value range 
    settings_packet.data.global_id = settings_packet_downlink.data.global_id;
    settings_packet.data.length = settings_packet_downlink.data.length;
    settings_packet.data.send_interval = constrain(settings_packet_downlink.data.send_interval, 0, 24 * 60);
    settings_packet.data.read_interval = constrain(settings_packet_downlink.data.read_interval, 0, 3600);

    lis.wake_up_free_fall_setup(settings_packet.data.triggered_threshold, settings_packet.data.triggered_duration, settings_packet.data.free_fall);

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(":configure(");
    serial_debug.print("s:");
    serial_debug.print(settings_packet.data.send_interval);
    serial_debug.print(" r:");
    serial_debug.print(settings_packet.data.read_interval);
    serial_debug.println(")");;
#endif
    return 0;
}

uint8_t MODULE_ACCELEROMETER::get_settings_length()
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(":get_settings_length(");
    serial_debug.println(")");;
#endif
    return sizeof(module_settings_data_t);
}

//TODO not implemented yet
uint8_t MODULE_ACCELEROMETER::set_downlink_data(uint8_t * data, size_t * size)
{
    return 0;
}

module_flags_e MODULE_ACCELEROMETER::scheduler(void)
{
    uint32_t elapsed = millis() - send_timestamp;

    if ((settings_packet.data.send_interval != 0) 
            && (elapsed >= (settings_packet.data.send_interval * 60 * 1000)))
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
            && (elapsed >= (settings_packet.data.read_interval * 1000))
            && (M_IDLE == flags))
    {
        read_timestamp = millis();
        flags = M_READ;

#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(":scheduler(");
        serial_debug.println("_read_values)");
#endif
    }
    return flags;
}

uint8_t MODULE_ACCELEROMETER::initialize(void)
{
    settings_packet.data.read_interval = 10;
    settings_packet.data.send_interval = 1;
    settings_packet.data.triggered_threshold = 1;
    settings_packet.data.triggered_duration = 1;
    settings_packet.data.free_fall=1;
    flags = M_ERROR;

    if(lis.begin() == false)
    {
#ifdef serial_debug
        serial_debug.print("accelerometer_init(");
        serial_debug.println("accel error)");
#endif
        return 0;
    }

    pinMode(MODULE_ACCELEROMETER_INT1, INPUT);
    attachInterrupt(digitalPinToInterrupt(MODULE_ACCELEROMETER_INT1), accelerometer_callback, RISING);
    lis.wake_up_free_fall_setup(settings_packet.data.triggered_threshold, settings_packet.data.triggered_duration, settings_packet.data.free_fall);
    flags = M_IDLE;
    return 1;
}

uint8_t MODULE_ACCELEROMETER::send(uint8_t * data, size_t * size)
{
    //from the readings_packet

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": send(");
    serial_debug.println(")");
#endif

    readings_packet.data.accel_x_avg = (uint8_t) get_bits(r_accel_x.r_mean, 2000, 2000, 8);
    readings_packet.data.accel_x_min = (uint8_t) get_bits(r_accel_x.r_min, -2000, 2000, 8);
    readings_packet.data.accel_x_max = (uint8_t) get_bits(r_accel_x.r_max, -2000, 2000, 8);
    clear_value(&r_accel_x);

    readings_packet.data.accel_y_avg = (uint8_t) get_bits(r_accel_y.r_mean, -2000, 2000, 8);
    readings_packet.data.accel_y_min = (uint8_t) get_bits(r_accel_y.r_min, -2000, 2000, 8);
    readings_packet.data.accel_y_max = (uint8_t) get_bits(r_accel_y.r_max, -2000, 2000, 8);
    clear_value(&r_accel_y);

    readings_packet.data.accel_z_avg = (uint8_t) get_bits(r_accel_z.r_mean, -2000, 2000, 8);
    readings_packet.data.accel_z_min = (uint8_t) get_bits(r_accel_z.r_min, -2000, 2000, 8);
    readings_packet.data.accel_z_max = (uint8_t) get_bits(r_accel_z.r_max, -2000, 2000, 8);
    clear_value(&r_accel_z);

    memcpy(data,&readings_packet.bytes[0], sizeof(module_readings_data_t));
    *size = sizeof(module_readings_data_t);
    flags = M_IDLE;
    return 1;
}

void MODULE_ACCELEROMETER::event(event_e event)
{
    if(EVENT_MOTION == event)
    {
#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(": motion(");
        serial_debug.println(")");
#endif
    }
}

void MODULE_ACCELEROMETER::print_data(void)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": print_data(");
    serial_debug.println(")");
#endif
}

uint8_t MODULE_ACCELEROMETER::read(void)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": read(");
    serial_debug.println(")");
#endif

    accel_data axis;
    axis = lis.read_accel_values();
    push_value(axis.x_axis, &r_accel_x);
    push_value(axis.y_axis, &r_accel_y);
    push_value(axis.z_axis, &r_accel_z);

    flags = M_IDLE;
    return 1;
}

uint8_t MODULE_ACCELEROMETER::running(void)
{
    return 0;
}
