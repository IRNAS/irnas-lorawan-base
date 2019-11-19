# Documentation

## Overview of projects directory
```C
├── decoder.js
├── docs
|  ├── DOCS.md                      // documentation in md file
|  └── fsm_diagram.svg  
├── encoded_settings_template.json  // Created by .py script 
├── encoder.js                      
├── encoder_generator.py            // Generates encoder from header files 
├── manual_decoder.js
├── README.md
└── src
   ├── LIS2DW12.cpp                 // Driver for accelerometer
   ├── LIS2DW12.h                   
   ├── lorawan.cpp                  // Lorawan library
   ├── lorawan.h
   ├── main.ino                     // Main program
   ├── module.cpp                   
   ├── module.h                     // Module prototype
   ├── module_accelerometer.cpp     
   ├── module_accelerometer.h       // Module file for accelerometer
   ├── module_gps_ublox.cpp
   ├── module_gps_ublox.h           // Module file for gps
   ├── module_pira.cpp
   ├── module_pira.h                // Module file for PIRA functionality
   ├── module_system.cpp
   ├── module_system.h              // Module file for whole system
   ├── project.cpp          
   ├── project.h                    // Project specific defines        
   ├── project_utils.cpp
   ├── project_utils.h              // Common utility functions
   ├── rtc.cpp                     
   ├── rtc.h
   ├── settings.cpp
   └── settings.h                   // Deals with saving and extracing settings to and from EEPROM
```
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
* If `callback_periodic()` returns true it will exit the function and start the main loop again 
* Otherwise it will keep repeating the while loop until `remaining_sleep` variable reaches 0, then it will exit and start main loop.
* If `callback_periodic()` returns true that means that some event was generated which requires immediate action 

**bool callback_periodic()**

* Resets the watchdog
* Iterates through each modules `event()` method in case system_event is different than `EVENT_NONE`
* Iterate through modules and check flags for activity requests
* if wakeup is needed return true otherwise false

## Flow diagram of finite state machine
![fsm_diagram](fsm_diagram.svg)

## Motivation for and explanation of modules

Many Irnas projects use the same LoRa module by Murata in combination with GSP module and accelerometer.
All projects also require LoRa connectivity to send and receive data.
Writing very similar code over and over again would be counterproductive so we came up with a module design.
External peripherals or special functionalities are defined in modules. 
The idea is that after user implements all necessary methods and enables module inside project.h file, he won't have to do anything more, as main fsm will take care of this.
State machine will iterate through all modules and preform their functions. For example, following for loop will initialize all modules:
```
for (size_t count = 0; count < N_MODULES; count++)
{
    modules[count]->initialize();
}
```
Each module has private structure `module_settings_data_t` where it is written all the data that that module will be sending over LoRa, how often and so on.

To better understand this implementation, lets follow through accelerometer module example:

* User creates new class called MODULE_"name", in our case MODULE_ACCELEROMETER
* In public go all functions that are predefined in module class inside module.h. This functions are the only ones that are called from main.ino.  
* In private go all functions and variables needed for internal logic
* In project.cpp user adds new instance of module, following previous naming conventions, for example:  
`module *s_ACCEL = new myModule<MODULE_ACCELEROMETER>(5); // global id 5`
* And adds module to array of active modules below:
`module *modules[] = {s_SYSTEM,s_GPS,s_ACCEL};`

How templating and calling methods works?

* When function like `s_ACCEL->send(data, size)` is called when first go into template class myModule.
* In there all functions are inherited from `module` class.
* `send(data, size)` is executed, which returns result from `module.send(data, size)`
* Because module is an instance of class myModuleType which is an template, what is actually executed is this `s_ACCEL.send(data, size)`

In that way we ensure, that call to a function returns correct modules method.
