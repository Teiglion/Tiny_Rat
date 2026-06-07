#include "disp_driver.h"

static void display_select(){gpio_put(PIN_LCD_CS,0);}
static void display_deselect(){gpio_put(PIN_LCD_CS,1);}
static void display_cmd_mode(){gpio_put(PIN_LCD_DC,0);}
static void display_data_mode(){gpio_put(PIN_LCD_DC,1);}
//static void DisplayDeinit(){spi_deinit(LCD_SPI_PORT);}

void lcd_reset() {
    gpio_put(PIN_LCD_RST, 1);
    sleep_ms(20);
    gpio_put(PIN_LCD_RST, 0);
    sleep_ms(20);
    gpio_put(PIN_LCD_RST, 1);
    sleep_ms(120);
}

void lcd_write_cmd(uint8_t cmd)
{
    display_select();
    display_cmd_mode();
    spi_write_blocking(LCD_SPI_PORT,&cmd,1);
    display_deselect();
}

void lcd_write_data(uint8_t *data, size_t len)
{
    display_select();
    display_data_mode();
    spi_write_blocking(LCD_SPI_PORT,data,len);
    display_deselect();
}

void lcd_write_cmd_data(uint8_t cmd,uint8_t *data, size_t len)
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

void DisplayInit()
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

void fill_rect(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
    if(x>=LCD_WIDTH || y>=LCD_HEIGHT)return;
    if(w==0 || h==0)return;
    if(x+w>LCD_WIDTH)w=LCD_WIDTH-x;
    if(y+h>LCD_HEIGHT)h=LCD_HEIGHT-y;
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

void stream_img(FILE *f,uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if(x>=LCD_WIDTH || y>=LCD_HEIGHT)return;
    if(w==0 || h==0)return;
    if(x+w>LCD_WIDTH)w=LCD_WIDTH-x;
    if(y+h>LCD_HEIGHT)h=LCD_HEIGHT-y;
    set_window(x,y,x+w-1,y+h-1);
    display_select();
    display_data_mode();
    uint32_t total=(uint32_t)w*h*2,sent=0;
    uint8_t buf[128];
    while(sent<total)
    {
        uint32_t remaining=total-sent;
        uint32_t chunk = remaining > sizeof(buf) ? sizeof(buf) : remaining;
        size_t got = fread(buf,1,chunk,f);
        if(got!=chunk)
        {
            for(size_t i=got;i<chunk;i++)
            buf[i]=0xFF;
        }
        spi_write_blocking(LCD_SPI_PORT,buf,chunk);
        sent+=chunk;
    }
    display_deselect();
}
