#include "disk_app.h"
#include "sfw/apps/launcher_app.h"

typedef struct
{
    bool started;
} disk_ctx;

static disk_ctx disk_ctx_data = {0};

static void disk_draw(void)
{
    node **root = ui_root();

    if (*root)kill_node(root);

    create_layer(root, 0, 0, 240, 280, "/apps/disk_app/storage_mode.res", 0);
}

static void disk_enter(app *self)
{
    disk_ctx *ctx = (disk_ctx *)self->ctx;

    ctx->started = false;
    disk_draw();
}

static void disk_on_event(app *self, const app_event *ev)
{
    (void)self;
    (void)ev;
}

static void disk_tick(app *self)
{
    disk_ctx *ctx = (disk_ctx *)self->ctx;

    if (ctx->started) return;

    ctx->started = true;
    set_mode(STORAGE);
}

static void disk_exit(app *self)
{
    disk_ctx *ctx = (disk_ctx *)self->ctx;
    ctx->started = false;
}

app disk_app = {
    .name = "disk",
    .enter = disk_enter,
    .on_event = disk_on_event,
    .tick = disk_tick,
    .exit = disk_exit,
    .ctx = &disk_ctx_data
};  