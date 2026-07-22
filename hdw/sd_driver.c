#include "sd_driver.h"

static void sd_select(){gpio_put(SD_CS,0);}
static void sd_deselect(){gpio_put(SD_CS,1);}

bool sd_initialized = false;
bool sd_is_sdhc = false;
static uint32_t sd_sector_count_cached = 0;

void sd_send(tx_param *pkg)
{
    uint8_t buf[6];
    buf[0]=0x40 | pkg->cmd;
    buf[1]=pkg->arg >> 24;
    buf[2]=pkg->arg >> 16;
    buf[3]=pkg->arg >> 8;
    buf[4]=pkg->arg & 0xFF;
    buf[5]=pkg->crc;

    // Some command sequences keep CS asserted across multiple transfers,
    // so selection is controlled by the caller through pkg->selected.
    if(!pkg->selected){pkg->selected=1; sd_select();}

    spi_write_blocking(SD_SPI_PORT,buf,sizeof(buf));
}

void sd_recieve(rx_param *pkg)
{
     // First wait for the R1 response byte: 0xFF means the card is still busy
    for(int i=0;i<pkg->wait_limit;i++)
    {
        uint8_t temp;
        spi_read_blocking(SD_SPI_PORT,0xFF,&temp,1);
        if(temp != 0xFF)
        {
            if(pkg->resp_buf != NULL)pkg->resp_buf[0]=temp;
            if(temp != pkg->expected_r1)pkg->error=1;
            break;
        }
        if(i==pkg->wait_limit-1)pkg->error=1;
    }
    // Some commands return extra response bytes after R1
    if(!pkg->error && pkg->resp_len > 1 && pkg->resp_buf != NULL)spi_read_blocking(SD_SPI_PORT,0xFF,pkg->resp_buf+1,pkg->resp_len-1);
    if(!pkg->error && pkg->data_len)
    {
        // For data reads we also wait for the start token 0xFE
        for(int i=0;i<pkg->wait_limit;i++)
        {
         uint8_t temp;
          spi_read_blocking(SD_SPI_PORT,0xFF,&temp,1);
          if(temp == 0xFE)
           {
               if(pkg->data_buf != NULL)spi_read_blocking(SD_SPI_PORT,0xFF,pkg->data_buf,pkg->data_len);
               else
               {
                // Caller may request to skip payload and just clock it out
                 uint8_t t;
                 for(int j=0;j<pkg->data_len;j++)spi_read_blocking(SD_SPI_PORT,0xFF,&t,1);
               }
               break;
           }
           if(i==pkg->wait_limit-1)pkg->error=1;
        }
        // CRC is currently ignored, but it still has to be consumed from the bus.
        uint8_t crc[2];
        if(!pkg->error)spi_read_blocking(SD_SPI_PORT,0xFF,crc,2);
    }
    sd_deselect();
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
         // Send extra dummy clocks with CS high so the card can enter SPI mode cleanly.
        uint8_t buf[10]={0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF,0xFF};
        spi_write_blocking(SD_SPI_PORT,buf,sizeof(buf));
    }

    tx_param tx_pkg={.cmd=0,.arg=0,.crc=0,.selected=0};
    tx_pkg.arg=0x00;
    tx_pkg.crc=0x95;
    sd_send(&tx_pkg);
    
    uint8_t r;
    rx_param rx_pkg={.error=0,.crc_ok=0,.wait_limit=100,.data_len=0,.resp_buf=&r,.resp_len=1,.expected_r1=0x01};
    sd_recieve(&rx_pkg);

    // CMD0 should put the card into idle state in SPI mode.
    if(rx_pkg.expected_r1 == r && !rx_pkg.error)
    {
        tx_pkg.selected=0;
        tx_pkg.cmd=0x08;
        tx_pkg.arg=0x000001AA;
        tx_pkg.crc=0x87;
        sd_send(&tx_pkg);


        rx_pkg.error=0;
        rx_pkg.data_len=0;
        rx_pkg.expected_r1=0x01;
        uint8_t rbuf[5];
        rx_pkg.resp_buf=rbuf;
        rx_pkg.resp_len=5;
        sd_recieve(&rx_pkg);
        // CMD8 echo tells us that this is a modern card that understands 2.7-3.6V.
        if(rbuf[1]== 0x00 && rbuf[2]== 0x00 && rbuf[3]== 0x01 && rbuf[4]==0xAA)
        {
            int done=0;
            while(!done)
            {
                tx_pkg.selected=0;
                tx_pkg.cmd=55;
                tx_pkg.arg=0x00;
                tx_pkg.crc=0x01;
                sd_send(&tx_pkg);

                rx_pkg.error=0;
                rx_pkg.data_len=0;
                rx_pkg.expected_r1=0x01;
                r=0;
                rx_pkg.resp_buf=&r;
                rx_pkg.resp_len=1;
                sd_recieve(&rx_pkg);

                if(rx_pkg.expected_r1 == r && !rx_pkg.error)
                {
                    r=1;
                    tx_pkg.selected=0;
                    tx_pkg.cmd=41;
                    tx_pkg.arg=0x40000000;
                    tx_pkg.crc=0x01;
                    sd_send(&tx_pkg);

                    rx_pkg.error=0;
                    rx_pkg.data_len=0;
                    rx_pkg.expected_r1=0x00;
                    rx_pkg.resp_buf=&r;
                    rx_pkg.resp_len=1;
                    sd_recieve(&rx_pkg);
                    // ACMD41 may need many retries; 0x00 means the card is ready.
                    if(r == 0x00)done=1;
                }
            }

            tx_pkg.selected=0;
            tx_pkg.cmd=58;
            tx_pkg.arg=0x00;
            tx_pkg.crc=0x01;
            sd_send(&tx_pkg);

            rx_pkg.error=0;
            rx_pkg.data_len=0;
            rx_pkg.expected_r1=0x00;
            for(int i=0;i<5;i++)rbuf[i]=0;
            rx_pkg.resp_buf=rbuf;
            rx_pkg.resp_len=5;
            sd_recieve(&rx_pkg);
            if (!rx_pkg.error && rbuf[0] == 0x00) 
            {
                uint32_t ocr_val =((uint32_t)rbuf[1] << 24) |((uint32_t)rbuf[2] << 16) | ((uint32_t)rbuf[3] << 8) |((uint32_t)rbuf[4]);
                // OCR CCS bit tells us whether the card uses SDHC/SDXC block addressing.
                sd_is_sdhc = (ocr_val & (1u << 30)) != 0;
            }
            
        }
        
    }
     // Right now init is considered successful only for SDHC-style cards.
    sd_initialized=sd_is_sdhc;
    if (sd_initialized) {
        // After card init we can switch to a faster SPI clock.
        spi_init(SD_SPI_PORT, 20000000);
    }
}

