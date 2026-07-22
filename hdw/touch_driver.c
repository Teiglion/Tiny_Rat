#include "touch_driver.h"

// Clamp raw controller value to calibrated range
// and remap it to LCD pixel coordinates.
static uint16_t map_coord(int v, int in_min, int in_max, int out_max)
{
    if (v < in_min) v = in_min;
    if (v > in_max) v = in_max;
    return (uint16_t)((v - in_min) * out_max / (in_max - in_min));
}

static void touch_reset() {
    gpio_put(PIN_TOUCH_RST, 0);
    sleep_ms(20);
    gpio_put(PIN_TOUCH_RST, 1);
    sleep_ms(200);
}

static bool touch_read_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    int ret = i2c_write_timeout_us(TOUCH_I2C_PORT, TOUCH_ADDR, &reg, 1, true, 2000);
    if (ret < 0) return false;

    ret = i2c_read_timeout_us(TOUCH_I2C_PORT, TOUCH_ADDR, buf, len, false, 2000);
    return ret == (int)len;
}

static bool touch_write_regs(uint8_t reg, uint8_t *buf, size_t len)
{
    uint8_t temp[16];
    if(len+1 > sizeof(temp))return false;
    // Small writes are packed into one local buffer: register + payload.
    temp[0]=reg;
    for(size_t i=0;i<len;i++)
    {
        temp[i+1]=buf[i];
    }
    int ret = i2c_write_timeout_us(TOUCH_I2C_PORT, TOUCH_ADDR, temp, len+1, false, 2000);
    return ret == (int)(len+1);
}

bool TouchRead(uint8_t *data, uint16_t *x, uint16_t *y)
{
    if (!data || !x || !y) return false;

    // INT is active low: if the line is high, there is nothing new to read.
    if (gpio_get(PIN_TOUCH_INT) != 0) return false;
    
    if (!touch_read_regs(0x01, data, 6))return false;

    // Ignore empty packet / no-touch state.
    if (data[1] == 0 && data[0] == 0x00) return false;

    // Raw touch coordinates come from controller registers
    // and are then mapped into the LCD coordinate space.
    uint16_t tx = ((data[2] & 0x0F) << 8) | data[3];
    uint16_t ty = ((data[4] & 0x0F) << 8) | data[5];
    *x = map_coord(tx, TOUCH_X_MIN, TOUCH_X_MAX, LCD_WIDTH - 1);
    *y = map_coord(ty, TOUCH_Y_MIN, TOUCH_Y_MAX, LCD_HEIGHT - 1);
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
    uint8_t no_sleep=0x01;
    uint8_t timeout = 0xFF; 

    // Keep controller awake and stretch auto-sleep timeout
    // so touch reporting stays responsive during normal app work.
    if(!touch_write_regs(0xFE,&no_sleep,1))return;
    sleep_ms(5);
    if(!touch_write_regs(0xF9,&timeout,1))return;
    sleep_ms(5);
    
}