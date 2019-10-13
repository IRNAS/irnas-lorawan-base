#ifndef MODULE_CONFIGURE_H_
#define MODULE_CONFIGURE_H_

//include modules
//#include "module.h"
#include "module_system.h"
#include "module_gps_ublox.h"
#include "module_pira.h"

#define N_MODULES 2
#define N_MODULES_TOTAL N_MODULES+1

extern module *s_SYSTEM;
extern module *s_GPS;
extern module *s_PIRA;
extern module *modules[];

#endif