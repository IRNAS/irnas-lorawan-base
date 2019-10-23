#include "module_configure.h"

// Default system module
// core settings global id 1
module *s_SYSTEM = new myModule<MODULE_SYSTEM>(2); // global id 2
module *s_GPS = new myModule<MODULE_GPS_UBLOX>(3); // global id 3
module *s_PIRA = new myModule<MODULE_PIRA>(4); // global id 4

// Array of modules to be loaded - project specific
module *modules[] = {s_SYSTEM,s_GPS,s_PIRA};