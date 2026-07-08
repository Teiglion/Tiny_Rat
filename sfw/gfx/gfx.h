#ifndef GFX_H
#define GFX_H

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

bool load_img_to_buf(const char *name, uint8_t *buf, size_t need);
void buf_fill_rect(uint8_t *buf, uint16_t buf_w, uint16_t buf_h, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void buf_draw_sprite_rgb565(uint8_t *buf, uint16_t buf_w, uint16_t buf_h, uint16_t x, uint16_t y, const uint16_t *sprite, uint16_t spr_w, uint16_t spr_h, uint16_t transparent);

#endif