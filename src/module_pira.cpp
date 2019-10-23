#include "module_pira.h"

#define serial_debug Serial
uint8_t MODULE_PIRA::configure(uint8_t *data, size_t *size){
    #ifdef serial_debug
        serial_debug.print(name);
        serial_debug.print(":configure(");
        serial_debug.println(")");;
    #endif
}

uint8_t MODULE_PIRA::get_settings_length(){
    return sizeof(module_settings_data_t);
}

uint8_t MODULE_PIRA::set_downlink_data(uint8_t *data, size_t *size){

}

module_flags_e MODULE_PIRA::scheduler(void){

  pira_state_machine();
  if(status_pira_state_machine != IDLE_PIRA)  {
    flags=M_RUNNING;
  }
  return flags;
}

uint8_t MODULE_PIRA::initialize(void){
    settings_packet.data.read_interval=10;
    settings_packet.data.send_interval=1;
    flags=M_IDLE;

    MODULE_PIRA_SERIAL.begin(115200);
    MODULE_PIRA_SERIAL.setWakeup(1);
    MODULE_PIRA_SERIAL.onReceive(Callback(&MODULE_PIRA::uart_receive, this));

    status_pira_state_machine = WAIT_STATUS_ON;
    state_prev = WAIT_STATUS_ON;

        // Get reset cause
    uint32_t status_error_reset = STM32L0.resetCause();

    // Initially enable RaspberryPi power
    pinMode(MODULE_PIRA_5V, OUTPUT);
    digitalWrite(MODULE_PIRA_5V, HIGH);

    // Prepare status pin
    pinMode(MODULE_PIRA_STATUS, INPUT_PULLDOWN);

    // Start I2C communication
    Wire.begin();

    // Start Uart communication
    MODULE_PIRA_SERIAL.begin(115200);
    while(!MODULE_PIRA_SERIAL){}

    // RTC init
    init_rtc(TIME_INIT_VALUE);

    #ifdef serial_debug
        serial_debug.print("Cause for reset = ");
        serial_debug.println(status_error_reset);
    #endif
    uart_command_send('e', status_error_reset);
}

uint8_t MODULE_PIRA::send(uint8_t *data, size_t *size){
    //form the readings_packet

    #ifdef serial_debug
        serial_debug.print(name);
        serial_debug.print(": send(");
        serial_debug.println(")");
    #endif

    memcpy(data,&readings_packet.bytes[0], sizeof(module_readings_data_t));
    *size=sizeof(module_readings_data_t);
    flags=M_IDLE;
    return 1;
}

void MODULE_PIRA::print_data(void){

}

uint8_t MODULE_PIRA::read(void){
    #ifdef serial_debug
        serial_debug.print(name);
        serial_debug.print(": read(");
        serial_debug.println(")");
    #endif

    flags=M_IDLE;
    return 1;
}

uint8_t MODULE_PIRA::running(void){
  // Receive any command from raspberry
  uart_command_receive();

  // Get the current time from RTC
  readings_packet.data.status_time = (uint64_t)time();
  #ifdef serial_debug
      time_t time_string = (time_t)readings_packet.data.status_time;
      serial_debug.print("Time as a basic string = ");
      serial_debug.println(ctime(&time_string));
      print_status_values();
  #endif

  // Update status values in not in IsDLE_PIRA state
  if(status_pira_state_machine != IDLE_PIRA)
  {
      send_status_values();
  }

  pira_state_machine();
}

void MODULE_PIRA::uart_receive()
{

    while (MODULE_PIRA_SERIAL.available() > 0)
    {
      MODULE_PIRA_SERIAL.read();
    }
}
/**
 * @brief RTC object, used to read and write time to rtc chip 
 */
ISL1208_RTC rtc; 



/**
 * @brief Function parses recived commands depending on starting character
 *
 * @param[in] *rxBuffer
 *
 * @return none (void)
 */
