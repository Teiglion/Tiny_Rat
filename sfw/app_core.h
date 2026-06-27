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
#include "sfw/scenaries.h"

typedef enum
{
    NORMAL,
    STORAGE,
    INJECT
}mode;

void SystemInit(void);
void system_loop(void);
void normal_mode(void);
void normal_tick(app_event *ev);
void storage_mode(void);
void storage_tick(app_event *ev);
void inject_mode(void);
void inject_tick(void);
void enter_mode(mode m);
#endif