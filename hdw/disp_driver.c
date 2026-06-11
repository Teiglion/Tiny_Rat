#include "disp_driver.h"

#define FRAMEBUF_SIZE (LCD_WIDTH * LCD_HEIGHT * 2)

uint8_t framebuf[FRAMEBUF_SIZE];
static void display_select(){gpio_put(PIN_LCD_CS,0);}
static void display_deselect(){gpio_put(PIN_LCD_CS,1);}
static void display_cmd_mode(){gpio_put(PIN_LCD_DC,0);}
static void display_data_mode(){gpio_put(PIN_LCD_DC,1);}

void lcd_reset() {
    gpio_put(PIN_LCD_RST, 1);
    sleep_ms(20);
    gpio_put(PIN_LCD_RST, 0);
    sleep_ms(20);
    gpio_put(PIN_LCD_RST, 1);
    sleep_ms(120);
}

void touch_reset() {
    gpio_put(PIN_TOUCH_RST, 0);
    sleep_ms(20);
    gpio_put(PIN_TOUCH_RST, 1);
    sleep_ms(200);
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

static bool touch_read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    int ret = i2c_write_timeout_us(TOUCH_I2C_PORT, TOUCH_ADDR, &reg, 1, true, 2000);
    if (ret < 0) return false;

    ret = i2c_read_timeout_us(TOUCH_I2C_PORT, TOUCH_ADDR, buf, len, false, 2000);
    return ret == (int)len;
}

bool TouchRead(uint16_t *x, uint16_t *y)
{
    if (!x || !y) return false;

    uint8_t status;
    if (!touch_read_regs(0x02, &status,1)) {
        return false;
    }

    if ((status & 0x0F) == 0) {
        return false;
    }

    uint8_t data[4];
    if (!touch_read_regs(0x03, data, 4)) {
        return false;
    }

    uint16_t tx = ((data[0] & 0x0F) << 8) | data[1];
    uint16_t ty = ((data[2] & 0x0F) << 8) | data[3];

    *x = tx;
    *y = ty;
    return true;
}

void TouchInit(void)
{
    i2c_init(TOUCH_I2C_PORT,400000);
    gpio_set_function(PIN_TOUCH_SCL,GPIO_FUNC_I2C);
    gpio_set_function(PIN_TOUCH_SDA,GPIO_FUNC_I2C);
    gpio_pull_up(PIN_TOUCH_SDA);
    gpio_pull_up(PIN_TOUCH_SCL);

    gpio_init(PIN_TOUCH_RST);
    gpio_set_dir(PIN_TOUCH_RST,GPIO_OUT);

    gpio_init(PIN_TOUCH_INT);
    gpio_set_dir(PIN_TOUCH_INT,GPIO_IN);
    gpio_pull_up(PIN_TOUCH_INT);
    touch_reset();
}

void TouchTestToFile(void)
{
    FIL file;
    UINT bw;
    FRESULT fr;

    fr = f_open(&file, "touch_cst816.txt", FA_CREATE_ALWAYS | FA_WRITE);
    if (fr != FR_OK) return;

    f_write(&file, "cst816 test start\r\n", 18, &bw);
    f_sync(&file);

    if (gpio_get(PIN_TOUCH_INT)) {
        f_write(&file, "INT=1\r\n", 7, &bw);
    } else {
        f_write(&file, "INT=0\r\n", 7, &bw);
    }
    f_sync(&file);

    uint8_t val;

    if (touch_read_regs(0x01, &val,1)) {
        f_write(&file, "REG01 OK\r\n", 10, &bw);
    } else {
        f_write(&file, "REG01 FAIL\r\n", 12, &bw);
    }

    if (touch_read_regs(0x02, &val,1)) {
        f_write(&file, "REG02 OK\r\n", 10, &bw);
    } else {
        f_write(&file, "REG02 FAIL\r\n", 12, &bw);
    }

    uint8_t data[4];
    if (touch_read_regs(0x03, data, 4)) {
        f_write(&file, "XY READ OK\r\n", 12, &bw);

        uint16_t x = ((data[0] & 0x0F) << 8) | data[1];
        uint16_t y = ((data[2] & 0x0F) << 8) | data[3];

        char line[64];
        int len = snprintf(line, sizeof(line), "X=%u Y=%u\r\n", x, y);
        f_write(&file, line, (UINT)len, &bw);
    } else {
        f_write(&file, "XY READ FAIL\r\n", 14, &bw);
    }

    f_sync(&file);
    f_close(&file);
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

void stream_img(FIL *f,uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if(x>=LCD_WIDTH || y>=LCD_HEIGHT)return;
    if(w==0 || h==0)return;
    if(x+w>LCD_WIDTH)w=LCD_WIDTH-x;
    if(y+h>LCD_HEIGHT)h=LCD_HEIGHT-y;
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

void draw_buf(uint16_t x, uint16_t y, uint16_t w, uint16_t h)
{
    if(x>=LCD_WIDTH || y>=LCD_HEIGHT)return;
    if(w==0 || h==0)return;
    if(x+w>LCD_WIDTH)w=LCD_WIDTH-x;
    if(y+h>LCD_HEIGHT)h=LCD_HEIGHT-y;
    set_window(x,y,x+w-1,y+h-1);
    display_select();
    display_data_mode();
    uint32_t total=(uint32_t)w*h*2,sent=0;
    while(sent<total)
    {
        uint32_t chunk =total-sent;
        if (chunk > 4096) chunk = 4096;
        spi_write_blocking(LCD_SPI_PORT, framebuf + sent, chunk);
        sent += chunk;
    }
    display_deselect();
}