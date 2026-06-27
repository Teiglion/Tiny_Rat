#ifndef TEXT_H
#define TEXT_H

#include <stdint.h>
#include "hdw/disp_driver.h"

void draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale);
void draw_text(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t scale);
void draw_text_cutted(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t scale);
uint16_t text_width(const char *text, uint8_t scale);
uint16_t text_height(uint8_t scale);

#endif