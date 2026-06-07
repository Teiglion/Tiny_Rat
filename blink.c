#include "pico/stdlib.h"
#include "hardware/spi.h"
#include "tusb.h"
#include "hdw/disp_driver.h"
#include "hdw/sd_driver.h"
#include "layers.h"

#define PIN_LED_RED      10
#define PIN_LED_GREEN    11
#define PIN_BUT1          6
#define PIN_BUT2          7  
#define PIN_BUT3          8
#define PIN_BUT4          9

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

int main() {
    int rc = pico_led_init();
    hard_assert(rc == PICO_OK);
    gpio_put(PIN_LED_GREEN,test());
    sleep_ms(50);
    gpio_put(PIN_LED_RED,!test());
    sleep_ms(50);
    layer l={0,0,240,280};
    tusb_init();
    DisplayInit();
    //load_img("1.res",&l);

    while (true) {
        tud_task();
        //gpio_put(PIN_LED_RED, tud_mounted());
        //gpio_put(PIN_LED_GREEN, !tud_mounted());
        sleep_ms(10);
    }
}