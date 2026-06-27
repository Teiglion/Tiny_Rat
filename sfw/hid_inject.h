#ifndef HID_INJECT_H
#define HID_INJECT_H

#include "tusb.h"
#include <stdint.h>

uint16_t tud_hid_get_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t report_type,uint8_t* buffer,uint16_t reqlen);
void tud_hid_set_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t report_type,uint8_t const* buffer,uint16_t bufsize);
bool ascii_to_hid(char c, uint8_t *modifier, uint8_t *keycode);
void hid_send_key(uint8_t modifier, uint8_t keycode);
void hid_send_keys(uint8_t modifier, uint8_t key1, uint8_t key2, uint8_t key3, uint8_t key4, uint8_t key5, uint8_t key6);
void hid_send_text(const char *s);

#endif