#include "module_pira.h"
#include "debug.h"
#include "Int64String.h"
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>

#ifdef MODULE_PIRA_DEBUG
#define NAME "pira"
#define serial_debug  Serial
#endif

Adafruit_BME280 bme; // comm over I2C

uint8_t MODULE_PIRA::configure(uint8_t * data, size_t * size)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": configure(");
    serial_debug.println(")");;
#endif

    if (* size != sizeof(module_settings_data_t))
    {
        return 0;
    }
    // copy to buffer
    module_settings_packet_t settings_packet_downlink;
    memcpy(&settings_packet_downlink.bytes[0],data, sizeof(module_settings_data_t));
    // validate settings value range 
    settings_packet.data.global_id = settings_packet_downlink.data.global_id;
    settings_packet.data.length = settings_packet_downlink.data.length;

    settings_packet.data.read_interval = constrain(settings_packet_downlink.data.read_interval, 0, 0xff);
    settings_packet.data.send_interval = constrain(settings_packet_downlink.data.send_interval, 0, 0xff);
    settings_packet.data.safety_power_period = constrain(settings_packet_downlink.data.safety_power_period, 0, 0xffffffff);
    settings_packet.data.safety_sleep_period = constrain(settings_packet_downlink.data.safety_sleep_period, 0, 0xffffffff);
    settings_packet.data.safety_reboot = constrain(settings_packet_downlink.data.safety_reboot, 0, 0xffffffff);
    settings_packet.data.operational_wakeup = constrain(settings_packet_downlink.data.operational_wakeup, 0, 0xffffffff);
}

uint8_t MODULE_PIRA::get_settings_length()
{
    return sizeof(module_settings_data_t);
}

uint8_t MODULE_PIRA::set_downlink_data(uint8_t * data, size_t * size)
{

}

module_flags_e MODULE_PIRA::scheduler(void)
{

    uint32_t elapsed = millis() - rpi_turned_off_timestamp;

    // rpi_turned_off_timestamp is 0 only at boot.
    //if ((elapsed >= min(settings_packet.data.operational_wakeup, settings_packet.data.safety_sleep_period) * 1000) ||  0 == rpi_turned_off_timestamp)
    
    //Different way of checking if we have to run pira
    if (global_activate_pira > 0)
    {
#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(": scheduler(");
        serial_debug.println("ACTIVATED)");
#endif
        // Do not turn on raspberry pi if voltage is too low
        uint16_t voltage = get_voltage_in_mv(MODULE_SYSTEM_BAN_MON_AN);
        if(voltage > MODULE_PIRA_UNDERVOLTAGE_THRESHOLD)
        {
            if (M_IDLE == flags)
            {
                flags = M_RUNNING;
#ifdef serial_debug
                serial_debug.print(NAME);
                serial_debug.print(": scheduler(");
                serial_debug.println("run)");
#endif
            }
        }
        else
        {
#ifdef serial_debug
                serial_debug.print(NAME);
                serial_debug.print(": scheduler(");
                serial_debug.print("undervoltage lockout!, voltage = ");
                serial_debug.print(voltage);
                serial_debug.println(" mV)");
#endif
        }
    }
    else
    {
#ifdef serial_debug
        serial_debug.print(NAME);
        serial_debug.print(": scheduler(");
        serial_debug.println("DEACTIVATED)");
#endif
    }
    return flags;
}

