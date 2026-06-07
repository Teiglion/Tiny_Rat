#include "sd_driver.h"

static void sd_select(){gpio_put(SD_CS,0);}
static void sd_deselect(){gpio_put(SD_CS,1);}

void sd_send(uint8_t *cmd, uint8_t len){spi_write_blocking(SD_SPI_PORT,cmd,len);}

uint8_t *sd_recieve(uint8_t *buf,uint8_t len)
{
    spi_read_blocking(SD_SPI_PORT,0xFF,buf,len);
    return buf;
}

void SDInit(void)
{
    spi_init(SD_SPI_PORT,400000);
    spi_set_format(SD_SPI_PORT,8,0,0,SPI_MSB_FIRST);
    gpio_set_function(SD_MISO,GPIO_FUNC_SPI);
    gpio_set_function(SD_MOSI,GPIO_FUNC_SPI);
    gpio_set_function(SD_SCK,GPIO_FUNC_SPI);

    gpio_init(SD_CS);
    gpio_set_dir(SD_CS,GPIO_OUT);
    gpio_put(SD_CS,1);

    sd_select();
    {
        uint8_t cmd[10]={0xFF};
        sd_send_cmd(cmd);
    }

    {
        uint8_t cmd[6]={0x40,0x00,0x00,0x00,0x00,0x95};
        sd_send_cmd(cmd);
        uint8_t response[8];
        sd_response(response,0x08);
    }


    sd_deselect();
}