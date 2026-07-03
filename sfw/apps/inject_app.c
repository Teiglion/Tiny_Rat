#include "inject_app.h"
#include "sfw/apps/launcher_app.h"
#include "sfw/apps/scen_app.h"

typedef struct
{
    bool done;
    char path[250];
} inject_ctx;

static inject_ctx inject_ctx_data = {0};

const char *inject_get_path(void){return inject_ctx_data.path;}

void inject_set_path(const char *path)
{
    if (!path) return;

    memset(inject_ctx_data.path, 0, sizeof(inject_ctx_data.path));
    strncpy(inject_ctx_data.path, path, sizeof(inject_ctx_data.path) - 1);
    inject_ctx_data.done = false;
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
    ctx->done = false;
    inject_draw();
}

static void inject_on_event(app *self, const app_event *ev)
{
    (void)self;
    (void)ev;
}

static void inject_tick(app *self)
{
    inject_ctx *ctx = (inject_ctx *)self->ctx;

    if (!tud_mounted()) return;
    if (ctx->done) return;
    if (ctx->path[0] == '\0') return;

    ctx->done = true;
    run_scen(ctx->path);
    app *back = app_prev();
    if (!back || back == app_current()) 
    {
        app_open(&launcher_app);
        return;
    }
    app_open(back);
}

static void inject_exit(app *self)
{
    inject_ctx *ctx = (inject_ctx *)self->ctx;
    ctx->done = false;
}

app inject_app = {
    .name = "inject",
    .enter = inject_enter,
    .on_event = inject_on_event,
    .tick = inject_tick,
    .exit = inject_exit,
    .ctx = &inject_ctx_data
};