uint8_t MODULE_PIRA::initialize(void)
{
    settings_packet.data.read_interval = 10;
    settings_packet.data.send_interval = 1;
    settings_packet.data.safety_power_period = 900;
    settings_packet.data.safety_sleep_period = 60;
    settings_packet.data.safety_reboot = 45;
    settings_packet.data.operational_wakeup = 60;

    readings_packet.data.empty_space = 0;
    readings_packet.data.photo_count = 0;
    readings_packet.data.status_time = 0;
    readings_packet.data.next_wakeup = 0;
    readings_packet.data.error_values = 0;

    // start the module in active state
    status_pira_state_machine = IDLE_PIRA;
    stateTimeoutStart = 0;
    stateTimeoutDuration = 0;
    state_prev = IDLE_PIRA;
    flags = M_IDLE;
    rpi_turned_off_timestamp = 0;
    rpi_power_pin_pulled_high = 0;
    rpi_power_pin_pulled_low = 0;

    public_data.data_1 = 0; 
    public_data.data_2 = 0;

    // Initially enable RaspberryPi power
    pinMode(MODULE_5V_EN, OUTPUT);
    digitalWrite(MODULE_5V_EN, LOW);
    pinMode(MODULE_PIRA_5V, OUTPUT);
    digitalWrite(MODULE_PIRA_5V, LOW);

    // Prepare status pin
    pinMode(MODULE_PIRA_STATUS, INPUT_PULLDOWN);

    // Start Uart communication
    MODULE_PIRA_SERIAL.begin(115200);

    // Enable bme sensor
    //uint8_t status = bme.begin();  
    // You can also pass in a Wire library object like &Wire2
    // status = bme.begin(0x76, &Wire2)
    //if (!status) {
    //    Serial.println("Could not find a valid BME280 sensor, check wiring, address, sensor ID!");
    //    Serial.print("SensorID was: 0x"); Serial.println(bme.sensorID(),16);
    //    Serial.print("        ID of 0xFF probably means a bad address, a BMP 180 or BMP 085\n");
    //    Serial.print("   ID of 0x56-0x58 represents a BMP 280,\n");
    //    Serial.print("        ID of 0x60 represents a BME 280.\n");
    //    Serial.print("        ID of 0x61 represents a BME 680.\n");
    //    while (1) delay(10);
    //}

}

uint8_t MODULE_PIRA::send(uint8_t * data, size_t * size)
{
    //from the readings_packet

#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": send(");
    serial_debug.println(")");
#endif

    readings_packet.data.status_time = rtc_time_read(); 
    readings_packet.data.next_wakeup = min(settings_packet.data.operational_wakeup, settings_packet.data.safety_sleep_period);
    readings_packet.data.cycle_duration = (rpi_power_pin_pulled_high - rpi_power_pin_pulled_low);

    //We need to reset timestamps to enable single reading
    rpi_power_pin_pulled_low = 0;
    rpi_power_pin_pulled_high = 0;

    serial_debug.println("Cycle_duration is: ");
    serial_debug.println(readings_packet.data.cycle_duration);

    memcpy(data, &readings_packet.bytes[0], sizeof(module_readings_data_t));
    *size = sizeof(module_readings_data_t);
    flags = M_IDLE;
    return 1;
}

void MODULE_PIRA::event(event_e event)
{

}

void MODULE_PIRA::print_data(void)
{

}

uint8_t MODULE_PIRA::read(void)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": read(");
    serial_debug.println(")");
#endif

    flags = M_IDLE;
    return 1;
}

uint8_t MODULE_PIRA::running(void)
{
  // Receive any command from raspberry
  pira_state_machine();
}

/**
 * @brief Function parses recived commands depending on starting character
 *
 * @param[in] *rxBuffer
 *
 * @return none (void)
 */
void MODULE_PIRA::uart_command_parse(uint8_t * rxBuffer)
{
    uint8_t first_char = rxBuffer[0];
    uint8_t second_char = rxBuffer[1];
    uint32_t data = 0;

    data = (rxBuffer[2] << 24) | 
           (rxBuffer[3] << 16) | 
           (rxBuffer[4] << 8) | 
           (rxBuffer[5]);

    if (second_char == ':')
    {
#ifdef serial_debug
            serial_debug.print(NAME);
            serial_debug.print(": received( ");
            serial_debug.print((char) first_char);
            serial_debug.println(" )");
#endif
        switch(first_char)
        {
            //TODO: constrain values
            case 't':

                rtc_time_sync((time_t) data, false);
            break;

            case 'p':
                settings_packet.data.safety_power_period = data;
            break;

            case 's':
                settings_packet.data.safety_sleep_period = data;
            break;
            
            case 'c':
                //To be defined how to react on c command
            break;
            
            case 'r':
                settings_packet.data.safety_reboot = data;
            break;
            
            case 'w':
                settings_packet.data.operational_wakeup = data;
            break;
            
            case 'f':
                readings_packet.data.empty_space = data;
            break;
            
            case 'i':
                readings_packet.data.photo_count = data;
                // Get photo count value for display purposes
                public_data.data_1 = data;
            break;
            
            default:
            break;
        }
    }
    else
        // TODO This serial will go back to RPI, should you send something else?
        MODULE_PIRA_SERIAL.print("Incorrect format, this shouldn't happen!");
}

/**
 * @brief Encodes and sends data over uart
 *
 * @param[in] command
 * @param[in] data
 *
 * @return none (void)
 */
