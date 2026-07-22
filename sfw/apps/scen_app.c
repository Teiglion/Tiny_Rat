#include "scen_app.h"
#include "sfw/apps/launcher_app.h"
#include "sfw/apps/inject_app.h"

#define SCEN_PATH "/apps/scen_app/scripts"
#define SCEN_PER_PAGE 8
#define SCEN_NAME_LEN 64

#define LIST_X 20
#define LIST_Y0 25
#define LIST_STEP 30
#define TOUCH_X1 20
#define TOUCH_X2 220

typedef struct
{
    bool is_end;
    bool is_top;
    int index;
    int visible_count;
    char names[SCEN_PER_PAGE][SCEN_NAME_LEN];

    bool confirm_open;
    char selected_path[128];
    char selected_name[SCEN_NAME_LEN];
} scen_ctx;

static scen_ctx scen_ctx_data = {0};

static void scen_clear_names(scen_ctx *ctx)
{
    if (!ctx) return;

    ctx->visible_count = 0;
    for (int i = 0; i < SCEN_PER_PAGE; i++)
        memset(ctx->names[i], 0, sizeof(ctx->names[i]));
}

static void scen_reset_confirm(scen_ctx *ctx)
{
    if (!ctx) return;

    ctx->confirm_open = false;
    memset(ctx->selected_path, 0, sizeof(ctx->selected_path));
    memset(ctx->selected_name, 0, sizeof(ctx->selected_name));
}

static void scen_build_path(char *dst, size_t dst_sz, const char *name)
{
    if (!dst || dst_sz == 0 || !name) return;
    snprintf(dst, dst_sz, "%s/%s", SCEN_PATH, name);
}

static int scen_row_from_touch(int x, int y)
{
    if (x < TOUCH_X1 || x > TOUCH_X2) return -1;
    if (y < LIST_Y0) return -1;

    int row = (y - LIST_Y0) / LIST_STEP;
    if (row < 0 || row >= SCEN_PER_PAGE) return -1;

    return row;
}

static void scen_draw_list_base(void)
{
    node **root = ui_root();

    if (*root) kill_node(root);
    create_layer(root, 0, 0, 240, 280, NULL, 0xFFFF);
}

void print_scens(scen_ctx *ctx, const char *path)
{
    if (!ctx || !path) return;

    // index is a 1-based offset of the first visible script on the current page.
    ctx->is_top = (ctx->index <= 1);
    ctx->is_end = true;
    scen_clear_names(ctx);

    DIR dir;
    FILINFO fno;
    FRESULT res;

    fill_rect(0, 0, 240, 280, 0xFFFF);

    res = f_opendir(&dir, path);
    if (res != FR_OK) return;

    int skip = ctx->index - 1;
    while (skip > 0)
    {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0)
        {
            f_closedir(&dir);
            return;
        }
        skip--;
    }

    int y = LIST_Y0;
    while (ctx->visible_count < SCEN_PER_PAGE)
    {
        res = f_readdir(&dir, &fno);
        if (res != FR_OK || fno.fname[0] == 0)
            break;

        strncpy(ctx->names[ctx->visible_count], fno.fname, SCEN_NAME_LEN - 1);
        ctx->names[ctx->visible_count][SCEN_NAME_LEN - 1] = '\0';

        draw_text_cutted(LIST_X, y, ctx->names[ctx->visible_count], 0x0000, 2);
        y += 15;
        fill_rect(LIST_X, y, 200, 2, 0x0000);
        y += 15;

        ctx->visible_count++;
    }

    res = f_readdir(&dir, &fno);
    if (res == FR_OK && fno.fname[0] != 0)
        ctx->is_end = false;

    if (!ctx->is_top) draw_text(100, 10, "PREV", 0x0000, 1);
    if (!ctx->is_end) draw_text(100, 270, "NEXT", 0x0000, 1);

    f_closedir(&dir);
}

static void scen_redraw_list(scen_ctx *ctx)
{
    scen_draw_list_base();
    print_scens(ctx, SCEN_PATH);
}

