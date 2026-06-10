#ifndef LAYERS_H
#define LAYERS_H

#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include "hdw/disp_driver.h"
#include "hdw/FatFS/ff.h"
typedef struct
{
    uint16_t x,y,w,h;
}layer;

void load_img(const char *name, layer *l);
void load_buf(layer *l);

#endif