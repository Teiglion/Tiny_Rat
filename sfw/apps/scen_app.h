#ifndef SCEN_APP_H
#define SCEN_APP_H

#include <stdint.h>
#include "sfw/hid_inject.h"
#include "hdw/FatFS/ff.h"
#include "sfw/text.h"
#include "sfw/app_core.h"
#include "sfw/apps/inject_app.h"

extern app scen_app;
void run_scen(const char *file);

#endif