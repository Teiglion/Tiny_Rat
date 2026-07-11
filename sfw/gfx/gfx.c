#include "gfx.h"
#include "hdw/FatFS/ff.h"

bool load_img_to_buf(const char *name, uint8_t *buf, size_t need)
{
    if(!name || !buf || !need)return false;
    FIL file;
    FRESULT fr;
    UINT br = 0;
    fr = f_open(&file, name, FA_READ);
    if (fr != FR_OK) return false;
    fr = f_read(&file, buf, need, &br);
    f_close(&file);
    if (fr != FR_OK) return false;
    if (br < need) {
        for (uint32_t i = br; i < need; i++) buf[i] = 0x00;
    }
    return true;
}

void buf_fill_rect(uint8_t *buf, uint16_t buf_w, uint16_t buf_h, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if(buf && x < buf_w && y < buf_h && w && h)
    {
        uint16_t *b = (uint16_t *)buf;
        if(x+w > buf_w) w = buf_w - x;
        if(y+h > buf_h) h = buf_h - y;
        for(uint16_t i = y; i < y+h; i++)
        {
            for(uint16_t j = x; j < x+w; j++)
            {
                uint16_t index = i * buf_w + j;
                b[index] = color;
            }
        }
    }
}

void buf_draw_sprite_rgb565(uint8_t *buf, uint16_t buf_w, uint16_t buf_h, uint16_t x, uint16_t y, const uint16_t *sprite, uint16_t spr_w, uint16_t spr_h, uint16_t transparent)
{
    if(buf && sprite && x < buf_w && y < buf_h && spr_w && spr_h)
    {
        uint16_t *b = (uint16_t *)buf;
        uint16_t dr_w=spr_w,dr_h=spr_h;
        if(x+spr_w > buf_w) dr_w = buf_w - x;
        if(y+spr_h > buf_h) dr_h = buf_h - y;
        
        for(uint16_t i = y; i < y+dr_h; i++)
        {
            for(uint16_t j = x; j < x+dr_w; j++)
            {
                uint16_t index = i * buf_w + j;
                uint16_t src_index = (i-y) * spr_w + (j-x);
                if(sprite[src_index] != transparent)b[index] = sprite[src_index];
            }
        }
    }
}
