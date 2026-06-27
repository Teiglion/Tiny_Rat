#include "hid_inject.h"

uint16_t tud_hid_get_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t report_type,uint8_t* buffer,uint16_t reqlen)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t report_type,uint8_t const* buffer,uint16_t bufsize)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
}

bool ascii_to_hid(char c, uint8_t *modifier, uint8_t *keycode)
{
    *modifier = 0;
    *keycode = 0;
    if(c >= 'a' && c <= 'z')
    {
        *keycode = HID_KEY_A + (c - 'a');
        return true;
    }
    if(c >= 'A' && c <= 'Z')
    {
        *modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        *keycode = HID_KEY_A + (c - 'A');
        return true;
    }
    if(c >= '1' && c <= '9')
    {
        *keycode = HID_KEY_1 + (c - '1');
        return true;
    }
    if(c == '0')
    {
        *keycode = HID_KEY_0;
        return true;
    }
    if(c == ' ')
    {
        *keycode = HID_KEY_SPACE;
        return true;
    }
    if(c == '\n')
    {
        *keycode = HID_KEY_ENTER;
        return true;
    }
    if(c == '.')
    {
        *keycode = HID_KEY_PERIOD;
        return true;
    }
    if(c == ',')
    {
        *keycode = HID_KEY_COMMA;
        return true;
    }
    if(c == '-')
    {
        *keycode = HID_KEY_MINUS;
        return true;
    }
    if(c == '_')
    {
        *modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        *keycode = HID_KEY_MINUS;
        return true;
    }
    if(c == '/')
    {
        *keycode = HID_KEY_SLASH;
        return true;
    }
    if(c == ':')
    {
        *modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        *keycode = HID_KEY_SEMICOLON;
        return true;
    }
    if (c == '@')
    {
        *modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        *keycode = HID_KEY_2;
        return true;
    }
    return false;
}

void hid_send_key(uint8_t modifier, uint8_t keycode)
{
    uint8_t press[6] = {0};
    uint8_t release[6] = {0};
    press[0] = keycode;
    while (!tud_mounted())tud_task();
    while (!tud_hid_ready())tud_task();
    tud_hid_keyboard_report(1,modifier,press);
    for (int i = 0; i < 20; i++) 
    {
        tud_task();
        sleep_ms(1);
    }
    tud_hid_keyboard_report(1,0,release);
    for (int i = 0; i < 20; i++) 
    {
        tud_task();
        sleep_ms(1);
    }
}

void hid_send_keys(uint8_t modifier, uint8_t key1, uint8_t key2, uint8_t key3, uint8_t key4, uint8_t key5, uint8_t key6)
{
    uint8_t press[6] = { key1, key2, key3, key4, key5, key6 };
    uint8_t release[6] = {0};
    while (!tud_mounted())tud_task();
    while (!tud_hid_ready())tud_task();
    tud_hid_keyboard_report(1,modifier,press);
    for (int i = 0; i < 20; i++) 
    {
        tud_task();
        sleep_ms(1);
    }
    tud_hid_keyboard_report(1,0,NULL);
    for (int i = 0; i < 20; i++) 
    {
        tud_task();
        sleep_ms(1);
    }
}

void hid_send_text(const char *s)
{
    while(*s)
    {
        uint8_t modifier = 0;
        uint8_t keycode = 0;
        if(ascii_to_hid(*s,&modifier,&keycode))
        {
            hid_send_key(modifier,keycode);
        }
        s++;
    }
}