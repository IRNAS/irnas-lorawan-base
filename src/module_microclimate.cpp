#include "module_microclimate.h"

// Before this was declared as Arduino String
#define NAME  "microclimate"
#define serial_debug Serial

// Dps310 object
Dps310 Dps310PressureSensor = Dps310();

// HDC2080 object
HDC2080 hdc2080 = HDC2080();

uint8_t MODULE_MICROCLIMATE::configure(uint8_t * data, size_t * size)
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

uint8_t MODULE_MICROCLIMATE::get_settings_length()
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(":get_settings_length(");
    serial_debug.println(")");
#endif
    return sizeof(module_settings_data_t);
}


uint8_t MODULE_MICROCLIMATE::set_downlink_data(uint8_t *data, size_t *size)
{

}

module_flags_e MODULE_MICROCLIMATE::scheduler(void)
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

uint8_t MODULE_MICROCLIMATE::initialize(void)
{
    settings_packet.data.read_interval = 10;
    settings_packet.data.send_interval = 1;

     //DSO130 sensor
    Dps310PressureSensor.begin(Wire);
    //hdc2080
    hdc2080.begin();

    flags = M_IDLE;
}

uint8_t MODULE_MICROCLIMATE::send(uint8_t *data, size_t *size)
{
    //from the readings_packet

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": send(");
    serial_debug.println(")");
#endif

    readings_packet.data.humidity_avg = (uint8_t) get_bits(r_humidity.r_mean, 0, 100, 16);
    readings_packet.data.humidity_min = (uint8_t) get_bits(r_humidity.r_min, 0, 100, 16);
    readings_packet.data.humidity_max = (uint8_t) get_bits(r_humidity.r_max, 0, 100, 16);
    clear_value(&r_humidity);

    readings_packet.data.pressure_avg = (uint8_t) get_bits(r_pressure.r_mean, 0, 110000, 16);
    readings_packet.data.pressure_min = (uint8_t) get_bits(r_pressure.r_min, 0, 110000, 16);
    readings_packet.data.pressure_max = (uint8_t) get_bits(r_pressure.r_max, 0, 110000, 16);
    clear_value(&r_pressure);

    readings_packet.data.temperature_avg = (uint8_t) get_bits(r_temperature.r_mean ,-40, 80, 8);
    readings_packet.data.temperature_min = (uint8_t) get_bits(r_temperature.r_min, -40, 80, 8);
    readings_packet.data.temperature_max = (uint8_t) get_bits(r_temperature.r_max, -40, 80, 8);
    clear_value(&r_temperature);

    memcpy(data,&readings_packet.bytes[0], sizeof(module_readings_data_t));
    *size = sizeof(module_readings_data_t);
    flags = M_IDLE;
    return 1;
}

void MODULE_MICROCLIMATE::event(event_e event)
{

}

void MODULE_MICROCLIMATE::print_data(void)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": print_data(");
    serial_debug.println(")");
#endif
}

uint8_t MODULE_MICROCLIMATE::read(void)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": read(");
    serial_debug.println(")");
#endif
    
    //DPOS130 read
    int32_t dsp310_temp;
    int32_t dsp310_pres;
    
    int ret = Dps310PressureSensor.measureTempOnce(dsp310_temp, 7);
    ret += Dps310PressureSensor.measurePressureOnce(dsp310_pres, 7);
    dsp310_pres=dsp310_pres;

    push_value(dsp310_pres,&r_pressure);
  
    hdc2080.read();
    float hdc2080_temp = hdc2080.getTemp();
    float hdc2080_hum = hdc2080.getHum();

    push_value(dsp310_pres,&r_temperature);
    push_value(dsp310_pres,&r_humidity);


#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": read(");
    serial_debug.print("temperature_mean: ");
    serial_debug.print(r_temperature.r_mean);
    serial_debug.print(" pressure_mean: ");
    serial_debug.print(r_pressure.r_mean);
    serial_debug.print(" humidity_mean: ");
    serial_debug.print(r_humidity.r_mean);
    serial_debug.println(")");
#endif

    flags = M_IDLE;
    return 1;
}

uint8_t MODULE_MICROCLIMATE::running(void)
{
}


/* *** Pressure altitude calculation ***
 * Original equation by Steve Randall (http://randomaerospace.com):
 * a [m] = ln(p0 [Pa] / p [Pa]) * 7238,3 [m]
 * a [m]:   altitude;                          unit: Meter
 * p0 [Pa]: pressure at sea level;             unit: Pascal
 * p [Pa]:  pressure at altitude to calculate; unit: Pascal
 * 7238,3:  correcture factor, found by Steve; unit: must be in Meter to do it mathematically correct.
 * In this sketch equation is adapted to Arduino's language.
 * You always have to use long-variables because integer will fail at altitudes above 32.768 km resp. pressure less than 1096 Pa.
 * Suggested pressure sensor: MS5607 ... MS5611 Series.
 */

// Function calculation of pressure altitude
long MODULE_MICROCLIMATE::calcPressureAltitude(long _PressureInPascal) {
  long _Temp = (log(101325.0 / _PressureInPascal) * 72383.0);
  long _PressureAltitude = 0;
  if (_Temp % 10 > 5) {
    _PressureAltitude = 1;
  }
  _PressureAltitude = _PressureAltitude + long(_Temp / 10);
  return _PressureAltitude;
}