#ifndef TOUCH_DRIVER_H
#define TOUCH_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "hardware/i2c.h"
#include "pico/stdlib.h"

#define TOUCH_I2C_PORT     i2c0
#define TOUCH_ADDR         0x15
#define PIN_TOUCH_SDA      24
#define PIN_TOUCH_SCL      25
#define PIN_TOUCH_INT      26
#define PIN_TOUCH_RST      27

#define LCD_WIDTH    240
#define LCD_HEIGHT   280
#define TOUCH_X_MIN  10
#define TOUCH_X_MAX  229
#define TOUCH_Y_MIN  30
#define TOUCH_Y_MAX  259

void TouchInit(void);
bool TouchRead(uint8_t *data, uint16_t *x, uint16_t *y);

#endif