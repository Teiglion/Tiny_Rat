#ifndef LAYERS_H
#define LAYERS_H

#include <stdlib.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "pico/stdlib.h"
#include "hdw/disp_driver.h"
#include "hdw/FatFS/ff.h"

typedef struct
{
    uint16_t x,y,w,h;
    char *res_name;
    uint16_t color;
}layer;

typedef struct node
{
    layer l;
    struct node *prev;
}node;

void load_img(const char *name, layer *l);
void load_buf(layer *l);

void create_layer(node **list, uint16_t in_x, uint16_t in_y, uint16_t in_w, uint16_t in_h,char *res_name, uint16_t color);
void push_node(node **list, uint16_t in_x, uint16_t in_y, uint16_t in_w, uint16_t in_h, const char *in_res_name, uint16_t in_color);
void pop_node(node **list);
void build_with_res(node *list);

#endif