#include "app_core.h"
#include "sfw/apps/launcher_app.h"
#include "sfw/apps/scen_app.h"
#include "sfw/apps/disk_app.h"
#include "sfw/apps/inject_app.h"

static FATFS fs;
static node *win = NULL;
static bool fs_mounted = false;
static bool usb_ready = false;
static mode MODE = UI;
static app *current_app = NULL;
static bool storage_exit_pending = false;

node **ui_root(void){return &win;}
mode get_mode(void){return MODE;}
app *app_current(void){return current_app;}

void app_open(app *a)
{
    if (!a) return;

    if (current_app && current_app->exit) current_app->exit(current_app);

    current_app = a;

    if (current_app->enter) current_app->enter(current_app);
}

void set_mode(mode M)
{
    if (M == MODE) return;
    leave_mode(MODE);
    MODE = M;
    enter_mode(MODE);
}

void enter_mode(mode M)
{
    switch (M)
    {
        case UI:
            if (usb_msc_state() == MSC_HIDDEN && !fs_mounted)
            {
                FRESULT fr = f_mount(&fs, "", 1);
                hard_assert(fr == FR_OK);
                fs_mounted = true;
            }

            app_open(&launcher_app);
            break;

        case STORAGE:
            if (fs_mounted)
            {
               f_mount(NULL, "", 1);
               fs_mounted = false;
            }
            
            storage_exit_pending = false;
            usb_msc_show();
            break;

        case INJECT:
            if (usb_msc_state() == MSC_HIDDEN && !fs_mounted)
            {
                FRESULT fr = f_mount(&fs, "", 1);
                hard_assert(fr == FR_OK);
                fs_mounted = true;
            }

            inject_start();
            break;
    }
}

void leave_mode(mode M)
{
    switch (M)
    {
        case UI: if (current_app && current_app->exit) current_app->exit(current_app);break;

        case STORAGE: break;

        case INJECT: inject_stop();break;
    }
}

int pico_init(void)
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

    return PICO_OK;
}

void SystemInit(void)
{
    int rc = pico_init();
    hard_assert(rc == PICO_OK);
    sleep_ms(30);

    SDInit();
    TouchInit();
    DisplayInit();

    FRESULT fr = f_mount(&fs, "", 1);
    hard_assert(fr == FR_OK);
    fs_mounted = true;

    tusb_init();
    usb_ready = true;

    create_layer(&win, 0, 0, 240, 280, "/sys/TR.res", 0);
    sleep_ms(1500);
    kill_node(&win);

    MODE = UI;
    current_app = NULL;
    enter_mode(UI);
}

static void ui_loop(const app_event *ev)
{
    if (current_app && current_app->on_event && ev->type != NONE)current_app->on_event(current_app, ev);

    if (current_app && current_app->tick)current_app->tick(current_app);
}

static void storage_loop(const app_event *ev)
{
    if (!storage_exit_pending && ev->type == TOUCH && ev->x >= 20 && ev->x <= 220 && ev->y >= 220 && ev->y <= 260)
    {
        usb_msc_hide();
        storage_exit_pending = true;
        return;
    }
    if (storage_exit_pending && usb_msc_state() == MSC_HIDDEN)
    {
        storage_exit_pending = false;
        MODE = UI;
        enter_mode(UI);
    }
}

static void inject_loop(const app_event *ev)
{
    (void)ev;
    inject_tick();
}

void system_loop(void)
{
    while (true)
    {
        if (usb_ready)tud_task();
        usb_msc_poll();

        if ((MODE == UI || MODE == INJECT) && usb_msc_state() == MSC_HIDDEN && !fs_mounted)
        {
            FRESULT fr = f_mount(&fs, "", 1);
            hard_assert(fr == FR_OK);
            fs_mounted = true;
        }

        app_event ev = {0};
        event_handler(&ev);

        switch (MODE)
        {
            case UI: ui_loop(&ev);break;

            case STORAGE: storage_loop(&ev);break;

            case INJECT: inject_loop(&ev);break;
        }
    }
}