#include <STM32L0.h> 
#include <TimerMillis.h>
#include "board.h"
#include "lorawan.h"
#include "project_utils.h"

#define serial_debug  Serial

enum state_e{
  INIT,
  LORAWAN_INIT,
  LORAWAN_JOIN_START,
  LORAWAN_JOIN_DONE,
  MODULE_INIT,
  APPLY_SETTINGS,
  IDLE,
  SETTINGS_SEND,
  MODULE_SEND,
  LORAWAN_TRANSMIT,
  HIBERNATION
};

state_e state = INIT;
state_e state_goto_timeout;
state_e state_prev = INIT;
unsigned long state_timeout_start;
unsigned long state_timeout_duration;
// Variable to monitor when the loop has been started
unsigned long event_loop_start = 0;
long sleep = -1; // reset the sleep after loop, set in every state if required
long lora_join_fail_count=0;

// function prototypes because Arduino failes if using enum otherwise
boolean callbackPeriodic(void);
void state_transition(state_e next);
bool state_check_timeout(void);

/**
 * @brief Callback ocurring periodically for triggering events and wdt
 * 
 */
boolean callbackPeriodic(void){
  //periodic.start(callbackPeriodic, 5000);
  STM32L0.wdtReset();
  
  /*#ifdef serial_debug
    serial_debug.print("wdt(): ");
    serial_debug.println(millis());
  #endif*/

  // determine which events need to be scheduled, except in hibernation
  if(state!=HIBERNATION){
    //TODO call scheduler events of all modules
  }

  // if the main loop is running and not sleeping
  if(event_loop_start!=0){
    unsigned long elapsed = millis()-event_loop_start;
    // if loop has been running for more then 60s, then reboot system
    if(elapsed>=60*1000){
      STM32L0.reset();
    }
  }

  // check all modules for flags that indicate activity

  return false;
}

/**
 * @brief change to next state and implement a timeout for each state
 * 
 */
void state_transition(state_e next){
  // mark the time state has been entered
  state_timeout_start = millis();
  // move to the following state
  state=next;
}

/**
 * @brief check if the state has timed out
 * 
 */
bool state_check_timeout(void){
  // timeout can be disabled
  if(state_timeout_duration==0){
    return false;
  }
  unsigned long elapsed = millis()-state_timeout_start;
  //check if we have been in the existing state too long
  if(elapsed >=state_timeout_duration){
    return true;
  }
  return false;
}

/**
 * @brief Setup function called on boot
 * 
 */
void setup() {
  //STM32L0.stop(60000); //limits the reboot continuous cycle from happening for any reason, likely low battery
  // Watchdog
  STM32L0.wdtEnable(18000);
  analogReadResolution(12);

  // Serial port debug setup
  #ifdef serial_debug
    serial_debug.begin(115200);
  #endif
  #ifdef serial_debug
    serial_debug.print("setup(serial debug begin): ");
    serial_debug.print("resetCause: ");
    serial_debug.println(STM32L0.resetCause(),HEX);
  #endif

  // start the FSM with LoraWAN init
  state = INIT;
}

/**
 * @brief Main system loop running the FSM
 * 
 */