void MODULE_PIRA::uart_command_parse(uint8_t *rxBuffer)
{
    uint8_t firstChar = rxBuffer[0];
    uint8_t secondChar = rxBuffer[1];
    uint32_t data = 0;

    data = (rxBuffer[2] << 24) | 
           (rxBuffer[3] << 16) | 
           (rxBuffer[4] << 8) | 
           (rxBuffer[5]);

    if (secondChar == ':')
    {
        switch(firstChar)
        {
            case 't':
                //MODULE_PIRA_SERIAL.println("t: received");
                time((time_t)data);
                break;
            case 'p':
                //MODULE_PIRA_SERIAL.println("p: received");
                settings_packet.data.safety_power_period = data;
                break;
            case 's':
                //MODULE_PIRA_SERIAL.println("s: received");
                settings_packet.data.safety_sleep_period = data;
                break;
            case 'c':
                //MODULE_PIRA_SERIAL.println("c: received");
                //MODULE_PIRA_SERIAL.println("To be defined how to react on c: command");
                break;
            case 'r':
                //MODULE_PIRA_SERIAL.println("r: received");
                settings_packet.data.safety_reboot = data;
                break;
            case 'w':
                //MODULE_PIRA_SERIAL.println("w: received");
                settings_packet.data.operational_wakeup = data;
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
    MODULE_PIRA_SERIAL.write((int)command);
    MODULE_PIRA_SERIAL.write(':');
    MODULE_PIRA_SERIAL.write((int)((data & 0xFF000000)>>24));
    MODULE_PIRA_SERIAL.write((int)((data & 0x00FF0000)>>16));
    MODULE_PIRA_SERIAL.write((int)((data & 0x0000FF00)>>8));
    MODULE_PIRA_SERIAL.write((int)( data & 0x000000FF));
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
                    rxBuffer[rxIndex] != 'w')
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
    uart_command_send('t', (uint32_t)readings_packet.data.status_time);
    uart_command_send('o', get_overview_value());
    uart_command_send('b', (uint32_t)0);
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
    serial_debug.print("Battery level in V = ");
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
}

/**
 * @brief Gets overview value, it depends in which state is currently pira
 *
 * @return uint32
 */
uint32_t MODULE_PIRA::get_overview_value(void)
{
    // Calculate overview value
    if(status_pira_state_machine == WAIT_STATUS_ON || 
       status_pira_state_machine == WAKEUP)
    {
        return settings_packet.data.safety_power_period - pira_elapsed;
    }
    else if(status_pira_state_machine == REBOOT_DETECTION)
    { 
        return settings_packet.data.safety_reboot - pira_elapsed;
    }
    else
    {
        return 0;
    }
}

/**
 * @brief Intializes rtc and sets init time value
 *
 * @return none (void)
 */
void MODULE_PIRA::init_rtc(time_t t)
{
    rtc.begin();

    //Try to open the ISL1208
    if(rtc.isRtcActive()) //checks if the RTC is available on the I2C bus
    {
#ifdef serial_debug
        serial_debug.println("RTC detected!");
#endif
        //Check if we need to reset the time
        uint8_t powerFailed = read8(ISL1208_ADDRESS, ISL1208_SR); 

        if(powerFailed & 0x01)
        {
#ifdef serial_debug
            //The time has been lost due to a power complete power failure
            serial_debug.println("RTC has lost power! Resetting time...");
#endif
            //Set RTC time to Mon, 1 Jan 2018 00:00:00
            time(t);
        }
    }
    else
    {
#ifdef serial_debug
        serial_debug.println("RTC not detected!");
#endif
    }
}

/**
 * @brief Reads time from RTC
 *
 * @return time_t
 */
time_t MODULE_PIRA::time()
{
    //Setup a tm structure based on the RTC
    struct tm timeinfo;
    timeinfo.tm_sec = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_SC));
    timeinfo.tm_min = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_MN));

    //Make sure we get the proper hour regardless of the mode
    char hours = read8(ISL1208_ADDRESS, ISL1208_HR);
    if (hours & (1 << 7))
    {
        //RTC is in 24-hour mode
        timeinfo.tm_hour = bcd2bin(hours & 0x3F);
    }
    else
    {
        //RTC is in 12-hour mode
        timeinfo.tm_hour = bcd2bin(hours & 0x1F);

        //Check for the PM flag
        if (hours & (1 << 5))
        {
            timeinfo.tm_hour += 12;
        }
    }

    //Continue reading the registers
    timeinfo.tm_mday = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_DT));
    timeinfo.tm_mon  = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_MO)) - 1;
    timeinfo.tm_year = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_YR)) + 100;
    timeinfo.tm_wday = bcd2bin(read8(ISL1208_ADDRESS, ISL1208_DW));

    //Return as a timestamp
    return mktime(&timeinfo);
}

/**
 * @brief Writes time to RTC
 *
 * @return none (void)
 */
void MODULE_PIRA::time(time_t t)
{
    //Convert the time to a tm
    struct tm *timeinfo = localtime(&t);

    /* The clock has an 8 bit wide bcd-coded register (they never learn)
     * for the year. tm_year is an offset from 1900 and we are interested
     * in the 2000-2099 range, so any value less than 100 is invalid.
     */
    if (timeinfo->tm_year < 100)
    {
        return;
    }

    //Read the old SR register value
    char sr = read8(ISL1208_ADDRESS, ISL1208_SR);

    //Enable RTC writing
    write8(ISL1208_ADDRESS, ISL1208_SR, sr | (1 << 4));

    //Write the current time
    write8(ISL1208_ADDRESS, ISL1208_SC, bin2bcd(timeinfo->tm_sec));
    write8(ISL1208_ADDRESS, ISL1208_MN, bin2bcd(timeinfo->tm_min));
    write8(ISL1208_ADDRESS, ISL1208_HR, bin2bcd(timeinfo->tm_hour) | (1 << 7));
    write8(ISL1208_ADDRESS, ISL1208_DT, bin2bcd(timeinfo->tm_mday));
    write8(ISL1208_ADDRESS, ISL1208_MO, bin2bcd(timeinfo->tm_mon + 1));
    write8(ISL1208_ADDRESS, ISL1208_YR, bin2bcd(timeinfo->tm_year - 100));
    write8(ISL1208_ADDRESS, ISL1208_DW, bin2bcd(timeinfo->tm_wday & 7));

    //Disable RTC writing
    write8(ISL1208_ADDRESS, ISL1208_SR, sr);
}

