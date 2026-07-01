#include "usb_mode.h"
#include "pico/stdlib.h"

static volatile msc_state_t state = MSC_HIDDEN;
static volatile bool show_attention_pending = false;
static volatile absolute_time_t hide_deadline;
static volatile bool hide_deadline_armed = false;

#define MSC_HIDE_DELAY_MS 800

void usb_msc_show(void)
{
    hide_deadline_armed = false;
    show_attention_pending = true;
    state = MSC_APPEARING;
}

void usb_msc_hide(void)
{
    show_attention_pending = false;
    state = MSC_DISAPPEARING;
    hide_deadline = make_timeout_time_ms(MSC_HIDE_DELAY_MS);
    hide_deadline_armed = true;
}

void usb_msc_poll(void)
{
    if (state == MSC_DISAPPEARING && hide_deadline_armed)
    {
        if (absolute_time_diff_us(get_absolute_time(), hide_deadline) <= 0)
        {
            hide_deadline_armed = false;
            state = MSC_HIDDEN;
        }
    }
}

bool usb_msc_is_enabled(void)
{
    return state == MSC_APPEARING || state == MSC_READY;
}

bool usb_msc_is_ready(void)
{
    return state == MSC_READY;
}

msc_state_t usb_msc_state(void)
{
    return state;
}

bool usb_msc_take_attention(void)
{
    if (!show_attention_pending)
        return false;

    show_attention_pending = false;
    state = MSC_READY;
    return true;
}