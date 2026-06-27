#include "app_core.h"

static FATFS fs;
static node *win = NULL;
static mode MODE;
static mode LAST_MODE;
static bool inject_started;
static bool inject_done;
static bool busy;

int pico_init(void) {
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
    MODE = 0;
    LAST_MODE = 0;
    DisplayInit();
}

void system_loop(void)
{
    app_event ev={0};
    LAST_MODE = MODE;
    enter_mode(MODE);

    while(true)
    {
        if(LAST_MODE != MODE)
        {
            LAST_MODE=MODE;
            enter_mode(MODE);
        }
        if(MODE == NORMAL)normal_tick(&ev);
        else if(MODE == STORAGE)storage_tick(&ev);
        else if(MODE == INJECT)inject_tick();
    }
}

void enter_mode(mode m)
{
    if(m == NORMAL)normal_mode();
    else if(m == STORAGE)storage_mode();
    else if(m == INJECT)inject_mode();
}

void normal_mode(void)
{
    FRESULT fr;
    fr = f_mount(&fs, "", 1);
    hard_assert(fr == FR_OK);
    create_layer(&win,0,0,240,280,"/sys/TR.res",0);
    sleep_ms(1500);
    kill_node(&win);
    create_layer(&win,0,0,240,280,"/sys/11.res",0);
    add_sublayer(&win->l,40,20,66,66,"/apps/disk_app/disk_app.res",0);
    add_sublayer(&win->l,134,20,66,66,"/apps/scen_app/scen_app.res",0);
    busy = false;
}

void normal_tick(app_event *ev)
{
    event_handler(ev);
    if (ev->type != NONE) fill_rect(ev->x, ev->y, 3, 3, 0xF800);

    if(ev->type == TOUCH && ev->x >= 40 && ev->x <= 106 && ev->y >= 20 && ev->y <= 86 && !busy)
    {
        busy = true;
        create_layer(&win,0,0,240,280,"/apps/disk_app/storage_mode.res",0);
        MODE = STORAGE;
    }

    if(ev->type == TOUCH && ev->x >= 134 && ev->x <= 200 && ev->y >= 20 && ev->y <= 86 && !busy)
    {
        busy = true;
        //create_layer(&win,0,0,240,280,"/apps/scen_app/sc.res",0);
        create_layer(&win,0,0,240,280,0,0xFFFF);
        print_scens("/apps/scen_app/scripts");
    }

    if (ev->type == BUT1)
    {
        create_layer(&win,0,0,240,280,"/apps/inject_app/inj.res",0);
        MODE = INJECT;
    } 
        
    if (ev->type == SWIPE_RIGHT)
    {
        busy = false;
        pop_node(&win);
    }

    if (ev->type == BUT3)
    {
        busy = false;
        collapse_list(&win);
    }

    if(ev->type == BUT4)
    {
        busy = true;
        create_layer(&win,0,0,240,280,"2.res",0);
    }
}

void storage_mode(void)
{   
    f_mount(NULL, "", 1);
    usb = USB_MSC;
    tusb_init();
}

void storage_tick(app_event *ev)
{
    tud_task();
    event_handler(ev);
    if(ev->type == TOUCH && ev->x >= 20 && ev->x <= 220 && ev->y >= 220 && ev->y <= 260)
    {
        watchdog_reboot(0,0,0);
    }
}

void inject_mode(void)
{
    usb = USB_HID;
    inject_started = false;
    inject_done = false;
    tusb_init();
}

void inject_tick(void)
{
    tud_task();
    if (!tud_mounted()) return;
    if (inject_done) return;
    if (!inject_started) 
    {
        inject_started = true;
        hid_send_key(KEYBOARD_MODIFIER_LEFTGUI,HID_KEY_R);
        sleep_ms(500);
        hid_send_text("cmd");
        hid_send_key(0, HID_KEY_ENTER);
        sleep_ms(1500);
        hid_send_text("ssh teiglion@192.168.1.11");
        hid_send_key(0,HID_KEY_ENTER);
        sleep_ms(1500);
        hid_send_text("upgrad3");
        hid_send_key(0,HID_KEY_ENTER);
        inject_done = true;
        watchdog_reboot(0,0,0);
    }
}