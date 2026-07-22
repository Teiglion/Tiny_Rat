#include "hid_inject.h"

uint16_t tud_hid_get_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t report_type,uint8_t* buffer,uint16_t reqlen)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) reqlen;

    // This injector does not expose custom input reports on request.
    return 0;
}

void tud_hid_set_report_cb(uint8_t instance,uint8_t report_id,hid_report_type_t report_type,uint8_t const* buffer,uint16_t bufsize)
{
    (void) instance;
    (void) report_id;
    (void) report_type;
    (void) buffer;
    (void) bufsize;
    // Host output reports are currently ignored.
}

bool ascii_to_hid(char c, uint8_t *modifier, uint8_t *keycode)
{
    if(modifier)*modifier = 0;
    if(keycode)*keycode = 0;

    // This is a simple US-layout ASCII -> HID mapping.
    // Host layout still decides what character finally appears on screen.
    if(c >= 'a' && c <= 'z')
    {
        if(keycode)*keycode = HID_KEY_A + (c - 'a');
        return true;
    }
    if(c >= 'A' && c <= 'Z')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_A + (c - 'A');
        return true;
    }
    if(c >= '1' && c <= '9')
    {
        if(keycode)*keycode = HID_KEY_1 + (c - '1');
        return true;
    }
    if(c == '0')
    {
        if(keycode)*keycode = HID_KEY_0;
        return true;
    }
    if(c == ' ')
    {
        if(keycode)*keycode = HID_KEY_SPACE;
        return true;
    }
    if(c == '\n')
    {
        if(keycode)*keycode = HID_KEY_ENTER;
        return true;
    }
    if(c == '.')
    {
        if(keycode)*keycode = HID_KEY_PERIOD;
        return true;
    }
    if(c == ',')
    {
        if(keycode)*keycode = HID_KEY_COMMA;
        return true;
    }
    if(c == '-')
    {
        if(keycode)*keycode = HID_KEY_MINUS;
        return true;
    }
    if(c == '_')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_MINUS;
        return true;
    }
    if(c == '/')
    {
        if(keycode)*keycode = HID_KEY_SLASH;
        return true;
    }
    if(c == '\\')
    {
        if(keycode)*keycode = HID_KEY_BACKSLASH;
        return true;
    }
    if(c == '"')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_APOSTROPHE;
        return true;
    }
    if (c == '!')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_1;
        return true;
    }
    if (c == '@')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_2;
        return true;
    }
    if (c == '#')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_3;
        return true;
    }
    if (c == '$')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_4;
        return true;
    }
    if (c == '%')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_5;
        return true;
    }
    if (c == '^')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_6;
        return true;
    }
    if (c == '&')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_7;
        return true;
    }
    if (c == '*')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_8;
        return true;
    }
    if (c == '(')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_9;
        return true;
    }
    if (c == ')')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_0;
        return true;
    }
    if(c == ';')
    {
        if(keycode)*keycode = HID_KEY_SEMICOLON;
        return true;
    }
    if (c == ':')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_SEMICOLON;
        return true;
    }
    if(c == '\'')
    {
        if(keycode)*keycode = HID_KEY_APOSTROPHE;
        return true;
    }
    if(c == '=')
    {
        if(keycode)*keycode = HID_KEY_EQUAL;
        return true;
    }
    if(c == '+')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_EQUAL;
        return true;
    }
    if(c == '[')
    {
        if(keycode)*keycode = HID_KEY_BRACKET_LEFT;
        return true;
    }
    if(c == ']')
    {
        if(keycode)*keycode = HID_KEY_BRACKET_RIGHT;
        return true;
    }
    if(c == '{')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_BRACKET_LEFT;
        return true;
    }
    if(c == '}')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_BRACKET_RIGHT;
        return true;
    }
    if(c == '<')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_COMMA;
        return true;
    }
    if(c == '>')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_PERIOD;
        return true;
    }
    if(c == '?')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_SLASH;
        return true;
    }
    if(c == '|')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_BACKSLASH;
        return true;
    }
    if(c == '`')
    {
        if(keycode)*keycode = HID_KEY_GRAVE;
        return true;
    }
    if(c == '~')
    {
        if(modifier)*modifier = KEYBOARD_MODIFIER_LEFTSHIFT;
        if(keycode)*keycode = HID_KEY_GRAVE;
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

    // Send key press first, then an empty report to release it.
    // Without the release report the host may treat the key as still held.
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

    // Keyboard report can carry up to 6 simultaneous non-modifier keys.
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

void hid_send_text(const char *s)
{
    if (!s) return;
    while(*s)
    {
        uint8_t modifier = 0;
        uint8_t keycode = 0;
        
        // Unsupported characters are silently skipped for now.
        if(ascii_to_hid(*s,&modifier,&keycode))
        {
            hid_send_key(modifier,keycode);
        }
        s++;
    }
}