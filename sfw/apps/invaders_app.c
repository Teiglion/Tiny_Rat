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

#define INVADER_STEP_MS 800
#define BULLET_STEP_MS   20
#define SHOT_STEP_MS   800
#define ALIEN_SHOT_MS   800

static uint16_t SHUTTLE_Y = 143;

static uint32_t last_invader_step;
static uint32_t last_bullet_step;
static uint32_t last_shot_step;
static uint32_t last_alien_shot;

typedef struct 
{
    bool dir;
    bool on;
    bool new;
    uint16_t pos_x;
    uint16_t pos_y;
}bullet;

typedef struct
{
    bool is_started;
    bool dir;
    bool gameover;
    bool win;
    node **root;
    int counter;
    aliens enemy[20];
    bullet bullets[20];
    int step;
} invaders_ctx;
static invaders_ctx invaders_ctx_data = {0};

static uint32_t now_ms(void){return to_ms_since_boot(get_absolute_time());}

static void build_spr_part(const uint16_t *spr, uint16_t *dest)
{
    // Build mirrored lower half from the upper 23x10 sprite slice.
    if(!spr || !dest)return;
    for(int i=0; i < 10; i++)
    {
        for(int j = 0; j<23;j++)
        {
            dest[i*23+j] = spr[(9-i)*23+j];
        }
    }
}

static void spawn_bullet(invaders_ctx *ctx, uint16_t x, uint16_t y, bool dir)
{
    if (!ctx) return;

    for (int i = 0; i < 20; i++)
    {
        if (!ctx->bullets[i].on)
        {
            ctx->bullets[i].on = true;
            ctx->bullets[i].pos_x = x;
            ctx->bullets[i].pos_y = y;
            ctx->bullets[i].dir = dir;
            ctx->bullets[i].new = true;
            return;
        }
    }
}

static void clear_bullet(bullet *b)
{
    if (!b) return;
    b->on = false;
    b->new = false;
    b->dir = false;
    b->pos_x = 0;
    b->pos_y = 0;
}

static int random_shooter(invaders_ctx *ctx, int exclude)
{
    if (!ctx) return -1;

    int candidates[20];
    int count = 0;

    for (int i = 0; i < 20; i++)
    {
        if (!ctx->enemy[i].on) continue;
        if (i == exclude) continue;

        bool front = true;

        for (int j = 0; j < 20; j++)
        {
            if (j == i) continue;
            if (!ctx->enemy[j].on) continue;

            // Only the front alien in each lane is allowed to shoot.
            if (ctx->enemy[j].pos_y == ctx->enemy[i].pos_y && ctx->enemy[j].pos_x < ctx->enemy[i].pos_x)
            {
                front = false;
                break;
            }
        }

        if (front)
        {
            candidates[count++] = i;
        }
    }

    if (count == 0) return -1;

    return candidates[rand() % count];
}

