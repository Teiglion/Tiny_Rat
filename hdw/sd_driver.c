#include "sd_driver.h"

static void sd_select(){gpio_put(SD_CS,0);}
static void sd_deselect(){gpio_put(SD_CS,1);}

void sd_send(tx_param *pkg)
{
    uint8_t buf[6];
    buf[0]=0x40 | pkg->cmd;
    buf[1]=pkg->arg >> 24;
    buf[2]=pkg->arg >> 16;
    buf[3]=pkg->arg >> 8;
    buf[4]=pkg->arg & 0xFF;
    buf[5]=pkg->crc;
    if(!pkg->selected)pkg->selected=1;
    spi_write_blocking(SD_SPI_PORT,buf,sizeof(buf));
}

uint8_t *sd_recieve(rx_param *pkg)
{
    
   // spi_read_blocking(SD_SPI_PORT,0xFF,buf,6);
   sd_deselect();
   pkg->deselect=1;
    //return buf;
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

    {
        uint8_t buf[10]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        spi_write_blocking(SD_SPI_PORT,buf,sizeof(buf));
    }

    {
        tx_param pkg={.cmd=0,.arg=0,.crc=0,.selected=0};
        pkg.arg=0x00;
        pkg.crc=0x95;
        sd_send(&pkg);

    }


    sd_deselect();
}