#ifndef APP_CORE_H
#define APP_CORE_H

#include "hdw/FatFS/ff.h"
#include "sfw/events.h"
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

FATFS *fs_root(void);
node **ui_root(void);

app *app_current(void);
app *app_prev(void);
void app_open(app *a);

bool fs_mount_root(void);
void fs_unmount_root(void);
bool fs_root_is_mounted(void);

#endif