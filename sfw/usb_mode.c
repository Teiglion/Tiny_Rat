#include "usb_mode.h"

static volatile msc_state_t state = MSC_HIDDEN;
static volatile bool fail_tur_once = false;
static volatile bool report_sense_once = false;

void usb_msc_show(void)
{
    state = MSC_READY;
    fail_tur_once = true;
    report_sense_once = true;   
}

void usb_msc_hide(void)
{
    state = MSC_HIDDEN;
    fail_tur_once = false;
    report_sense_once = false;
}

bool usb_msc_is_ready(void)
{
    return state == MSC_READY;
}

msc_state_t usb_msc_state(void)
{
    return state;
}

bool usb_msc_should_fail_tur_once(void)
{
    if (fail_tur_once)
    {
        fail_tur_once = false;
        return true;
    }

    return false;
}

bool usb_msc_should_report_sense_once(void)
{
    if (report_sense_once)
    {
        report_sense_once = false;
        return true;
    }

    return false;
}