static void scen_open_confirm(scen_ctx *ctx)
{
    node **root = ui_root();

    if (!ctx || !root) return;
    if (ctx->confirm_open) return;

    create_layer(root, 0, 0, 240, 280, "/apps/scen_app/confirm.res", 0);
    draw_text(20, 40, ctx->selected_name, 0x0000, 3);

    ctx->confirm_open = true;
}

static void scen_close_confirm(scen_ctx *ctx)
{
    node **root = ui_root();

    if (!ctx || !root || !*root) return;
    if (!ctx->confirm_open) return;

    pop_node(root);
    scen_reset_confirm(ctx);

    print_scens(ctx, SCEN_PATH);
}

void run_scen(const char *file)
{
    FIL f;
    FRESULT fr;
    char buf[500];

    // Scenario format:
    // - lines starting with "KEY" are parsed as HID key commands
    // - all other lines are sent as plain text
    fr = f_open(&f, file, FA_READ);
    if (fr != FR_OK) return;

    while (f_gets(buf, sizeof(buf), &f))
    {
        char *p = buf;
        bool comm = false;

        if (*p == 'K' && *(p + 1) == 'E' && *(p + 2) == 'Y')
        {
            // KEY-prefixed line switches parser into command mode.
            comm = true;
            p++; p++; p++;
        }

        uint8_t mod[4] = {0};
        uint8_t keys[6] = {0};
        int i = 0, j = 0;
        bool plus = false;

        while (*p && *p != '\r' && *p != '\n' && *p != '\0')
        {
            if (*p == '+' && comm)
            {
                 // After '+', remaining characters are converted from ASCII to HID keycodes.
                plus = true;
                p++;
                continue;
            }

            if (comm && !plus)
            {
                if (j < 6)
                {
                    switch (*p)
                    {
                        case 'W': if (i < 4) mod[i++] = KEYBOARD_MODIFIER_LEFTGUI; break;
                        case 'S': if (i < 4) mod[i++] = KEYBOARD_MODIFIER_LEFTSHIFT; break;
                        case 'C': if (i < 4) mod[i++] = KEYBOARD_MODIFIER_LEFTCTRL; break;
                        case 'A': if (i < 4) mod[i++] = KEYBOARD_MODIFIER_LEFTALT; break;
                        case '1': if (j < 6) keys[j++] = HID_KEY_F1; break;
                        case '2': if (j < 6) keys[j++] = HID_KEY_F2; break;
                        case '3': if (j < 6) keys[j++] = HID_KEY_F3; break;
                        case '4': if (j < 6) keys[j++] = HID_KEY_F4; break;
                        case '5': if (j < 6) keys[j++] = HID_KEY_F5; break;
                        case '6': if (j < 6) keys[j++] = HID_KEY_F6; break;
                        case '7': if (j < 6) keys[j++] = HID_KEY_F7; break;
                        case '8': if (j < 6) keys[j++] = HID_KEY_F8; break;
                        case '9': if (j < 6) keys[j++] = HID_KEY_F9; break;
                        case '0': if (j < 6) keys[j++] = HID_KEY_F10; break;
                        case '-': if (j < 6) keys[j++] = HID_KEY_F11; break;
                        case '=': if (j < 6) keys[j++] = HID_KEY_F12; break;
                        case 'E': if (j < 6) keys[j++] = HID_KEY_ENTER; break;
                        case 'e': if (j < 6) keys[j++] = HID_KEY_ESCAPE; break;
                        case 'T': if (j < 6) keys[j++] = HID_KEY_TAB; break;
                        case 'D': if (j < 6) keys[j++] = HID_KEY_DELETE; break;
                        case 'B': if (j < 6) keys[j++] = HID_KEY_BACKSPACE; break;
                        case 's': if (j < 6) keys[j++] = HID_KEY_SPACE; break;
                        case 'I': if (j < 6) keys[j++] = HID_KEY_INSERT; break;
                        case 'P': if (j < 6) keys[j++] = HID_KEY_PRINT_SCREEN; break;
                        case '{': if (j < 6) keys[j++] = HID_KEY_ARROW_UP; break;
                        case '}': if (j < 6) keys[j++] = HID_KEY_ARROW_DOWN; break;
                        case '[': if (j < 6) keys[j++] = HID_KEY_ARROW_LEFT; break;
                        case ']': if (j < 6) keys[j++] = HID_KEY_ARROW_RIGHT; break;
                        case '^': if (j < 6) keys[j++] = HID_KEY_PAGE_UP; break;
                        case 'v': if (j < 6) keys[j++] = HID_KEY_PAGE_DOWN; break;
                        case 'H': if (j < 6) keys[j++] = HID_KEY_HOME; break;
                        case 'N': if (j < 6) keys[j++] = HID_KEY_END; break;
                        case 'X': if (j < 6) keys[j++] = HID_KEY_CAPS_LOCK; break;
                        case 'M': if (j < 6) keys[j++] = HID_KEY_MENU; break;
                    }
                }
            }

            if (comm && plus && j < 6)
            {
                ascii_to_hid(*p, NULL, keys + j);
                j++;
            }

            p++;
        }

        if (comm)
            hid_send_keys(mod[0] | mod[1] | mod[2] | mod[3], keys[0], keys[1], keys[2], keys[3], keys[4], keys[5]);
        else
            hid_send_text(buf);

        // Fixed delay between scenario lines to give the host time to process input.
        sleep_ms(1500);
    }

    f_close(&f);
}

