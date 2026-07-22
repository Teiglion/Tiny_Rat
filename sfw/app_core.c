#include "app_core.h"
#include "pico/stdlib.h"
#include "tusb.h"
#include "hdw/disp_driver.h"
#include "hdw/sd_driver.h"
#include "sfw/apps/launcher_app.h"

static FATFS fs;
static node *win = NULL;
static bool fs_mounted = false;
static app *current_app = NULL;
static app *prev_app = NULL;

FATFS *fs_root(void){return &fs;}
bool fs_root_is_mounted(void){return fs_mounted;}
node **ui_root(void){return &win;}
app *app_current(void){return current_app;}
app *app_prev(void){return prev_app;}

void app_open(app *a)
{
    if (!a) return;
    if (a == current_app) return;
    prev_app = current_app;

    // App switch is explicit: current app exits before the next one enters.
    if (current_app && current_app->exit) current_app->exit(current_app);
    current_app = a;
    if (current_app->enter) current_app->enter(current_app);
}

bool fs_mount_root(void)
{
    if (fs_mounted)return true;
    // Register the root FatFS volume once and keep a local mounted flag.
    fs_mounted = (f_mount(&fs, "", 1) == FR_OK);
    return fs_mounted;
}

void fs_unmount_root(void)
{
    if (!fs_mounted)return;
    f_mount(NULL, "", 1);
    fs_mounted = false;
}

static void pico_init(void)
{
    gpio_init(PIN_LED_RED);
    gpio_set_dir(PIN_LED_RED, GPIO_OUT);

    gpio_init(PIN_LED_GREEN);
    gpio_set_dir(PIN_LED_GREEN, GPIO_OUT);

    gpio_init(PIN_BUT1);
    gpio_set_dir(PIN_BUT1, GPIO_IN);

    gpio_init(PIN_BUT2);
    gpio_set_dir(PIN_BUT2, GPIO_IN);

    gpio_init(PIN_BUT3);
    gpio_set_dir(PIN_BUT3, GPIO_IN);

    gpio_init(PIN_BUT4);
    gpio_set_dir(PIN_BUT4, GPIO_IN);
}

void SystemInit(void)
{
    pico_init();
    sleep_ms(30);

    SDInit();
    TouchInit();
    DisplayInit();

    hard_assert(fs_mount_root());

    tusb_init();

    create_layer(&win, 0, 0, 240, 280, "/sys/TR.res", 0);
    sleep_ms(1500);
    kill_node(&win);

    app_open(&launcher_app);
}

void system_loop(void)
{
    while (true)
    {
        // TinyUSB expects tud_task() to be pumped regularly from the main loop.
        tud_task();
        app_event ev = {0};
        event_handler(&ev);
        
        // Deliver input/event first, then give the active app its regular tick.
        if (current_app && current_app->on_event && ev.type != NONE)current_app->on_event(current_app, &ev);
        if (current_app && current_app->tick)current_app->tick(current_app);
    }
}

