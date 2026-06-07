#include "layers.h"

void load_img(const char *name, layer *l)
{
    FILE *f=fopen(name,"rb");
    if(!f)return;
    stream_img(f, l->x, l->y, l->w, l->h);
    fclose(f);
}