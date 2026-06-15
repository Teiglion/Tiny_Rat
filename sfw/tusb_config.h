#pragma once

#ifdef __cplusplus
extern "C" {
#endif

//--------------------------------------------------------------------
// Common configuration
//--------------------------------------------------------------------

#define CFG_TUSB_MCU             OPT_MCU_RP2040



#define CFG_TUD_ENABLED          1
#define CFG_TUH_ENABLED          0

#define CFG_TUSB_RHPORT0_MODE   OPT_MODE_DEVICE

#ifndef CFG_TUD_ENDPOINT0_SIZE
#define CFG_TUD_ENDPOINT0_SIZE   64
#endif

//--------------------------------------------------------------------
// Device configuration
//--------------------------------------------------------------------
#define CFG_TUD_MSC                 1
#define CFG_TUD_MSC_EP_BUFSIZE      512

#define CFG_TUD_MAX_SPEED        OPT_MODE_FULL_SPEED

//------------- CLASS -------------//
#define CFG_TUD_CDC              0
#define CFG_TUD_MSC              1
#define CFG_TUD_HID              0
#define CFG_TUD_MIDI             0
#define CFG_TUD_VENDOR           0

//------------- HID -------------//
#define CFG_TUD_HID_EP_BUFSIZE   16

#ifdef __cplusplus
}
#endif