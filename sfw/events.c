#include "events.h"

void event_handler(app_event *ev)
{
    uint8_t data[6];
    if(!TouchRead(data, &(ev->x), &(ev->y)) || (data[1] == 0 && data[0] == 0x00))ev->type=NONE;
    else
    {
        if(data[0] == 0x01)ev->type=SWIPE_UP;
        if(data[0] == 0x02)ev->type=SWIPE_DOWN;
        if(data[0] == 0x03)ev->type=SWIPE_LEFT;
        if(data[0] == 0x04)ev->type=SWIPE_RIGHT;
        if(data[0] == 0x05)ev->type=TOUCH;
    }

    if(gpio_get(PIN_BUT1) == 0)ev->type = BUT1;
    if(gpio_get(PIN_BUT2) == 0)ev->type = BUT2;
    if(gpio_get(PIN_BUT3) == 0)ev->type = BUT3;
    if(gpio_get(PIN_BUT4) == 0)ev->type = BUT4;
}