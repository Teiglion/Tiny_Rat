#include "disp_driver.h"

uint8_t framebuf[FRAMEBUF_SIZE];
static void display_select(){gpio_put(PIN_LCD_CS,0);}
static void display_deselect(){gpio_put(PIN_LCD_CS,1);}
static void display_cmd_mode(){gpio_put(PIN_LCD_DC,0);}
static void display_data_mode(){gpio_put(PIN_LCD_DC,1);}

static void lcd_reset() {
    gpio_put(PIN_LCD_RST, 1);
    sleep_ms(20);
    gpio_put(PIN_LCD_RST, 0);
    sleep_ms(20);
    gpio_put(PIN_LCD_RST, 1);
    sleep_ms(120);
}

static void lcd_write_cmd(uint8_t cmd)
{
    display_select();
    display_cmd_mode();
    spi_write_blocking(LCD_SPI_PORT,&cmd,1);
    display_deselect();
}

static void lcd_write_data(const uint8_t *data, size_t len)
{
    display_select();
    display_data_mode();
    spi_write_blocking(LCD_SPI_PORT,data,len);
    display_deselect();
}

static void lcd_write_cmd_data(uint8_t cmd,const uint8_t *data, size_t len)
{
    display_select();
    display_cmd_mode();
    spi_write_blocking(LCD_SPI_PORT,&cmd,1);
    if(len)
    {
        display_data_mode();
        spi_write_blocking(LCD_SPI_PORT,data,len);
    }
    display_deselect();
}

void DisplayInit(void)
{
    spi_init(LCD_SPI_PORT,20000000);
    spi_set_format(LCD_SPI_PORT,8,SPI_CPOL_0, SPI_CPHA_0, SPI_MSB_FIRST);
    gpio_set_function(PIN_LCD_SCK,GPIO_FUNC_SPI);
    gpio_set_function(PIN_LCD_MOSI,GPIO_FUNC_SPI);

    gpio_init(PIN_LCD_CS);
    gpio_set_dir(PIN_LCD_CS,GPIO_OUT);
    gpio_put(PIN_LCD_CS,1);

    gpio_init(PIN_LCD_DC);
    gpio_set_dir(PIN_LCD_DC,GPIO_OUT);

    gpio_init(PIN_LCD_RST);
    gpio_set_dir(PIN_LCD_RST,GPIO_OUT);

    gpio_init(PIN_LCD_BK);
    gpio_set_dir(PIN_LCD_BK,GPIO_OUT);
    gpio_put(PIN_LCD_BK,0);

    lcd_reset();

    lcd_write_cmd(0x01);
    sleep_ms(150);

        lcd_write_cmd(0x11);
        sleep_ms(120);

    {
        uint8_t d[]={0x55};
        lcd_write_cmd_data(0x3A,d,sizeof(d));
    }

    {
        uint8_t d[]={0x00};
        lcd_write_cmd_data(0x36,d,sizeof(d));
    }

    lcd_write_cmd(0x21);
    lcd_write_cmd(0x13);
    lcd_write_cmd(0x29); 
    fill_rect(0,0,240,280,0x0000);
    sleep_ms(20);
    gpio_put(PIN_LCD_BK,1);
}

void set_window(uint16_t x0,uint16_t y0,uint16_t x1,uint16_t y1)
{
    uint8_t data[4];
    x0+=LCD_X_OFFSET;
    x1+=LCD_X_OFFSET;
    y0+=LCD_Y_OFFSET;
    y1+=LCD_Y_OFFSET;

    data[0]=x0 >> 8;
    data[1]=x0 & 0xFF;
    data[2]=x1 >> 8;
    data[3]=x1 & 0xFF;
    lcd_write_cmd_data(0x2A,data,sizeof(data));

    data[0]=y0 >> 8;
    data[1]=y0 & 0xFF;
    data[2]=y1 >> 8;
    data[3]=y1 & 0xFF;
    lcd_write_cmd_data(0x2B,data,sizeof(data));

    lcd_write_cmd(0x2C);
}

static bool clip_rect(uint16_t x, uint16_t y, uint16_t *w, uint16_t *h)
{
    if (x >= LCD_WIDTH || y >= LCD_HEIGHT) return false;
    if (*w == 0 || *h == 0) return false;
    if (x + *w > LCD_WIDTH)  *w = LCD_WIDTH - x;
    if (y + *h > LCD_HEIGHT) *h = LCD_HEIGHT - y;
    return true;
}

void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if (!clip_rect(x, y, &w, &h)) return;
    set_window(x,y,x+w-1,y+h-1);
    uint8_t buf[128];
    for(int i=0;i<128;i+=2)
    {
        buf[i]= color >> 8;
        buf[i+1] = color & 0xFF;
    }
    display_select();
    display_data_mode();
    uint32_t total=(uint32_t)w*h,sent=0;
    while(sent<total)
    {
        uint32_t remaining=total-sent;
        uint32_t chunk = remaining > 64 ? 64 : remaining;
        spi_write_blocking(LCD_SPI_PORT,buf,chunk*2);
        sent+=chunk;
    }
    display_deselect();
}

void stream_img(FIL *f,uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if (!clip_rect(x, y, &w, &h)) return;
    set_window(x,y,x+w-1,y+h-1);
    display_select();
    display_data_mode();
    uint32_t total=(uint32_t)w*h*2,sent=0;
    uint8_t buf[2048];
    while(sent<total)
    {
        uint32_t remaining=total-sent;
        uint32_t chunk = remaining > sizeof(buf) ? sizeof(buf) : remaining;
        UINT br = 0;
        FRESULT fr = f_read(f, buf, chunk, &br);
        if (fr != FR_OK) break;
        if (br < chunk) {
            for (UINT i = br; i < chunk; i++) buf[i] = 0x00;
        }
        spi_write_blocking(LCD_SPI_PORT,buf,chunk);
        sent+=chunk;
    }
    display_deselect();
}

static void draw_raw_buf(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buf)
{
    if (!clip_rect(x, y, &w, &h)) return;

    set_window(x, y, x + w - 1, y + h - 1);
    display_select();
    display_data_mode();

    uint32_t total = (uint32_t)w * h * 2;
    uint32_t sent = 0;

    while (sent < total)
    {
        uint32_t chunk = total - sent;
        if (chunk > 4096) chunk = 4096;
        spi_write_blocking(LCD_SPI_PORT, buf + sent, chunk);
        sent += chunk;
    }

    display_deselect();
}

void draw_buf(uint16_t x, uint16_t y, uint16_t w, uint16_t h){draw_raw_buf(x, y, w, h, framebuf);}

void draw_buf_ex(uint16_t x, uint16_t y, uint16_t w, uint16_t h, const uint8_t *buf){draw_raw_buf(x, y, w, h, buf);}