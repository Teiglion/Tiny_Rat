#include "text.h"

#define FONT_W 5
#define FONT_H 7

static const uint8_t font_digits[10][5] = {
    {0x3E,0x51,0x49,0x45,0x3E}, // 0
    {0x00,0x42,0x7F,0x40,0x00}, // 1
    {0x42,0x61,0x51,0x49,0x46}, // 2
    {0x21,0x41,0x45,0x4B,0x31}, // 3
    {0x18,0x14,0x12,0x7F,0x10}, // 4
    {0x27,0x45,0x45,0x45,0x39}, // 5
    {0x3C,0x4A,0x49,0x49,0x30}, // 6
    {0x01,0x71,0x09,0x05,0x03}, // 7
    {0x36,0x49,0x49,0x49,0x36}, // 8
    {0x06,0x49,0x49,0x29,0x1E}  // 9
};

static const uint8_t font_upper[26][5] = {
    {0x7E,0x11,0x11,0x11,0x7E}, // A
    {0x7F,0x49,0x49,0x49,0x36}, // B
    {0x3E,0x41,0x41,0x41,0x22}, // C
    {0x7F,0x41,0x41,0x22,0x1C}, // D
    {0x7F,0x49,0x49,0x49,0x41}, // E
    {0x7F,0x09,0x09,0x09,0x01}, // F
    {0x3E,0x41,0x49,0x49,0x7A}, // G
    {0x7F,0x08,0x08,0x08,0x7F}, // H
    {0x00,0x41,0x7F,0x41,0x00}, // I
    {0x20,0x40,0x41,0x3F,0x01}, // J
    {0x7F,0x08,0x14,0x22,0x41}, // K
    {0x7F,0x40,0x40,0x40,0x40}, // L
    {0x7F,0x02,0x0C,0x02,0x7F}, // M
    {0x7F,0x04,0x08,0x10,0x7F}, // N
    {0x3E,0x41,0x41,0x41,0x3E}, // O
    {0x7F,0x09,0x09,0x09,0x06}, // P
    {0x3E,0x41,0x51,0x21,0x5E}, // Q
    {0x7F,0x09,0x19,0x29,0x46}, // R
    {0x46,0x49,0x49,0x49,0x31}, // S
    {0x01,0x01,0x7F,0x01,0x01}, // T
    {0x3F,0x40,0x40,0x40,0x3F}, // U
    {0x1F,0x20,0x40,0x20,0x1F}, // V
    {0x7F,0x20,0x18,0x20,0x7F}, // W
    {0x63,0x14,0x08,0x14,0x63}, // X
    {0x03,0x04,0x78,0x04,0x03}, // Y
    {0x61,0x51,0x49,0x45,0x43}  // Z
};

static const uint8_t font_symbols[][5] = {
    {0x00,0x00,0x00,0x00,0x00}, // space
    {0x00,0x60,0x60,0x00,0x00}, // .
    {0x00,0x36,0x36,0x00,0x00}, // :
    {0x08,0x08,0x08,0x08,0x08}, // -
    {0x40,0x40,0x40,0x40,0x40}, // _
    {0x20,0x10,0x08,0x04,0x02}, // /
    {0x00,0x00,0x5F,0x00,0x00}, // !
    {0x14,0x14,0x14,0x14,0x14}, // =
};

static const uint8_t *glyph_for_char(char c)
{
    if(c >= '0' && c <= '9')return font_digits[c-'0'];
    if(c >= 'A' && c <= 'Z')return font_upper[c-'A'];

    // Lowercase letters reuse uppercase glyphs to keep the font tiny.
    if(c >= 'a' && c <= 'z')return font_upper[c-'a'];
    switch (c)
    {
        case ' ': return font_symbols[0];
        case '.': return font_symbols[1];
        case ':': return font_symbols[2];
        case '-': return font_symbols[3];
        case '_': return font_symbols[4];
        case '/': return font_symbols[5];
        case '!': return font_symbols[6];
        case '=': return font_symbols[7];
        default:  return font_symbols[0];
    }
}

void draw_char(uint16_t x, uint16_t y, char c, uint16_t color, uint8_t scale)
{
    if (scale == 0) scale = 1;
    const uint8_t *glyph = glyph_for_char(c);

    // Each glyph is stored as 5 columns of bit-packed pixels.
    for (uint8_t col=0;col<FONT_W;col++)
    {
        uint8_t bits = glyph[col];
        for(uint8_t row=0;row<FONT_H;row++)
        {
            if (bits & (1 << row))fill_rect(x+col*scale,y+row*scale,scale,scale,color);
        }
    }
}

void draw_text(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t scale)
{
    if (!text) return;
    if (scale == 0) scale = 1;
    uint16_t cursor_x = x;
    uint16_t cursor_y = y;
    while(*text)
    {
        if (*text == '\n') 
        {
            cursor_x = x;
            cursor_y += (FONT_H + 1) * scale + scale;
            text++;
            continue;
        }

        // Auto-wrap when the next character would go too far to the right.
        if(cursor_x+20 >= LCD_WIDTH)
        {
            cursor_x = x;
            cursor_y += (FONT_H + 1) * scale + scale;
            continue;
        }
        draw_char(cursor_x,cursor_y,*text,color,scale);
        cursor_x += (FONT_W + 1) * scale;
        text++;
    }
}

void draw_text_cutted(uint16_t x, uint16_t y, const char *text, uint16_t color, uint8_t scale)
{
    if (!text) return;
    if (scale == 0) scale = 1;
    uint16_t cursor_x = x;
    uint16_t cursor_y = y;
    while(*text)
    {
        if (*text == '\n') 
        {
            cursor_x = x;
            cursor_y += (FONT_H + 1) * scale + scale;
            text++;
            continue;
        }

        // Cut text instead of wrapping and mark truncation with '/'.
        if((cursor_x+20)+FONT_W*scale >= LCD_WIDTH)
        {
            draw_char(cursor_x,cursor_y,'/',color,scale);
            break;
        }
        draw_char(cursor_x,cursor_y,*text,color,scale);
        cursor_x += (FONT_W + 1) * scale;
        text++;
    }
}

uint16_t text_width(const char *text, uint8_t scale)
{
    if (!text) return 0;
    if (scale == 0) scale = 1;

    // Width is measured only for the first line.
    uint16_t w = 0;
    while (*text && *text != '\n') {
        w += (FONT_W + 1) * scale;
        text++;
    }
    return w;
}

uint16_t text_height(uint8_t scale)
{
    if (scale == 0) scale = 1;
    return FONT_H * scale;
}