static void invaders_enter(app *self)
{
    invaders_ctx *ctx = (invaders_ctx*)self->ctx;

    SHUTTLE_Y = 143;
    last_invader_step = 0;
    last_bullet_step = 0;
    last_shot_step = 0;
    last_alien_shot = 0;
    ctx->is_started = false;
    ctx->step = 1;
    ctx->gameover = false;
    ctx->win = false;
    ctx->counter = 0;
    for(int i = 0; i < 20; i++)
        clear_bullet(&ctx->bullets[i]);

    ctx->root = ui_root();
    create_layer(ctx->root,0,0,240,280,"/apps/invaders_app/menu.res",0);

    FIL file;
    FRESULT fr;
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
    uint32_t now = (now_ms());
    if(ctx->is_started == false && ev->type == TOUCH && ev->x >= 30 && ev->x <= 100 && ev->y >= 60 && ev->y <= 220)
    {   ctx->is_started = true;
        ctx->dir = true;
        ctx->counter = 2;
        last_invader_step = now;
        last_bullet_step  = now;
        last_shot_step    = now;
        last_alien_shot   = now;
        return;
    }

    if(ctx->win && (ev->type == BUT1 || ev->type == BUT2 || ev->type == BUT3 || ev->type == BUT4))
    {
        ctx->win = false;
        invaders_enter(self);
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
    uint32_t now = now_ms();
    if(ctx->win)
    {
        load_img("/apps/invaders_app/win.res", &(*ctx->root)->l);
        draw_buf(0,0,SCR_W,SCR_H);
        return;
    }

    load_img("/apps/invaders_app/back.res", &(*ctx->root)->l);
    
    uint16_t sprite_part[230] = {0};
    uint8_t aliens_left=0;

    // Invaders move in a zig-zag pattern:
    // several vertical lane steps, then one horizontal shift toward the shuttle.
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
        if(ctx->counter == 5)
        {
            for(int i=0;i<20;i++)
            {
                ctx->enemy[i].pos_x -=15;
                if(!ctx->dir)ctx->enemy[i].pos_y +=30;
                else ctx->enemy[i].pos_y -=30;
            }
            ctx->counter=0;
            ctx->dir = !ctx->dir;
        }
    }
        
    for(int i=0;i<20;i++)
    {
        if(ctx->enemy[i].on)
        {
            aliens_left++;
            const uint16_t *sprite = en[ctx->enemy[i].type];
            build_spr_part(sprite,sprite_part);
            buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,ctx->enemy[i].pos_x,ctx->enemy[i].pos_y,sprite,23,10,5);
            buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,ctx->enemy[i].pos_x,ctx->enemy[i].pos_y+10,sprite_part,23,10,5);
        }
    }

    if(aliens_left == 0)
    {
        ctx->win = true;
        return;
    }

    // Player fire is automatic while the round is running.
    if(now - last_shot_step >= SHOT_STEP_MS)
    {
        last_shot_step = now;
        spawn_bullet(ctx, SHUTTLE_X + 24, SHUTTLE_Y + 7, true);
    }

    // Spawn up to two enemy bullets from random front-line aliens.
    if(now - last_alien_shot >= ALIEN_SHOT_MS * 4)
    {
        last_alien_shot = now;

        int a = random_shooter(ctx, -1);
        if (a >= 0)spawn_bullet(ctx, ctx->enemy[a].pos_x - 1, ctx->enemy[a].pos_y + 7, false);
        int b = random_shooter(ctx, a);
        if (b >= 0)spawn_bullet(ctx, ctx->enemy[b].pos_x - 1, ctx->enemy[b].pos_y + 7, false);
    }

    for(int i=0;i<20; i++)
    {
        for(int j=0;j<20;j++)
        {
            // Simple bounding-box checks for bullet vs alien and bullet vs shuttle.
            if(ctx->enemy[j].on && ctx->bullets[i].on && ctx->bullets[i].dir && ctx->bullets[i].pos_x+6 >= ctx->enemy[j].pos_x && ctx->bullets[i].pos_x+6 <= ctx->enemy[j].pos_x+23 && ctx->bullets[i].pos_y+6 >= ctx->enemy[j].pos_y && ctx->bullets[i].pos_y <= ctx->enemy[j].pos_y+20)
            {
                ctx->enemy[j].on = false;
                clear_bullet(&ctx->bullets[i]);
                break;
            }
            if(ctx->bullets[i].on && !ctx->bullets[i].dir && ctx->bullets[i].pos_x <= SHUTTLE_X+23 && ctx->bullets[i].pos_y+6 >= SHUTTLE_Y && ctx->bullets[i].pos_y <= SHUTTLE_Y+20)
            {
                ctx->gameover = true;
                clear_bullet(&ctx->bullets[i]);
                break;
            }
        }
        if(ctx->bullets[i].on && (ctx->bullets[i].pos_x >= 200 || ctx->bullets[i].pos_x <= 20))
        {
            clear_bullet(&ctx->bullets[i]);
        }
    }

    if(now - last_bullet_step >= BULLET_STEP_MS)
    {   
        last_bullet_step = now;
        for(int i=0;i<20;i++)
        {
            if(ctx->bullets[i].on)
            {
                if(ctx->bullets[i].new)ctx->bullets[i].new = false;
                else if(ctx->bullets[i].dir)ctx->bullets[i].pos_x +=6;
                else ctx->bullets[i].pos_x -=6;
            }
        }
    }

    for(int i = 0; i< 20;i++)
    {
        if(ctx->bullets[i].on)
        {
            if(ctx->bullets[i].dir)buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,ctx->bullets[i].pos_x,ctx->bullets[i].pos_y,bul_a,6,6,5);
            else buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,ctx->bullets[i].pos_x,ctx->bullets[i].pos_y,bul_b,6,6,5);
        }
    }
    if(!ctx->gameover)
    {
        build_spr_part(en[6],sprite_part);
        buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,SHUTTLE_X,SHUTTLE_Y,en[6],23,10,5);
    }
    else 
    {
        build_spr_part(en[7],sprite_part);
        buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,SHUTTLE_X,SHUTTLE_Y,en[7],23,10,5);
    }

    buf_draw_sprite_rgb565(framebuf,SCR_W,SCR_H,SHUTTLE_X,SHUTTLE_Y+10,sprite_part,23,10,5);
    draw_buf(0,0,SCR_W,SCR_H);

    for(int i=0;i<20;i++)
    {
        if((ctx->enemy[i].on && ctx->enemy[i].pos_x <= SHUTTLE_X+23) || ctx->gameover)
        {
            // Restart the app state after loss or when aliens reach the shuttle line.
            invaders_enter(self);
            return;
        }
    }
    return;
}

static void invaders_exit(app *self)
{
    invaders_ctx *ctx = (invaders_ctx*)self->ctx;

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

