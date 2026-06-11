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


void push_node(node **list, uint16_t in_x, uint16_t in_y, uint16_t in_w, uint16_t in_h, const char *in_res_name, uint16_t in_color)
{

    if(*list==NULL)
    {
        *list= malloc(sizeof(node));
        if(!(*list))return;
        (*list)->prev=NULL;
    }
    else
    {
        node *temp=malloc(sizeof(node));
        if(!temp)return;
        temp->prev=*list;
        *list=temp;
    }
    (*list)->l.x=in_x;
    (*list)->l.y=in_y;
    (*list)->l.w=in_w;
    (*list)->l.h=in_h;
    if(!in_res_name)(*list)->l.color=in_color;
        else 
        {
            size_t len = strlen(in_res_name)+1;
            char *temp = malloc(len);
            if(!temp)
            {   
                free((*list));
                return;
            }
            memcpy(temp,in_res_name,len);
            *(temp+(len-1))='\0';
            (*list)->l.res_name=temp;
        }
}

void pop_node(node **list)
{
    if(!(*list)->prev)return;
    else
    {
        node *temp=(*list)->prev;
        if((*list)->l.res_name)free((*list)->l.res_name);
        free(*list);
        *list=temp;
        build_with_res(*list);
    }
}

void build_with_res(node *list)
{
    if(list->l.res_name)
    {
        load_img(list->l.res_name,&list->l);
        load_buf(&list->l);
    }
    else
    {
        fill_rect(list->l.x,list->l.y,list->l.w,list->l.h,list->l.color);
    }
    
}

void create_layer(node **list, uint16_t in_x, uint16_t in_y, uint16_t in_w, uint16_t in_h,char *res_name, uint16_t color)
{
    push_node(list, in_x,in_y,in_w,in_h,res_name,color);
    build_with_res(*list);

}