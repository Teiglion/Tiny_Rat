#ifndef APP_CORE_H
#define APP_CORE_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "tusb.h"
#include "hdw/disp_driver.h"
#include "hdw/sd_driver.h"
#include "hdw/FatFS/ff.h"
#include "sfw/events.h"
#include "sfw/usb_mode.h"
#include "sfw/hid_inject.h"
#include "sfw/layers.h"

typedef struct app app;
typedef void (*enter_fn)(app *self);
typedef void (*event_fn)(app *self, const app_event *ev);
typedef void (*tick_fn)(app *self);
typedef void (*exit_fn)(app *self);

struct app
{
    const char *name;
    enter_fn enter;
    event_fn on_event;
    tick_fn tick;
    exit_fn exit;
    void *ctx;
};

void SystemInit(void);
void system_loop(void);
node **ui_root(void);
app *app_current(void);
void app_open(app *a);
void storage_begin(void);
void storage_end_request(void);
bool storage_is_active(void);

#endif