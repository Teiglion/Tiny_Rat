#ifndef USB_MODE_H
#define USB_MODE_H

#include <stdbool.h>

typedef enum
{
    MSC_HIDDEN = 0,
    MSC_READY
} msc_state_t;

void usb_msc_show(void);
void usb_msc_hide(void);

bool usb_msc_is_enabled(void);
bool usb_msc_is_ready(void);
msc_state_t usb_msc_state(void);

bool usb_msc_should_fail_tur_once(void);
bool usb_msc_should_report_sense_once(void);

#endif