#include "module_pira.h"

// Before this was declared as Arduino String
#define NAME "pira"
#define serial_debug Serial

uint8_t MODULE_PIRA::configure(uint8_t * data, size_t * size)
{
#ifdef serial_debug
    serial_debug.print(NAME);
    serial_debug.print(": configure(");
    serial_debug.println(")");;
#endif

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

    uint32_t elapsed = millis() - timestamp;

    // Get next wake up value for display purposes
    public_data.data_2 = min(settings_packet.data.operational_wakeup, settings_packet.data.safety_sleep_period);

    // Timestamp is 0 only at boot.
    if (elapsed >= (min(settings_packet.data.operational_wakeup, settings_packet.data.safety_sleep_period) * 1000 ||  0 == timestamp))
    {
        // Do not turn on raspberry pi if voltage is too low
        uint16_t voltage = get_voltage_in_mv(MODULE_SYSTEM_BAN_MON_AN);
        if(voltage > MODULE_PIRA_UNDERVOLTAGE_THRESHOLD)
        {
            if (M_IDLE == flags)
            {
                timestamp = millis();
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
    return flags;
}

uint8_t MODULE_PIRA::initialize(void)
{
    settings_packet.data.read_interval = 10;
    settings_packet.data.send_interval = 1;
    settings_packet.data.status_battery = 10;
    settings_packet.data.safety_power_period = 600;
    settings_packet.data.safety_sleep_period = 600;
    settings_packet.data.safety_reboot = 120;
    settings_packet.data.operational_wakeup = 300;

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
    timestamp = 0;

    public_data.data_1 = 42;
    public_data.data_2 = 52;

    // Initially enable RaspberryPi power
    pinMode(MODULE_5V_EN, OUTPUT);
    digitalWrite(MODULE_5V_EN, LOW);
    pinMode(MODULE_PIRA_5V, OUTPUT);
    digitalWrite(MODULE_PIRA_5V, LOW);

    // Prepare status pin
    pinMode(MODULE_PIRA_STATUS, INPUT_PULLDOWN);

    // Start Uart communication
    MODULE_PIRA_SERIAL.begin(115200);
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

void MODULE_PIRA::uart_receive()
{
  //uart_command_receive();
    //while (MODULE_PIRA_SERIAL.available() > 0)
    //{
      //MODULE_PIRA_SERIAL.read();
    //}
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
        switch(first_char)
        {
#ifdef serial_debug
            serial_debug.print(NAME);
            serial_debug.print(": received( ");
            serial_debug.print(first_char);
            serial_debug.print(" )");
#endif

            //TODO: constrain values
            case 't':
                rtc_time_sync((time_t)data,false);
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
    MODULE_PIRA_SERIAL.write((int) command);
    MODULE_PIRA_SERIAL.write(':');
    MODULE_PIRA_SERIAL.write((int) ((data & 0xFF000000) >> 24));
    MODULE_PIRA_SERIAL.write((int) ((data & 0x00FF0000) >> 16));
    MODULE_PIRA_SERIAL.write((int) ((data & 0x0000FF00) >> 8));
    MODULE_PIRA_SERIAL.write((int) ( data & 0x000000FF));
    MODULE_PIRA_SERIAL.write('\n');
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

/**
 * @brief Sends status values over uart
 *
 * @return none (void)
 */
void MODULE_PIRA::send_status_values(void)
{
    uart_command_send('t', (uint32_t)rtc_time_read());
    uart_command_send('o', get_overview_value());
    //TODO battery currently not implemented?
    uart_command_send('b', (uint32_t) 0);
    uart_command_send('p', settings_packet.data.safety_power_period);
    uart_command_send('s', settings_packet.data.safety_sleep_period);
    uart_command_send('r', settings_packet.data.safety_reboot);
    uart_command_send('w', settings_packet.data.operational_wakeup);
    uart_command_send('a', (uint32_t)digitalRead(MODULE_PIRA_STATUS));
    uart_command_send('m', status_pira_state_machine);
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
 * @brief check if the state has timed out
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
    serial_debug.print(" ");
    serial_debug.print(flags);
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
            send_status_values();

            //Check status pin, if low then turn off power supply.
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
            flags = M_SEND;

            digitalWrite(MODULE_5V_EN, LOW);
            digitalWrite(MODULE_PIRA_5V, LOW);
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