#include "launcher_app.h"
#include "sfw/apps/scen_app.h"
#include "sfw/apps/disk_app.h"

typedef struct
{
    bool busy;
} launcher_ctx;

static launcher_ctx launcher_ctx_data = {0};

static void launcher_draw(void)
{
    node **root = ui_root();

    if (*root)kill_node(root);

    create_layer(root, 0, 0, 240, 280, "/sys/11.res", 0);
    add_sublayer(&(*root)->l, 40, 20, 66, 66, "/apps/disk_app/disk_app.res", 0);
    add_sublayer(&(*root)->l, 134, 20, 66, 66, "/apps/scen_app/scen_app.res", 0);
}

static void launcher_enter(app *self)
{
    launcher_ctx *ctx = (launcher_ctx *)self->ctx;

    ctx->busy = false;
    launcher_draw();
}

static void launcher_on_event(app *self, const app_event *ev)
{
    launcher_ctx *ctx = (launcher_ctx *)self->ctx;

    if (!ev) return;

    if (ev->type == TOUCH && ev->x >= 40 && ev->x <= 106 && ev->y >= 20 && ev->y <= 86 && !ctx->busy)
    {
        ctx->busy = true;
        app_open(&disk_app);
        return;
    }

    if (ev->type == TOUCH && ev->x >= 134 && ev->x <= 200 && ev->y >= 20 && ev->y <= 86 && !ctx->busy)
    {
        ctx->busy = true;
        app_open(&scen_app);
        return;
    }

    if (ev->type == BUT1 && !ctx->busy)
    {
        ctx->busy = true;
        set_mode(INJECT);
        return;
    }

    if (ev->type == SWIPE_RIGHT)
    {
        ctx->busy = false;
        return;
    }
}

static void launcher_tick(app *self)
{
    (void)self;
}

static void launcher_exit(app *self)
{
    launcher_ctx *ctx = (launcher_ctx *)self->ctx;
    ctx->busy = false;
}

app launcher_app = {
    .name = "launcher",
    .enter = launcher_enter,
    .on_event = launcher_on_event,
    .tick = launcher_tick,
    .exit = launcher_exit,
    .ctx = &launcher_ctx_data
};