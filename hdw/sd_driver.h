#ifndef SD_DRIVER_H
#define SD_DRIVER_H

#include <stdint.h>
#include <stddef.h>
#include "hardware/spi.h"
#include "pico/stdlib.h"

#define SD_CS   17
#define SD_MISO 16
#define SD_MOSI 19
#define SD_SCK  18
#define SD_SPI_PORT   spi0

typedef struct 
{
    uint8_t cmd;
    uint32_t arg;
    uint8_t crc;
    bool selected; 
}tx_param;

typedef struct 
{
    uint8_t resp_len;
    uint8_t *resp_buf;
    uint8_t wait_limit;
    bool data_flag;
    uint8_t *data_buf;
    uint8_t data_len;
    bool crc_ok;
    bool deselect;  
}rx_param;

void SDInit(void);
void sd_send(tx_param *pkg);
uint8_t *sd_recieve(rx_param *pkg);

#endif