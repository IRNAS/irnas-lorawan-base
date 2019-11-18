# Documentation

## Overview of projects directory

## Overview of main.ino file

**void setup()**

* Enables watchdog
* Prepares serial debug

**void loop()**

* resets sleep variable
* marks when loop was started in event_loop_start variable
* updates previous state in state_prev variable
* Executes finite state machine with switch case
* Checks if timeout happened, if yes changes state 
* resets event_loop_start_variable
* Calls `system_sleep()` if needed
* loops back to start of `loop()`

**void system_sleep(uint32_t sleep)**

* It will put STM32L0 to deep sleep for 5 seconds and then call `callback_periodic()` function
* If `callback_periodic()` returns true it will exit the the function and start the main loop again 
* Otherwise it will keep repeating the while loop until `remaining_sleep` variable reaches 0, then it will exit and start main loop.
* If `callback_periodic()` returns true that means that some event was generated which requires immediate action 

**bool callback_periodic()**

* Resets the watchdog
* Iterates through each modules `event()` method in case system_event is different than `EVENT_NONE`
* Iterate through modules and check flags for activity requests
* if wakeup is needed return true otherwise false

## Flow diagram of finite state machine
![fsm_diagram](fsm_diagram.svg)
