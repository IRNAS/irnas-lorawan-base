#include <STM32L0.h> 
#include <TimerMillis.h>
#include "project.h"
#include "lorawan.h"
#include "project_utils.h"
#include "settings.h"
#include "LIS2DW12.h"

#define serial_debug  Serial

// General system variables
int8_t active_module = -1;



// All possible states in finite state machine
enum state_e
{
    INIT,
    LORAWAN_INIT,
    LORAWAN_JOIN_START,
    LORAWAN_JOIN_DONE,
    MODULE_INIT,
    APPLY_SETTINGS,
    IDLE,
    SETTINGS_SEND,
    MODULE_READ,
    MODULE_SEND,
    LORAWAN_TRANSMIT,
    HIBERNATION
};

// Variables dealing with FSM
state_e state = INIT;
state_e state_goto_timeout;
state_e state_prev = INIT;
uint32_t state_timeout_start;
uint32_t state_timeout_duration;



// Variable to monitor when the loop has been started
uint32_t event_loop_start = 0;

// sleep variable, if -1 then MCU does not go into deep sleep after loop(), 
// otherwise it goes into deep sleep for milliseconds specified.
// Reset the sleep after loop, set in every state if required.
int32_t sleep = -1; 

// Variable that keeps track of how many times lora failed to join, used in 
// LORAWAN_JOIN_START and LORA_JOIN_DONE states
int16_t lora_join_fail_count = 0;

// Defined in project_utils.h, it is set only by interrupt, and reset to EVENT_NONE
// in callback_periodic()
event_e system_event = EVENT_NONE;

// Last packet buffer
uint8_t last_packet[51]; // 51 is the max packet size supported
size_t last_packet_size;
uint8_t last_packet_port;
uint32_t last_packet_time;

/**
 * @brief Callback ocurring periodically for triggering events and wdt
 * 
 * @returns true if wakeup is needed, otherwise false
 */
bool callback_periodic(void)
{
    STM32L0.wdtReset();
    // if the main loop is running and not sleeping
    if (event_loop_start != 0)
    {
        uint32_t  elapsed = millis() - event_loop_start;
        // if loop has been running for more then 60s, then reboot system
        if(elapsed >= 60 * 1000)
        {
            STM32L0.reset();
        }
    }

    bool wakeup_needed = false;

    if (lorawan_settings_new == true)
    {
        wakeup_needed = true;
    }

    if (EVENT_NONE != system_event )
    {
        // iterate through modules on event
        for (size_t count = 0; count < N_MODULES; count++)
        {
            modules[count]->event(system_event);
        }
        system_event = EVENT_NONE;
    }

    // iterate through modules and check flags for activity requested
    for (size_t count = 0; count < N_MODULES; count++)
    {
        module_flags_e flag = modules[count]->scheduler();
        if ((flag != M_IDLE) && (flag != M_ERROR))
        {
            wakeup_needed= true;
        }
    }

    // wake up the system if required
    if (wakeup_needed)
    {
        return true;
    }
    else
    {
        return false;
    }
}

/**
 * @brief Puts MCU into deep sleep 
 * 
 * @param sleep duration in ms
 */
void system_sleep(uint32_t sleep)
{
    uint32_t remaining_sleep = sleep;
    while(remaining_sleep > 0)
    {
        if(remaining_sleep > 5000)
        {
            remaining_sleep = remaining_sleep - 5000;
            STM32L0.stop(5000);
        }
        else
        {
            STM32L0.stop(remaining_sleep);
            remaining_sleep = 0;
        }
        // wake-up if event generated
        if(callback_periodic())
        {
            return;
        }
    }
}

/**
 * @brief Puts MCU into delay 
 * 
 * @param delay duration in ms
 * @note Needed here for a bad workaround, musnt be in final version
 */
void system_delay(uint32_t sleep)
{
    uint32_t remaining_sleep = sleep;
    while(remaining_sleep > 0)
    {
        if(remaining_sleep > 5000)
        {
            remaining_sleep = remaining_sleep - 5000;
            delay(5000);
        }
        else
        {
            delay(remaining_sleep);
            remaining_sleep = 0;
        }
        // wake-up if event generated
        if(callback_periodic())
        {
            return;
        }
    }
}

/**
 * @brief check if the state has timed out
 * 
 * @ returns false if state_timeout_duration was set to zero or timeout was not
 *  reached yet, otherwise returns true.
 */
bool state_check_timeout(void)
{
    if (0 == state_timeout_duration)
    {
        return false;
    }
    uint32_t  elapsed = millis() - state_timeout_start;
    if (elapsed >= state_timeout_duration)
    {
        return true;
    }
    else
    {
        return false;
    }
}

