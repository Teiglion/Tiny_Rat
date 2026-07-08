#include "invaders_app.h"
#include "pico/time.h"
#include "hdw/disp_driver.h"
#include "sfw/apps/launcher_app.h"
#include "sfw/gfx/gfx.h"
#include "invaders_res.h"

#define SCR_W 240
#define SCR_H 280
#define SCR SCR_H*SCR_W*2
#define ALIEN_X  200
#define ALIEN_Y  70
#define SHUTTLE_X  5

#define INVADER_STEP_MS 400
#define BULLET_STEP_MS   60
#define ANIM_STEP_MS    180

static int SHUTTLE_Y = 143;

static uint32_t last_invader_step = 0;
static uint32_t last_bullet_step = 0;
static uint32_t last_anim_step = 0;

typedef struct
{
    bool is_started;
    node **root;
    int counter;
    bool dir;
    aliens enemy[20];
    int step;
} invaders_ctx;
static invaders_ctx invaders_ctx_data = {0};

static uint32_t now_ms(void){return to_ms_since_boot(get_absolute_time());}

static void build_spr_part(const uint16_t *spr, uint16_t *dest)
{
    if(!spr || !dest)return;
    for(int i=0; i < 10; i++)
    {
        for(int j = 0; j<23;j++)
        {
            dest[i*23+j] = spr[(9-i)*23+j];
        }
    }
}

static void invaders_enter(app *self)
{
    invaders_ctx *ctx = (invaders_ctx*)self->ctx;
    ctx->is_started = false;
    ctx->step = 1;
    SHUTTLE_Y = 143;
    ctx->root = ui_root();
    create_layer(ctx->root,0,0,240,280,"/apps/invaders_app/menu.res",0);
    FIL file;
    FRESULT fr;
    UINT br = 0;
    fr = f_open(&file, "/apps/invaders_app/list.txt", FA_READ);
    if (fr != FR_OK) return;
    char buf[7];
    int row = 0;
    int x = ALIEN_X;
    int y = ALIEN_Y;
    while(f_gets(buf, sizeof(buf), &file))
    {
        for(int i = 0;i < 5; i++)
        {
            ctx->enemy[row+i].on = true;
            ctx->enemy[row+i].type = buf[i] - '0';
            ctx->enemy[row+i].pos_x = x;
            ctx->enemy[row+i].pos_y = y;
            y+=30;
        }
        row+=5;
        x-=30;
        y=ALIEN_Y;
    }
    f_close(&file);

}

static void invaders_on_event(app *self, const app_event *ev)
{
    invaders_ctx *ctx = (invaders_ctx*)self->ctx;
    if(ctx->is_started == false && ev->type == TOUCH && ev->x >= 30 && ev->x <= 100 && ev->y >= 60 && ev->y <= 220)
    {   ctx->is_started = true;
        ctx->dir = true;
        ctx->counter = 2;
        uint32_t now = (now_ms());
        last_invader_step = now;
        last_bullet_step  = now;
        last_anim_step    = now;
        return;
    }

    if(ctx->is_started && ev->type == BUT4)
    {
        if((SHUTTLE_Y - 14)>10)SHUTTLE_Y-=14;
        return;
    }

    if(ctx->is_started && ev->type == BUT3)
    {
        if((SHUTTLE_Y + 14) < 260)SHUTTLE_Y+=14;
        return;
    }

    if(ev->type == SWIPE_RIGHT)
    {
        ctx->is_started = false;
        app_open(&launcher_app);
        return;
    }
}

static void invaders_tick(app *self)
{
    invaders_ctx *ctx = (invaders_ctx*)self->ctx;
    if(!ctx->is_started)return;

    load_img("/apps/invaders_app/back.res", &(*ctx->root)->l);
    uint32_t now = now_ms();
    uint16_t sprite_part[230] = {0};
        
    if (now - last_invader_step >= INVADER_STEP_MS)
    {
        if(ctx->step % 2)
        {
            for(int i=0;i<20;i++)
            ctx->enemy[i].type++;
        }
        else 
        {
            for(int i=0;i<20;i++)
            ctx->enemy[i].type--;
        }
        last_invader_step = now;
        ctx->step++;
        if(ctx->dir && ctx->counter != 5)
        {
            for(int i=0;i<20;i++)
            ctx->enemy[i].pos_y +=30;
            ctx->counter++;
        }
        else if(!ctx->dir && ctx->counter != 5)
        {
            for(int i=0;i<20;i++)
            ctx->enemy[i].pos_y -=30;
            ctx->counter++;
        }
        if(!ctx->dir && ctx->counter == 5)
        {
            for(int i=0;i<20;i++)
            {
                ctx->enemy[i].pos_x -=30;
                ctx->enemy[i].pos_y +=30;
            }
            ctx->counter=0;
            ctx->dir = true;
        }
        else if(ctx->dir && ctx->counter == 5)
        {
            for(int i=0;i<20;i++)
            {
               ctx->enemy[i].pos_x -=30;
               ctx->enemy[i].pos_y -=30; 
            }
            ctx->counter=0;
            ctx->dir = false;
        }
    }
        
    for(int i=0;i<20;i++)
    {
        if(ctx->enemy[i].on)
        {
            const uint16_t *sprite = en[ctx->enemy[i].type];
            build_spr_part(sprite,sprite_part);
            buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,ctx->enemy[i].pos_x,ctx->enemy[i].pos_y,sprite,23,10,5);
            buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,ctx->enemy[i].pos_x,ctx->enemy[i].pos_y+10,sprite_part,23,10,5);
        }
    }
    build_spr_part(en[6],sprite_part);
    buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,SHUTTLE_X,SHUTTLE_Y,en[6],23,10,5);
    buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,SHUTTLE_X,SHUTTLE_Y+10,sprite_part,23,10,5);
    draw_buf(0,0,SCR_W,SCR_H);
    for(int i=0;i<20;i++)
    {
        if(ctx->enemy[i].on && ctx->enemy[i].pos_x <= SHUTTLE_X+23)
        {
            invaders_enter(self);
            return;
        }
    }
    return;
}

static void invaders_exit(app *self)
{
    invaders_ctx *ctx = (invaders_ctx*)self->ctx;

    ctx->is_started = false;
    ctx->counter = 0;
    ctx->step = 0;
    ctx->dir = false;

    if (ctx->root && *ctx->root)
        kill_node(ctx->root);
}

app invaders_app = {
    .name = "invaders",
    .enter = invaders_enter,
    .on_event = invaders_on_event,
    .tick = invaders_tick,
    .exit = invaders_exit,
    .ctx = &invaders_ctx_data
};