bool sd_sector_count(uint32_t *sector_count)
{
    if(!sector_count)return false;
    // Capacity is stable, so cache it after the first successful CSD read.
    if (sd_initialized && sd_sector_count_cached != 0) 
    {
        *sector_count = sd_sector_count_cached;
        return true;
    }
    tx_param tx_pkg={0};
    rx_param rx_pkg={0};

    tx_pkg.selected=0;
    tx_pkg.cmd=9;
    tx_pkg.arg=0x00000000;
    tx_pkg.crc=0x01;
    sd_send(&tx_pkg);

    rx_pkg.error=0;
    uint8_t dbuf[16];
    uint8_t r;
    rx_pkg.data_buf=dbuf;
    rx_pkg.data_len=16;
    rx_pkg.wait_limit=10000;
    rx_pkg.expected_r1=0x00;
    rx_pkg.resp_buf=&r;
    rx_pkg.resp_len=1;
    sd_recieve(&rx_pkg);

    if(rx_pkg.error)return false;

    uint8_t csd_structure = (dbuf[0] >> 6) & 0x03;
    // This formula is for CSD v2.0 only, which matches SDHC cards.
    if (csd_structure != 1) return false;
    
    uint32_t result = ((uint32_t)(dbuf[7] & 0x3F) << 16)| ((uint32_t)dbuf[8] << 8) | dbuf[9];
    *sector_count=(result+1)*1024;
    sd_sector_count_cached=(result+1)*1024;
    return true;
}