// If this prototype is not here, compiler throws an error
// that state_e is not defined in this scope.
void state_transition(state_e next);

/**
 * @brief Changes to next state
 * 
 */
void state_transition(state_e next)
{
    state_timeout_start = millis();
    state = next;
}

/**
 * @brief Setup function called on boot
 * 
 */
void setup() 
{
    STM32L0.wdtEnable(18000);
    analogReadResolution(12);

    pinMode(BOARD_LED,OUTPUT);
    digitalWrite(BOARD_LED,HIGH);

    // Serial port debug setup
#ifdef serial_debug
    serial_debug.begin(115200);
    serial_debug.print("setup(serial debug begin): ");
    serial_debug.print("resetCause: ");
    serial_debug.println(STM32L0.resetCause(),HEX);
#endif
    
    // Starting state
    state = INIT;
}

/**
 * @brief Main system loop running the FSM
 * 
 */
void loop() 
{
#ifdef serial_debug
    serial_debug.print("fsm(");
    serial_debug.print(state_prev);
    serial_debug.print(">");
    serial_debug.print(state);
    serial_debug.print(",");
    serial_debug.print(sleep);
    serial_debug.print(",");
    serial_debug.print(millis());
    serial_debug.println(")");
    serial_debug.flush();
#endif

    sleep = -1; // reset the sleep after loop, set in every state if required
    event_loop_start = millis(); // start the timer of the loop
    // update prevous state
    state_prev = state;

    // FSM implementaiton for clarity of process loop
    switch (state)
    {
        case INIT:
            // defaults for timing out
            state_timeout_duration = 0;
            state_goto_timeout = INIT;
            // check i2c
            check_i2c();
            // setup default settings
            settings_init();
            // setup RTC
            rtc_init();
            // load settings, currently can not return an error, thus proceed directly
            state_transition(LORAWAN_INIT);
        break;

        case LORAWAN_INIT:
            // defaults for timing out
            state_timeout_duration = 1000;
            state_goto_timeout = INIT;
            if (lorawan_init())
            {
                state_transition(LORAWAN_JOIN_START);
            }
            else
            {
                // TODO: decide what to do if LoraWAN fails
                // Currently very harsh, doing full system reset
                //must be handled TODO
                STM32L0.reset();
            }
        break;

        case LORAWAN_JOIN_START:
            // defaults for timing out
            state_timeout_duration = 0;
            state_goto_timeout = LORAWAN_JOIN_DONE;
            lorawan_joinCallback(); // call join again
            lora_join_fail_count++;
            state_transition(LORAWAN_JOIN_DONE);
        break;

        case LORAWAN_JOIN_DONE:
            // defaults for timing out
            // join once every 20s for the first 10 tries 
            // join once an hour for the first day 
            // join once every day until successful
            if (lora_join_fail_count < 10)
            {
                state_timeout_duration = 20000;
            }
            else if (lora_join_fail_count < 24)
            {
                state_timeout_duration = 60 * 60 * 1000;
            }   
            else
            {
                state_timeout_duration = 24 * 60 * 60 * 1000;
            }

            state_goto_timeout = LORAWAN_JOIN_START;

            // transition
            if (lorawan_joined())
            {
                state_transition(MODULE_INIT);
                lora_join_fail_count = 0;
            }
            else
            {
                sleep = 5000;
            }
        break;

        case MODULE_INIT:
            // defaults for timing out
            state_timeout_duration = 10000;
            state_goto_timeout = IDLE;

            for (size_t count = 0; count < N_MODULES; count++)
            {
                modules[count]->initialize();
            }
            state_transition(IDLE);
        break;

        case APPLY_SETTINGS:
            // defaults for timing out
            state_timeout_duration = 10000;
            state_goto_timeout = IDLE;
            settings_from_downlink(&lorawan_settings_buffer[0], lorawan_settings_length);
            state_transition(SETTINGS_SEND);
        break;

        case IDLE:
            // defaults for timing out
            state_timeout_duration = 25 * 60 * 60 * 1000; // 25h maximum
            state_goto_timeout = INIT;
            sleep = -1;

            //LED status 
            digitalWrite(BOARD_LED, HIGH);

            if (true == lorawan_settings_new )
            {
                lorawan_settings_new = false;
                state_transition(APPLY_SETTINGS);
                break;
            }

            for (size_t count = 0; count < N_MODULES; count++)
            {
                module_flags_e flag = modules[count]->get_flags();

                // order is important as send has priority over read
                active_module = -1;
                if (M_RUNNING == flag)
                {
                    //run the module
                    modules[count]->running();
                    sleep = 500; // keep iterating every 500s
                    // break; // Do not break and process running modules in parallel
                }
            }
            for (size_t count = 0; count < N_MODULES; count++)
            {
                module_flags_e flag = modules[count]->get_flags();

                if (M_SEND == flag)
                {
                    state_transition(MODULE_SEND);
                    active_module = count;
                    break;
                }
                else if (M_READ == flag)
                {
                    state_transition(MODULE_READ);
                    active_module = count;
                    break;
                }
                else if (M_ERROR == flag)
                {
                    //do nothing for now
                    //state_transition(MODULE_READ);
                    active_module = count;
                    //break; // must not break here as it otherwise blocks the modules with higher counter value indefinitely
                }
                digitalWrite(BOARD_LED, LOW);
                //TODO handle other flags
            }

            if (-1 == active_module)
            {
                // No active module, sleep until an event is generated
                sleep = 0;
            }
        break;

        case SETTINGS_SEND:
            // defaults for timing out
            state_timeout_duration = 2000;
            state_goto_timeout = IDLE;
            // action
            // transition
            if (settings_send())
            {
                state_transition(LORAWAN_TRANSMIT);
            }
            else
            {
                sleep = 1000;
            }
        break;

        case MODULE_READ:
            // defaults for timing out
            state_timeout_duration = 2000;
            state_goto_timeout = IDLE;
            // transition
            // TODO> prepare module for sending and do so if success
            if (modules[active_module]->read())
            {
                state_transition(IDLE);
                active_module = -1;
            }
            else
            {
                sleep = 1000;
            }
        break;

        case MODULE_SEND:
            {
                // defaults for timing out
                state_timeout_duration = 2000;
                state_goto_timeout = IDLE;
                // transition
                uint8_t * data = &last_packet[0];
                size_t * size = &last_packet_size;
                serial_debug.print("Active module is : ");
                serial_debug.println(active_module);
                last_packet_port = modules[active_module]->get_global_id();
                last_packet_time = millis();
                if (modules[active_module]->send(data,size))
                {
                    lorawan_send(last_packet_port, &last_packet[0], last_packet_size);
                    state_transition(LORAWAN_TRANSMIT);
                    active_module = -1;
                }
                else
                {
                    sleep = 1000;
                }
            }
        break;

        case LORAWAN_TRANSMIT:
            // defaults for timing out, transmission should not take more then 10s
            state_timeout_duration = 15000;
            state_goto_timeout = LORAWAN_INIT;
            // transition
            // if tx fails, reinit lorawan
            if (lorawan_send_successful)
            {
                state_transition(IDLE);
            }
            else
            {
                // tx successful flag is expected in about 3s after sending, note lora rx windows most complete
                sleep = 5000;
            }
        break;

        case HIBERNATION:
            // defaults for timing out
            state_timeout_duration = 24 * 60 * 60 * 1000; // 24h maximum
            state_goto_timeout = INIT;
            // action
            sleep = 60000; // until an event
        break;

        default:
            state = IDLE;
        break;

    }

    // check if the existing state has timed out and transition to next state
    if (state_check_timeout())
    {
#ifdef serial_debug
        serial_debug.print("timeout(");
        serial_debug.print(state);
        serial_debug.println(")");
#endif
        state_transition(state_goto_timeout);
    }

    // reset the event loop start to show the loop has finished
    event_loop_start = 0;
    if (sleep > 0)
    {
        // Very bad workaround for PIRA. We need to figure out how to receive 
        // uart messages while in deep sleep, 
        // for now we will delay when pira is running.
        module_flags_e flag = modules[2]->get_flags(); //Get flag of Pira module
        if(flag == M_RUNNING)
        {
            serial_debug.println("GOING INTO DELAY");
            system_delay(sleep);
        }
        else
        {
            serial_debug.println("GOING INTO SLEEP");
            system_sleep(sleep);
        }
        sleep = 0;
    }
    else if (sleep == 0)
    {
        sleep = -1;
        module_flags_e flag = modules[2]->get_flags(); //Get flag of Pira module
        if(flag == M_RUNNING)
        {
            serial_debug.println("GOING INTO DELAY");
            system_delay(25 * 3600 * 1000); // max 25h
        }
        else
        {
            serial_debug.println("GOING INTO SLEEP");
            system_sleep(25 * 3600 * 1000); // max 25h
        }
    }
    else
    {
        sleep = -1;
    }
}
