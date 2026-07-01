#include "tusb.h"
#include "usb_mode.h"

static tusb_desc_device_t const desc_device = {
    .bLength            = sizeof(tusb_desc_device_t),
    .bDescriptorType    = TUSB_DESC_DEVICE,
    .bcdUSB             = 0x0200,

    .bDeviceClass       = 0x00,
    .bDeviceSubClass    = 0x00,
    .bDeviceProtocol    = 0x00,

    .bMaxPacketSize0    = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor           = 0xCafe,
    .idProduct          = 0x4010,
    .bcdDevice          = 0x0100,

    .iManufacturer      = 0x01,
    .iProduct           = 0x02,
    .iSerialNumber      = 0x03,

    .bNumConfigurations = 0x01
};

uint8_t const *tud_descriptor_device_cb(void)
{
    return (uint8_t const *) &desc_device;
}

//--------------------------------------------------------------------+
// HID Report Descriptor
//--------------------------------------------------------------------+

uint8_t const hid_report_desc[] = {
    TUD_HID_REPORT_DESC_KEYBOARD(HID_REPORT_ID(1)),
    TUD_HID_REPORT_DESC_MOUSE(HID_REPORT_ID(2))
};

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance)
{
    (void) instance;
    return hid_report_desc;
}

//--------------------------------------------------------------------+
// Configuration Descriptor (COMPOSITE: MSC + HID)
//--------------------------------------------------------------------+

enum
{
    ITF_NUM_MSC = 0,
    ITF_NUM_HID,
    ITF_NUM_TOTAL
};

#define CONFIG_TOTAL_LEN   (TUD_CONFIG_DESC_LEN + TUD_MSC_DESC_LEN + TUD_HID_DESC_LEN)

#define EPNUM_MSC_OUT      0x01
#define EPNUM_MSC_IN       0x81
#define EPNUM_HID_IN       0x82

static uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, CONFIG_TOTAL_LEN, 0x00, 100),

    TUD_MSC_DESCRIPTOR(ITF_NUM_MSC, 4, EPNUM_MSC_OUT, EPNUM_MSC_IN, 64),

    TUD_HID_DESCRIPTOR(
        ITF_NUM_HID,
        5,
        HID_ITF_PROTOCOL_NONE,
        sizeof(hid_report_desc),
        EPNUM_HID_IN,
        CFG_TUD_HID_EP_BUFSIZE,
        1
    )
};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index)
{
    (void) index;
    return desc_configuration;
}

//--------------------------------------------------------------------+
// String Descriptors
//--------------------------------------------------------------------+

static const char *string_desc_arr[] = {
    (const char[]) { 0x09, 0x04 },
    "Teiglion",
    "RP2040 Composite",
    "123456",
    "SD Card",
    "HID Interface"
};

static uint16_t _desc_str[32];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid)
{
    (void) langid;

    uint8_t chr_count;

    if (index == 0)
    {
        _desc_str[1] = 0x0409;
        chr_count = 1;
    }
    else
    {
        if (!(index < 6)) return NULL;

        const char *str = string_desc_arr[index];
        chr_count = 0;

        while (str[chr_count] && chr_count < 31)
        {
            _desc_str[1 + chr_count] = str[chr_count];
            chr_count++;
        }
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}