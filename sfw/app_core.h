#ifndef APP_CORE_H
#define APP_CORE_H

#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "hardware/watchdog.h"
#include "tusb.h"
#include "hdw/disp_driver.h"
#include "hdw/sd_driver.h"
#include "hdw/FatFS/ff.h"
#include "sfw/events.h"
#include "sfw/usb_mode.h"
#include "sfw/hid_inject.h"
#include "sfw/layers.h"

typedef enum
{
    UI,
    STORAGE,
    INJECT
}mode;

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

int pico_init(void);
void SystemInit(void);
void system_loop(void);
node **ui_root(void);
mode get_mode(void);
void set_mode(mode M);
app *app_current(void);
void app_open(app *a);
void enter_mode(mode M);
void leave_mode(mode M);

#endif