void loop() {
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
  callbackPeriodic();

  // update prevous state
  state_prev=state;
  // FSM implementaiton for clarity of process loop
  switch (state)
  {
  case INIT:
    // defaults for timing out
    state_timeout_duration=0;
    state_goto_timeout=INIT;
    // LED diode
    // setup default settings
    settings_init();
    // load settings, currently can not return an error, thus proceed directly
    state_transition(LORAWAN_INIT);
    break;
  case LORAWAN_INIT:
    // defaults for timing out
    state_timeout_duration=1000;
    state_goto_timeout=INIT;
    // transition
    // if initialization successful, move forward
    if(lorawan_init()){
      state_transition(LORAWAN_JOIN_START);
    }
    else{
      // TODO: decide what to do if LoraWAN fails
      // Currently very harsh, doing full system reset
      STM32L0.reset();
    }
    break;
  case LORAWAN_JOIN_START:
    // defaults for timing out
    state_timeout_duration=0;
    state_goto_timeout=LORAWAN_JOIN_DONE;
    lorawan_joinCallback(); // call join again
    lora_join_fail_count++;
    state_transition(LORAWAN_JOIN_DONE);
    break;
  case LORAWAN_JOIN_DONE:
    // defaults for timing out
    // join once every 20s for the first 10 tries 
    // join once an hour for the first day 
    // join once every day until successful
    if(lora_join_fail_count<10){
      state_timeout_duration=20000;
    }
    else if(lora_join_fail_count<24){
      state_timeout_duration=60*60*1000;
    }    // TODO: apply settings to all modules
    else{
      state_timeout_duration=24*60*60*1000;
    }
    state_goto_timeout=LORAWAN_JOIN_START;
    // transition
    if(lorawan_joined()){
      state_transition(MODULE_INIT);
      lora_join_fail_count=0;
    }
    else{
      sleep=5000;
    }
    break;
  case MODULE_INIT:
    // defaults for timing out
    state_timeout_duration=10000;
    state_goto_timeout=IDLE;
    // setup default settings

    // initialize modules and apply settings to them
    state_transition(APPLY_SETTINGS);
    break;
  case APPLY_SETTINGS:
    // defaults for timing out
    state_timeout_duration=10000;
    state_goto_timeout=IDLE;
    // setup default settings

    // initialize modules and apply settings to them

    // transition
    if(true){
      state_transition(IDLE);
    }
    break;
  case IDLE:
    // defaults for timing out
    state_timeout_duration=25*60*60*1000; // 25h maximum
    state_goto_timeout=INIT;

    if(true){
    // check if any modules has flagged the requirement to be serviced or data sent
    }
    else{
      // sleep until an event is generated
      sleep=0;
    }
    break;
  case SETTINGS_SEND:
    // defaults for timing out
    state_timeout_duration=2000;
    state_goto_timeout=IDLE;
    // action
    // transition
    if(settings_send()){
      state_transition(LORAWAN_TRANSMIT);
    }
    else{
      sleep=1000;
    }
    break;
  case MODULE_SEND:
    // defaults for timing out
    state_timeout_duration=2000;
    state_goto_timeout=IDLE;
    // transition
    // TODO> prepare module for sending and do so if success
    if(true){
      state_transition(LORAWAN_TRANSMIT);
    }
    else{
      sleep=1000;
    }
    break;
  case LORAWAN_TRANSMIT:
    // defaults for timing out, transmission should not take more then 10s
    state_timeout_duration=15000;
    state_goto_timeout=LORAWAN_INIT;
    // transition
    // if tx fails, reinit lorawan
    if(lorawan_send_successful){
      state_transition(IDLE);
    }
    else{
      // tx successful flag is expected in about 3s after sending, note lora rx windows most complete
      sleep=5000;
    }
    break;
  case HIBERNATION:
    // defaults for timing out
    state_timeout_duration=24*60*60*1000; // 24h maximum
    state_goto_timeout=INIT;
    // action
    sleep=60000; // until an event
    break;
  default:
    state=IDLE;
    break;
  }

  // check if the existing state has timed out and transition to next state
  if(state_check_timeout()){
    #ifdef serial_debug
      serial_debug.print("timeout(");
      serial_debug.print(state);
      serial_debug.println(")");
    #endif
    state_transition(state_goto_timeout);
  }

  // reset the event loop start to show the loop has finished
  event_loop_start = 0;
  if(sleep>0){
    system_sleep(sleep);
    sleep=0;
  }
  else if(sleep==0){
    sleep=-1;
    system_sleep(25*3600*1000); // max 25h
  }
  else{
    sleep=-1;
  }
}

void system_sleep(unsigned long sleep){
  unsigned long remaining_sleep = sleep;
  callbackPeriodic();
  while(remaining_sleep>0){
    if(remaining_sleep>5000){
      remaining_sleep=remaining_sleep-5000;
      STM32L0.stop(5000);
    }
    else{
      STM32L0.stop(remaining_sleep);
      remaining_sleep=0;
    }
    //wake-up
    if(callbackPeriodic()){
      return;
    }
  }
}
