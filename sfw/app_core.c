#include "app_core.h"

static FATFS fs;
static node *win = NULL;
static mode MODE;
static mode LAST_MODE;

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
    DisplayInit();
    TouchInit();
    MODE = 0;
    LAST_MODE = 0;
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
    }
}

void enter_mode(mode m)
{
    if(m == NORMAL)normal_mode();
    else if(m == STORAGE)storage_mode();
}

void normal_mode(void)
{
    FRESULT fr;
    fr = f_mount(&fs, "", 1);
    hard_assert(fr == FR_OK);
    create_layer(&win,0,0,240,280,"11.res",0);
    add_sublayer(&win->l,40,20,66,66,"/apps/disk_app/disk_app.res",0);
     
}

void normal_tick(app_event *ev)
{
    event_handler(ev);
    if (ev->type != NONE) fill_rect(ev->x, ev->y, 3, 3, 0xF800);

    if(ev->type == TOUCH && ev->x >= 40 && ev->x <= 106 && ev->y >= 20 && ev->y <= 86)
    {
        fill_rect(50, 150, 80, 80, 0x8C71);
        MODE=1;
    }

    if (ev->type == BUT1)
    {

    } 
        
    if (ev->type == SWIPE_RIGHT)pop_node(&win);

    if (ev->type == BUT3)
    {
        collapse_list(&win);
    }

    if(ev->type == BUT4)
    {
        create_layer(&win,0,0,240,280,"2.res",0);
        add_sublayer(&win->l,100,100,50,50,"1.res",0);
    }
}

void storage_mode(void)
{   
    f_mount(NULL, "", 1);
    tusb_init();
}

void storage_tick(app_event *ev)
{
    tud_task();
    event_handler(ev);
    if(ev->type == TOUCH && ev->x >= 50 && ev->x <= 130 && ev->y >= 150 && ev->y <= 230)
    {
        watchdog_reboot(0,0,0);
    }
}