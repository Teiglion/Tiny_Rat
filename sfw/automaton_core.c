#include <stdlib.h>
#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "tusb.h"
#include "hdw/disp_driver.h"
#include "hdw/touch_driver.h"
#include "hdw/sd_driver.h"
#include "hdw/FatFS/ff.h"
#include "layers.h"

#define PIN_LED_RED 10
#define PIN_LED_GREEN 11
#define PIN_BUT1 6
#define PIN_BUT2 7
#define PIN_BUT3 8
#define PIN_BUT4 9

int pico_led_init(void) {
    gpio_init(PIN_LED_RED);
    gpio_set_dir(PIN_LED_RED, GPIO_OUT);

    gpio_init(PIN_LED_GREEN);
    gpio_set_dir(PIN_LED_GREEN, GPIO_OUT);

    return PICO_OK;
}

int pico_button_init(void) {
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

static void set_mode_leds(bool usb_disk_mode)
{
    gpio_put(PIN_LED_RED, usb_disk_mode ? 1 : 0);
    gpio_put(PIN_LED_GREEN, usb_disk_mode ? 0 : 1);
}


int main() {
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);

    pico_button_init();
    sleep_ms(30);

    bool usb_disk_mode = (gpio_get(PIN_BUT1) == 0);
    set_mode_leds(usb_disk_mode);

    SDInit();
    if (usb_disk_mode)
    {
        sleep_ms(100);
        tusb_init();

        while (true) {
            tud_task();
        }
    }
    else {
        
        DisplayInit();
        TouchInit();

        FATFS fs;
        FRESULT fr;
        fr = f_mount(&fs, "", 1);
        hard_assert(fr == FR_OK);

        node *win=NULL;
        create_layer(&win,0,0,240,280,"11.res",0);

        uint16_t x, y;
        while (true) 
        {
            sleep_ms(1);
            if (TouchRead(&x, &y)) fill_rect(x, y, 3, 3, 0xF800);
            if(gpio_get(PIN_BUT2) == 0)pop_node(&win);
            if(gpio_get(PIN_BUT3) == 0)collapse_list(&win);
            if(gpio_get(PIN_BUT4) == 0)
            {
                create_layer(&win,0,0,240,280,"2.res",0);
                add_sublayer(&win->l,100,100,50,50,"1.res",0);
            }
            
        }
    }
}