void MODULE_PIRA::uart_command_send(char command, uint32_t data)
{
    MODULE_PIRA_SERIAL.print(command);
    MODULE_PIRA_SERIAL.print(':');
    MODULE_PIRA_SERIAL.println(data);
}

/**
 * @brief Receives uart data
 *
 * @detail Data should be of exact specified format,
 *         otherwise all received data is going to be rejected
 *
 * @return none (void)
 */
void MODULE_PIRA::uart_command_receive(void)
{
    uint8_t rxBuffer[RX_BUFFER_SIZE] = "";
    uint8_t rxIndex = 0;
    if (MODULE_PIRA_SERIAL.available() != 0)
    {
        delay(10); // Without delay code thinks that it 
                   // gets only first character first
                   // and then the rest of the string, 
                   // final result is that they are received seperatly.
                   // A short delay prevents that.
        while (MODULE_PIRA_SERIAL.available() > 0)
        {
            rxBuffer[rxIndex] = MODULE_PIRA_SERIAL.read();
            if (rxIndex == 0)
            {
                if (rxBuffer[rxIndex] != 't' &&
                    rxBuffer[rxIndex] != 'p' &&
                    rxBuffer[rxIndex] != 's' &&
                    rxBuffer[rxIndex] != 'c' &&
                    rxBuffer[rxIndex] != 'r' &&
                    rxBuffer[rxIndex] != 'w' &&
                    rxBuffer[rxIndex] != 'f' &&
                    rxBuffer[rxIndex] != 'i')
                {
                    // Anything received that is not by protocol is discarded!
                    rxIndex = 0;
                }
                else
                {
                    // By protocol, continue receiving.
                    rxIndex++;
                }
            }
            else if (rxIndex == 1)
            {
                if (rxBuffer[rxIndex] != ':')
                {
                    // Anything received that is not by protocol is discarded!
                    rxIndex = 0;
                }
                else
                {
                    // By protocol, continue receiving.
                    rxIndex++;
                }
            }
            else
            {
                if (rxBuffer[rxIndex] == '\n')
                {
                    // All data withing the packet has been received, 
                    // parse the packet and execute commands
                    if (rxIndex == (RX_BUFFER_SIZE - 1))
                    {
                        //MODULE_PIRA_SERIAL.print("I received: ");
                        //MODULE_PIRA_SERIAL.print(rxBuffer);
                        uart_command_parse(rxBuffer);
                        rxIndex = 0;
                    }
                    else if (rxIndex == (RX_BUFFER_SIZE - 2))
                    {
                        // Sent data could be number 10, 
                        // which in ascii is equal to \n.
                        // This else if statement prevents 
                        // the number 10 from being discarded.
                        rxIndex++;
                    }
                    else
                    {
                        // Incorrect length, clean up buffer
                        for (int i = 0; i < RX_BUFFER_SIZE; i++)
                        {
                            rxBuffer[i] = 0;
                        }
                        rxIndex = 0;
                    }
                }
                else if (rxIndex == (RX_BUFFER_SIZE - 1))
                {
                    // We reached max lenght, but no newline, empty buffer
                    for (int i = 0; i < RX_BUFFER_SIZE; i++)
                    {
                        rxBuffer[i] = 0;
                    }
                    rxIndex = 0;
                }
                else
                {
                    rxIndex++;
                    if (rxIndex > (RX_BUFFER_SIZE - 1))
                    {
                        rxIndex = 0;
                    }
                }
            }
        }
    }
}

uint64_t read_uint64(byte buffer[], int start) {
  uint64_t result = 0;
  for( int i=start + 7; i>=start; i--)
  {
    result <<= 8;
    result |= buffer[i];
  }
  return result;
}

/**
 * @brief Sends status values over uart
 *
 * @return none (void)
 */
void MODULE_PIRA::send_status_values(void)
{   
    uint64_t cam_serial_id = read_uint64(global_relay_payload, 27);
    uint32_t camera_voltage = (global_relay_payload[15] << 8) | global_relay_payload[14];

    MODULE_PIRA_SERIAL.println("START VALUES");
    MODULE_PIRA_SERIAL.print("g:");
    MODULE_PIRA_SERIAL.println(STM32L0.getTemperature());

    uart_command_send('v', camera_voltage);
    uart_command_send('b', get_voltage_in_mv(MODULE_SYSTEM_BAN_MON_AN));

    
    MODULE_PIRA_SERIAL.print("h:");
    MODULE_PIRA_SERIAL.println(int64String(cam_serial_id));

    uart_command_send('p', settings_packet.data.safety_power_period);
    uart_command_send('r', settings_packet.data.safety_reboot);
    
    MODULE_PIRA_SERIAL.println("END VALUES");
}

