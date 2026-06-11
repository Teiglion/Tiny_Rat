#ifndef DISP_DRIVER_H
#define DISP_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include <stdio.h>
#include "hardware/spi.h"
#include "hardware/i2c.h"
#include "pico/stdlib.h"
#include "hdw/FatFS/ff.h"

#define PIN_LCD_SCK      14
#define PIN_LCD_MOSI     15
#define PIN_LCD_CS       13
#define PIN_LCD_DC       12
#define PIN_LCD_RST      28
#define PIN_LCD_BK       29

#define TOUCH_I2C_PORT     i2c0
#define TOUCH_ADDR         0x15
#define PIN_TOUCH_SDA      24
#define PIN_TOUCH_SCL      25
#define PIN_TOUCH_INT      26
#define PIN_TOUCH_RST      27

#define LCD_SPI_PORT     spi1
#define LCD_WIDTH        240
#define LCD_HEIGHT       280

#define LCD_X_OFFSET 0
#define LCD_Y_OFFSET 20

#define FRAMEBUF_SIZE (LCD_WIDTH * LCD_HEIGHT * 2)

extern uint8_t framebuf[];

void lcd_reset(void);
void lcd_write_cmd(uint8_t cmd);
void lcd_write_data(uint8_t *data, size_t len);
void lcd_write_cmd_data(uint8_t cmd, uint8_t *data, size_t len);
void DisplayInit(void);
void set_window(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1);
void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void stream_img(FIL *f,uint16_t x, uint16_t y, uint16_t w, uint16_t h);
void draw_buf(uint16_t x, uint16_t y, uint16_t w, uint16_t h);

void TouchInit(void);
void TouchTestToFile(void);
bool TouchRead(uint16_t *x, uint16_t *y);

#endif