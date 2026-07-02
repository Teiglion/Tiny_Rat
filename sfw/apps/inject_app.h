#ifndef INJECT_APP_H
#define INJECT_APP_H

#include <string.h>
#include "app_core.h"
#include "sfw/apps/scen_app.h"

extern app inject_app;

void inject_set_path(const char *path);
const char *inject_get_path(void);

#endif