/**
 * @brief Prints status values to uart, used for debugging only
 *
 * @return none (void)
 */
void MODULE_PIRA::print_status_values(void)
{
#ifdef serial_debug
    serial_debug.print("Battery level in V = ");
    //TODO battery currently not implemented?
    serial_debug.println(0);
    serial_debug.print("safety_power_period =");
    serial_debug.println(settings_packet.data.safety_power_period);
    serial_debug.print("safety_sleep_period =");
    serial_debug.println(settings_packet.data.safety_sleep_period);
    serial_debug.print("safety_reboot = ");
    serial_debug.println(settings_packet.data.safety_reboot);
    serial_debug.print("operational_wakeup = ");
    serial_debug.println(settings_packet.data.operational_wakeup);
    serial_debug.print("Status Pin = ");
    serial_debug.println(digitalRead(MODULE_PIRA_STATUS));
    serial_debug.print("Overview = ");
    serial_debug.println(get_overview_value());
#endif
}

/**
 * @brief Gets overview value, it depends in which state is currently pira
 *
 * @return uint32
 */
uint32_t MODULE_PIRA::get_overview_value(void)
{
    // Calculate overview value
    if(WAIT_STATUS_ON == status_pira_state_machine 
        || WAKEUP == status_pira_state_machine)
    {
        return settings_packet.data.safety_power_period - pira_elapsed;
    }
    else if(REBOOT_DETECTION == status_pira_state_machine)
    { 
        return settings_packet.data.safety_reboot - pira_elapsed;
    }
    else
    {
        return 0;
    }
}


/**
 * @brief Transitions to next state and saves the 
 * time when the state was entered.
 *
 */
void MODULE_PIRA::pira_state_transition(state_pira_e next)
{
  stateTimeoutStart = millis();
  state_prev = status_pira_state_machine;
  status_pira_state_machine = next;
}

/**
 * @brief Check if the state has timed out
 *
 * @return bool
 */
