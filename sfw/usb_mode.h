#ifndef USB_MODE_H
#define USB_MODE_H

#include <stdbool.h>
#include <stdint.h>

typedef enum
{
    MSC_HIDDEN = 0,
    MSC_APPEARING,
    MSC_READY,
    MSC_DISAPPEARING
} msc_state_t;

void usb_msc_show(void);
void usb_msc_hide(void);
void usb_msc_poll(void);

bool usb_msc_is_enabled(void);
bool usb_msc_is_ready(void);
msc_state_t usb_msc_state(void);
bool usb_msc_take_attention(void);

#endif