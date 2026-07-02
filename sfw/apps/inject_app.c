#include "inject_app.h"

typedef struct
{
    bool started;
    bool done;
    char path[250];
} inject_ctx;

static inject_ctx inject_ctx_data = {0};

void inject_set_path(const char *path)
{
    if (!path) return;

    memset(inject_ctx_data.path, 0, sizeof(inject_ctx_data.path));
    strncpy(inject_ctx_data.path, path, sizeof(inject_ctx_data.path) - 1);

    inject_ctx_data.started = false;
    inject_ctx_data.done = false;
}

const char *inject_get_path(void)
{
    return inject_ctx_data.path;
}

static void inject_draw(void)
{
    node **root = ui_root();

    if (*root) kill_node(root);
    create_layer(root, 0, 0, 240, 280, "/apps/inject_app/inj.res", 0);
}

static void inject_enter(app *self)
{
    inject_ctx *ctx = (inject_ctx *)self->ctx;

    ctx->started = false;
    ctx->done = false;
    inject_draw();
}

static void inject_on_event(app *self, const app_event *ev)
{
    (void)self;
    (void)ev;
}

static void inject_tick_app(app *self)
{
    inject_ctx *ctx = (inject_ctx *)self->ctx;

    tud_task();

    if (!tud_mounted()) return;
    if (ctx->done) return;
    if (ctx->path[0] == '\0') return;

    if (!ctx->started)
    {
        ctx->started = true;
        run_scen(ctx->path);
        ctx->done = true;
        app_open(&scen_app);
        return;
    }
}

static void inject_exit(app *self)
{
    inject_ctx *ctx = (inject_ctx *)self->ctx;

    ctx->started = false;
    ctx->done = false;
}

app inject_app = {
    .name = "inject",
    .enter = inject_enter,
    .on_event = inject_on_event,
    .tick = inject_tick_app,
    .exit = inject_exit,
    .ctx = &inject_ctx_data
};