#include "inject_app.h"
#include "sfw/apps/launcher_app.h"
#include "sfw/apps/scen_app.h"

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

void inject_start(void)
{
    inject_ctx_data.started = false;
    inject_ctx_data.done = false;
}

void inject_stop(void)
{
    inject_ctx_data.started = false;
    inject_ctx_data.done = false;
}

void inject_tick(void)
{
    tud_task();

    if (!tud_mounted()) return;
    if (inject_ctx_data.done) return;
    if (inject_ctx_data.path[0] == '\0') return;

    if (!inject_ctx_data.started)
    {
        inject_ctx_data.started = true;
        run_scen(inject_ctx_data.path);
        inject_ctx_data.done = true;
        set_mode(UI);
    }
}