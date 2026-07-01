#ifndef INJECT_APP_H
#define INJECT_APP_H

#include <string.h>
#include "app_core.h"

void inject_set_path(const char *path);
const char *inject_get_path(void);

void inject_start(void);
void inject_stop(void);
void inject_tick(void);

#endif