#include "project.h"

// Default system module
// core settings global id 1
//module *s_CORE = new myModule<MODULE_CORE>(1); // global id 1
module *s_SYSTEM = new myModule<MODULE_SYSTEM>(2); // global id 2
module *s_GPS = new myModule<MODULE_GPS_UBLOX>(3); // global id 3
module *s_PIRA = new myModule<MODULE_PIRA>(4); // global id 4
module *s_ACCEL = new myModule<MODULE_ACCELEROMETER>(5); // global id 5
module *s_MICROCLIMATE = new myModule<MODULE_MICROCLIMATE>(6); // global id 6

#ifdef PMP_v1
// Array of modules to be loaded - project specific
module *modules[] = {s_SYSTEM, s_GPS, s_PIRA};
#endif

#ifdef RHINO_v2_4
// Array of modules to be loaded - project specific
module *modules[] = {s_SYSTEM, s_GPS, s_ACCEL};
#endif

#ifdef LION_v2_3
// Array of modules to be loaded - project specific
module *modules[] = {s_SYSTEM, s_GPS, s_ACCEL};
#endif

#ifdef MICROCLIMATE_v2
// Array of modules to be loaded - project specific
module *modules[] = {s_SYSTEM, s_ACCEL, s_MICROCLIMATE};
#endif

