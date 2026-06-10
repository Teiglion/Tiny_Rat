#include "layers.h"

extern uint8_t framebuf[];

void load_img(const char *name, layer *l)
{
    FIL file;
    FRESULT fr;
    UINT br = 0;
    uint32_t need = (uint32_t)l->w * l->h * 2;
    if (need > FRAMEBUF_SIZE) return;
    fr = f_open(&file, name, FA_READ);
    if (fr != FR_OK) return;
    fr = f_read(&file, framebuf, need, &br);
    f_close(&file);
    if (fr != FR_OK) return;
    if (br < need) {
        for (uint32_t i = br; i < need; i++) framebuf[i] = 0x00;
    }
}

void load_buf( layer *l)
{
   draw_buf(l->x, l->y, l->w, l->h);
}