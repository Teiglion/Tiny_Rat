#include "layers.h"

extern uint8_t framebuf[];

void load_img(const char *name, layer *l)
{
    if (!name || !l) return;
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

    // Short reads are padded with zeros so partial resources do not leave junk in framebuf.
    if (br < need) {
        for (uint32_t i = br; i < need; i++) framebuf[i] = 0x00;
    }
}

void load_buf( layer *l)
{
    if (!l) return;
    draw_buf(l->x, l->y, l->w, l->h);
}

void push_node(node **list, uint16_t in_x, uint16_t in_y, uint16_t in_w, uint16_t in_h, const char *in_res_name, uint16_t in_color)
{

    // Layer list works like a stack: new node is always pushed to the front.
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
    (*list)->l.res_name=NULL;
    (*list)->l.sublayers=NULL;
    if(!in_res_name)(*list)->l.color=in_color;
    else 
    {
        size_t len = strlen(in_res_name)+1;
        char *t = malloc(len);
        if(!t)
        {   node *temp=(*list)->prev;
            free((*list));
            *list=temp;
            return;
        }
        // Resource name is copied so caller does not need to keep its string alive.
        memcpy(t,in_res_name,len);
        *(t+(len-1))='\0';
        (*list)->l.res_name=t;
    }
}

void free_sub(node **list)
{
    if(!list || !*list)return;
    while(*list)
    {
        node *temp=(*list)->prev;
        if((*list)->l.res_name)free((*list)->l.res_name);
        free(*list);
        *list=temp;
    }
}

static void build_with_res(node *list)
{
    if(!list)return;

    // Redraw the current top layer first.
    if(list->l.res_name)
    {
        load_img(list->l.res_name,&list->l);
        load_buf(&list->l);
    }
    else
    {
        fill_rect(list->l.x,list->l.y,list->l.w,list->l.h,list->l.color);
    }

    // Then draw its sublayers on top in stack order.
    node *temp=list->l.sublayers;
    while(temp)
    {
        if(temp->l.res_name)
        {
            load_img(temp->l.res_name,&temp->l);
            load_buf(&temp->l);
        }
        else
        {
            fill_rect(temp->l.x,temp->l.y,temp->l.w,temp->l.h,temp->l.color);
        }
        temp=temp->prev;
    }
}

void pop_node(node **list)
{
    if(!list || !*list)return;
    if(!(*list)->prev)return;
    node *temp=(*list)->prev;
    if((*list)->l.res_name)free((*list)->l.res_name);
    if((*list)->l.sublayers)free_sub(&(*list)->l.sublayers);
    free(*list);
    *list=temp;

    // After removing the top layer, redraw the one that becomes visible again.
    build_with_res(*list);  
}

void kill_node(node **list)
{
    if(!list || !*list)return;
    node *temp=(*list)->prev;
    if((*list)->l.res_name)free((*list)->l.res_name);
    if((*list)->l.sublayers)free_sub(&(*list)->l.sublayers);
    free(*list);
    *list=temp; 
}

void collapse_list(node **list)
{
    if(!list || !*list)return;

    // Drop everything above the bottom-most layer and rebuild the final visible state.
    while ((*list)->prev)
    {
        node *temp=(*list)->prev;
        if((*list)->l.res_name)free((*list)->l.res_name);
        if((*list)->l.sublayers)free_sub(&(*list)->l.sublayers);
        free(*list);
        *list=temp; 
    }
    build_with_res(*list);
}

void add_sublayer(layer *list, uint16_t in_x, uint16_t in_y, uint16_t in_w, uint16_t in_h, const char *in_res_name, uint16_t in_color)
{
    if(!list)return;
    push_node(&(list->sublayers), in_x, in_y, in_w, in_h, in_res_name, in_color);
    build_with_res(&(*list->sublayers));
}

void create_layer(node **list, uint16_t in_x, uint16_t in_y, uint16_t in_w, uint16_t in_h,const char *res_name, uint16_t color)
{
    push_node(list, in_x,in_y,in_w,in_h,res_name,color);
    build_with_res(*list);
}