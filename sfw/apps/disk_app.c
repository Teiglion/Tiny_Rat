#include "disk_app.h"
#include "sfw/usb_mode.h"
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
    if (!ev) return;
    if (ev->type == TOUCH && ev->x >= 20 && ev->x <= 220 && ev->y >= 220 && ev->y <= 260)
    {
        usb_msc_hide();
        hard_assert(fs_mount_root());
        app_open(&launcher_app);
    }
}

static void disk_tick(app *self)
{
    disk_ctx *ctx = (disk_ctx *)self->ctx;

    if (ctx->started) return;

    ctx->started = true;
    fs_unmount_root();
    usb_msc_show();
}

static void disk_exit(app *self)
{
    disk_ctx *ctx = (disk_ctx *)self->ctx;
    ctx->started = false;
    if (usb_msc_is_ready())usb_msc_hide();
    if (!fs_root_is_mounted())hard_assert(fs_mount_root());
}

app disk_app = {
    .name = "disk",
    .enter = disk_enter,
    .on_event = disk_on_event,
    .tick = disk_tick,
    .exit = disk_exit,
    .ctx = &disk_ctx_data
};  