static void scen_enter(app *self)
{
    scen_ctx *ctx = (scen_ctx *)self->ctx;

    ctx->index = 1;
    ctx->is_end = true;
    ctx->is_top = true;
    scen_clear_names(ctx);
    scen_reset_confirm(ctx);

    scen_redraw_list(ctx);
}

static void scen_on_event(app *self, const app_event *ev)
{
    scen_ctx *ctx = (scen_ctx *)self->ctx;
    if (!ctx || !ev) return;

    if (ctx->confirm_open)
    {
        if (ev->type == SWIPE_RIGHT)
        {
            scen_close_confirm(ctx);
            return;
        }

        if (ev->type == TOUCH)
        {
            if (ev->x >= 20 && ev->x <= 220 && ev->y >= 200 && ev->y <= 260)
            {
                inject_set_path(ctx->selected_path);
                scen_close_confirm(ctx);
                app_open(&inject_app);
                return;
            }
        }

        return;
    }

    if (ev->type == SWIPE_UP && ctx->index > SCEN_PER_PAGE)
    {
        ctx->index -= SCEN_PER_PAGE;
        scen_redraw_list(ctx);
        return;
    }

    if (ev->type == SWIPE_DOWN && !ctx->is_end)
    {
        ctx->index += SCEN_PER_PAGE;
        scen_redraw_list(ctx);
        return;
    }

    if (ev->type == SWIPE_RIGHT)
    {
        app_open(&launcher_app);
        return;
    }

    if (ev->type == TOUCH)
    {
        int row = scen_row_from_touch(ev->x, ev->y);
        if (row < 0) return;
        if (row >= ctx->visible_count) return;
        if (ctx->names[row][0] == '\0') return;

        strncpy(ctx->selected_name, ctx->names[row], SCEN_NAME_LEN - 1);
        ctx->selected_name[SCEN_NAME_LEN - 1] = '\0';
        scen_build_path(ctx->selected_path, sizeof(ctx->selected_path), ctx->names[row]);
        scen_open_confirm(ctx);
        return;
    }
}

static void scen_tick(app *self)
{
    (void)self;
}

static void scen_exit(app *self)
{
    scen_ctx *ctx = (scen_ctx *)self->ctx;
    node **root = ui_root();

    if (ctx->confirm_open && root && *root)
        pop_node(root);

    scen_clear_names(ctx);
    scen_reset_confirm(ctx);
    ctx->index = 1;
    ctx->is_end = true;
    ctx->is_top = true;
}

app scen_app = {
    .name = "scen",
    .enter = scen_enter,
    .on_event = scen_on_event,
    .tick = scen_tick,
    .exit = scen_exit,
    .ctx = &scen_ctx_data
};