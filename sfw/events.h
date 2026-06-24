#ifndef EVENTS_H
#define EVENTS_H

#include <stdint.h>
#include "pico/stdlib.h"
#include "hdw/touch_driver.h"

#define PIN_LED_RED 10
#define PIN_LED_GREEN 11
#define PIN_BUT1 6
#define PIN_BUT2 7
#define PIN_BUT3 8
#define PIN_BUT4 9

typedef enum
{
    NONE,
    TOUCH,
    SWIPE_UP,
    SWIPE_DOWN,
    SWIPE_RIGHT,
    SWIPE_LEFT,
    BUT1,
    BUT2,
    BUT3,
    BUT4
}event_type;

typedef struct {
    event_type type;
    uint16_t x;
    uint16_t y;
} app_event;

void event_handler(app_event *ev);
#endif