bool MODULE_PIRA::pira_state_check_timeout(void)
{
    // stateTimeoutDuration can be disabled
    if(stateTimeoutDuration == 0)
    {
        return false;
    }

    pira_elapsed = millis() - stateTimeoutStart;

    // All values come in seconds, so we also need pira_elapsed in seconds
    pira_elapsed = pira_elapsed / 1000; 

    //check if we have been in the current state too long
    if(pira_elapsed >= stateTimeoutDuration)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Returns state string from given state enum
 *
 * @param[in] status_pira_state_machine
 *
 * @return char*
 */
char * MODULE_PIRA::return_state(state_pira_e status_pira_state_machine)
{
    switch (status_pira_state_machine)
    {
        case IDLE_PIRA:
            return "IDLE_PIRA";
        break;

        case START_PIRA:
            return "START_PIRA";
        break;

        case WAIT_STATUS_ON:
            return "WAIT_STATUS_ON";
        break;

        case WAKEUP:
            return "WAKEUP";
        break;

        case REBOOT_DETECTION:
            return "REBOOT_DETECTION";
        break;

        case STOP_PIRA:
            return "STOP_PIRA";
        break;

        default:
            return "ERROR";
        break;
    }
}

/**
 * @brief Returns module flag from given flag enum
 *
 * @param[in] flag
 *
 * @return char*
 */
char * MODULE_PIRA::decode_flag(module_flags_e flag)
{
    switch (flag)
    {
        case M_IDLE:
            return "M_IDLE";
        break;

        case M_READ:
            return "M_READ";
        break;

        case M_SEND:
            return "M_SEND";
        break;

        case M_RUNNING:
            return "M_RUNNING";
        break;

        case M_ERROR:
            return "M_ERROR";
        break;

        default:
            return "M_ERROR";
        break;
    }
}

/**
 * @brief Finite state machine loop for Raspberry Pi
 *
 * @param[in] safety_power_period
 * @param[in] safety_sleep_period
 * @param[in] operational_wakeup
 * @param[in] safety_reboot
 * @param[in] turnOnRpi
 *
 * @return none (void)
 */
void MODULE_PIRA::pira_state_machine()
{
    //update previous state
    state_prev=status_pira_state_machine;

#ifdef serial_debug
    pira_elapsed = millis() - stateTimeoutStart;
    serial_debug.print(NAME);
    serial_debug.print(": fsm(");
    serial_debug.print(return_state(state_prev));
    serial_debug.print(" -> ");
    serial_debug.print(return_state(status_pira_state_machine));
    serial_debug.print(" ");
    serial_debug.print(pira_elapsed / 1000);
    serial_debug.print("/");
    serial_debug.print(stateTimeoutDuration);
    serial_debug.print("s, ");
    serial_debug.print(decode_flag(flags));
    serial_debug.println(")");
#endif

    switch(status_pira_state_machine)
    {
        case IDLE_PIRA:

            /*stateTimeoutDuration = min(settings_packet.data.operational_wakeup, settings_packet.data.safety_sleep_period);
            state_goto_timeout = START_PIRA;

            // wake up immediately after boot
            if(stateTimeoutStart == 0)
            {
                pira_state_transition(START_PIRA);
            }*/
            pira_state_transition(START_PIRA);
        break;

        case START_PIRA:

            flags = M_RUNNING;
            pira_state_transition(WAIT_STATUS_ON);
        break;

        case WAIT_STATUS_ON:

            stateTimeoutDuration = settings_packet.data.safety_reboot;
            state_goto_timeout = STOP_PIRA;

            // WAIT_STATUS_ON state reached, turn on power for raspberry pi
            digitalWrite(MODULE_5V_EN, HIGH);
            digitalWrite(MODULE_PIRA_5V, HIGH);

            // This is reseted in send method, after it has been used to calculate cycle duration
            if(0 == rpi_power_pin_pulled_high)
            {
                rpi_power_pin_pulled_high = millis();
            }

            // If status pin is read as high go to WAKEUP state
            if(digitalRead(MODULE_PIRA_STATUS))
            {
                pira_state_transition(WAKEUP);
            }
        break;

        case WAKEUP:

            stateTimeoutDuration = settings_packet.data.safety_power_period;
            state_goto_timeout = STOP_PIRA;
            uart_command_receive();
            serial_debug.println("SENDING STATUS VALUES");
            send_status_values();

            //Check status pin, if low then go to reboot detection
            if(!digitalRead(MODULE_PIRA_STATUS))
            {
                pira_state_transition(REBOOT_DETECTION);
            }
        break;

        case REBOOT_DETECTION:

            stateTimeoutDuration = settings_packet.data.safety_reboot;
            state_goto_timeout = STOP_PIRA;

            if(digitalRead(MODULE_PIRA_STATUS))
            {
                // RPi rebooted, go back to wake up
                pira_state_transition(WAIT_STATUS_ON);
            }
        break;

        case STOP_PIRA:
            //We only get here if we timeout
            flags = M_IDLE;
            rpi_turned_off_timestamp = millis();
            
#ifdef serial_debug
            serial_debug.print(NAME);
            serial_debug.print(": fsm(");
            serial_debug.print("RPI turned off, wake up in: ");
            serial_debug.print(min(settings_packet.data.operational_wakeup, 
                                   settings_packet.data.safety_sleep_period));
            serial_debug.println("s)");
#endif

            // Calculate when will next wake up occur, 
            // current time plus wakeup value and save it into public_data
            public_data.data_2 = rtc_time_read() + 
                                 min(settings_packet.data.operational_wakeup,
                                     settings_packet.data.safety_sleep_period);

            // Turn off Rpi
            digitalWrite(MODULE_5V_EN, LOW);
            digitalWrite(MODULE_PIRA_5V, LOW);

            // This is reseted in send method, 
            // after it has been used to calculate cycle duration
            if(0 == rpi_power_pin_pulled_low)
            {
                rpi_power_pin_pulled_low = millis();
            }

            // Reset global variables connected with 
            // HackThePoacher functionality
            global_activate_pira = 0;
            global_pira_wakeup_reason = 0;
            serial_debug.print("Time to DEACTIVATE PIRA");
            pira_state_transition(IDLE_PIRA);
        break;

        default:
            status_pira_state_machine = IDLE_PIRA;
            flags = M_IDLE;
        break;
    }

    // check if the existing state has timed out and transition to next state
    if(pira_state_check_timeout())
    {
        pira_state_transition(state_goto_timeout);
    }
}

specific_public_data_t MODULE_PIRA::getter()
{
    return public_data;
}
/*** end of file ***/