/**
 * @brief Reads a register over I2C
 *
 * @param[in] addr
 * @param[in] reg
 *
 * @return char
 */
char MODULE_PIRA::read8(char addr, char reg)
{
    //Select the register
    Wire.beginTransmission(addr); //send I2C address of RTC
    Wire.write(reg); //status register
    Wire.endTransmission();

    //Read the 8-bit register
    Wire.requestFrom(addr, 1); // now get the bytes of data...

    //Return the byte
    return Wire.read();
}


/**
 * @brief Writes to a register over I2C
 *
 * @param[in] addr
 * @param[in] reg
 * @param[in] data
 *
 * @return none (void)
 */
void MODULE_PIRA::write8(char addr, char reg, char data)
{
    //Select the register
    Wire.beginTransmission(addr); //send I2C address of RTC
    Wire.write(reg); //status register

    //Write to register
    Wire.write(data);
    Wire.endTransmission();
}


/**
 * @brief Transitions to next state and saves the 
 * time when the state was entered.
 *
 */
void MODULE_PIRA::pira_state_transition(state_pira_e next)
{
  stateTimeoutStart = millis();
  state_prev=status_pira_state_machine;
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
    pira_elapsed = pira_elapsed/1000; 

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
char* MODULE_PIRA::return_state(state_pira_e status_pira_state_machine)
{
    static char buffer[20];

    if(status_pira_state_machine == IDLE_PIRA)
    {
        sprintf(buffer, "%s", "IDLE_PIRA");
    }
    if(status_pira_state_machine == WAIT_STATUS_ON)
    {
        sprintf(buffer, "%s", "WAIT_STATUS_ON");
    }
    if(status_pira_state_machine == WAKEUP)
    {
        sprintf(buffer, "%s", "WAKEUP");
    }
    if(status_pira_state_machine == REBOOT_DETECTION)
    {
        sprintf(buffer, "%s", "REBOOT_DETECTION");
    }

    return buffer;
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
#ifdef serial_debug
    serial_debug.print("fsm(");
    serial_debug.print(return_state(state_prev));
    serial_debug.print(" -> ");
    serial_debug.print(return_state(status_pira_state_machine));
    serial_debug.print(",");
    serial_debug.print(millis());
    serial_debug.print(",");
    serial_debug.print("Timeout = ");
    serial_debug.print(get_overview_value());
    serial_debug.println(")");
    serial_debug.flush();
#endif

    switch(status_pira_state_machine)
    {
        case IDLE_PIRA:

            //Typical usecase would be that operational_wakeup < safety_sleep_period
            if(settings_packet.data.operational_wakeup < settings_packet.data.safety_sleep_period)
            {
                stateTimeoutDuration = settings_packet.data.operational_wakeup;
            }
            else
            {
                stateTimeoutDuration = settings_packet.data.safety_sleep_period;
            }

            state_goto_timeout = WAIT_STATUS_ON;
            flags=M_SEND;

            // IDLE_PIRA state reached, turn off power for raspberry pi
            digitalWrite(MODULE_PIRA_5V, LOW);

            //TODO: implement waking up RPi via LoraWAN
            /*if(settings_packet.data.turnOnRpi)
            {
                settings_packet.data.turnOnRpi = false;

                //Change state
                pira_state_transition(WAIT_STATUS_ON);
            }*/
        break;

        case WAIT_STATUS_ON:

            stateTimeoutDuration = settings_packet.data.safety_power_period;
            state_goto_timeout = IDLE_PIRA;

            // WAIT_STATUS_ON state reached, turn on power for raspberry pi
            digitalWrite(MODULE_PIRA_5V, HIGH);

            // If status pin is read as high go to WAKEUP state
            if(digitalRead(MODULE_PIRA_STATUS))
            {
                pira_state_transition(WAKEUP);
            }
        break;

        case WAKEUP:

            stateTimeoutDuration = settings_packet.data.safety_power_period;
            state_goto_timeout = IDLE_PIRA;

            //Check status pin, if low then turn off power supply.
            if(!digitalRead(MODULE_PIRA_STATUS))
            {
                pira_state_transition(REBOOT_DETECTION);
            }
        break;

        case REBOOT_DETECTION:

            stateTimeoutDuration = settings_packet.data.safety_reboot;
            state_goto_timeout = IDLE_PIRA;

            if(digitalRead(MODULE_PIRA_STATUS))
            {
                // RPi rebooted, go back to wake up
                pira_state_transition(WAKEUP);
            }
        break;

        default:

            status_pira_state_machine=IDLE_PIRA;
        break;
    }

    // check if the existing state has timed out and transition to next state
    if(pira_state_check_timeout())
    {
        pira_state_transition(state_goto_timeout);
    }
}