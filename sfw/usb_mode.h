#ifndef USB_MODE_H
#define USB_MODE_H

#include <stdint.h>

typedef enum
{
    USB_NONE,
    USB_MSC,
    USB_HID
} usb_persona_t;

extern volatile usb_persona_t usb;

#endif