bool sd_read_block(uint32_t block_num, uint8_t *buf, size_t len)
{
    bool err=true;
    if(!sd_initialized)return err;
    if(buf==NULL)return err;
    if (len != 512) return err;

    tx_param tx_pkg={0};
    tx_pkg.selected=0;
    tx_pkg.cmd=17;
    // For SDHC cards CMD17 uses block number directly, not byte address.
    tx_pkg.arg=block_num;
    tx_pkg.crc=0x01;
    sd_send(&tx_pkg);

    uint8_t r;
    rx_param rx_pkg={0};
    rx_pkg.error=0;
    rx_pkg.wait_limit = 10000;
    rx_pkg.expected_r1=0x00;
    rx_pkg.resp_buf=&r;
    rx_pkg.resp_len=1;
    rx_pkg.data_buf=buf;
    rx_pkg.data_len=len;
    sd_recieve(&rx_pkg);
    if(!rx_pkg.error)err=false;
    return err;
}

bool sd_write_block(uint32_t block_num, const uint8_t *buf, size_t len)
{
    if (!sd_initialized) return true;
    if (buf == NULL) return true;
    if (len != 512) return true;

    tx_param tx_pkg = {0};
    tx_pkg.selected = 0;
    tx_pkg.cmd = 24;
    // Same as read path: SDHC cards use logical block address directly.
    tx_pkg.arg = block_num;
    tx_pkg.crc = 0x01;
    sd_send(&tx_pkg);

    uint8_t r = 0xFF;
    uint8_t ff = 0xFF;

    // Wait until the card returns command response instead of idle bus bytes.
    for (uint32_t i = 0; i < 10000; i++) {
        spi_read_blocking(SD_SPI_PORT, 0xFF, &r, 1);
        if (r != 0xFF) break;
    }

    if (r != 0x00) {
        sd_deselect();
        spi_write_blocking(SD_SPI_PORT, &ff, 1);
        return true;
    }

    // One extra idle byte before the data token keeps the write flow stable.
    spi_write_blocking(SD_SPI_PORT, &ff, 1);

    uint8_t token = 0xFE;
    spi_write_blocking(SD_SPI_PORT, &token, 1);

    spi_write_blocking(SD_SPI_PORT, buf, len);

    // Dummy CRC is fine here because CRC checking is not used in SPI mode.
    uint8_t crc[2] = {0xFF, 0xFF};
    spi_write_blocking(SD_SPI_PORT, crc, 2);

    r = 0xFF;
    for (uint32_t i = 0; i < 10000; i++) {
        spi_read_blocking(SD_SPI_PORT, 0xFF, &r, 1);
        if (r != 0xFF) break;
    }

    // Data response 0x05 means "data accepted".
    if ((r & 0x1F) != 0x05) {
        sd_deselect();
        spi_write_blocking(SD_SPI_PORT, &ff, 1);
        return true;
    }

    // After accepting the block the card stays busy for a while,
    // and releases the bus back to 0xFF when internal programming is done.
    for (uint32_t i = 0; i < 200000; i++) 
    {
        spi_read_blocking(SD_SPI_PORT, 0xFF, &r, 1);
        if (r == 0xFF) 
        {
            sd_deselect();
            spi_write_blocking(SD_SPI_PORT, &ff, 1);
            return false;
        }
    }

    sd_deselect();
    spi_write_blocking(SD_SPI_PORT, &ff, 1);
    return true;
}