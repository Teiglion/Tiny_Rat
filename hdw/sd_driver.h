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
    size_t resp_len;
    uint8_t *resp_buf;
    uint32_t wait_limit;
    uint8_t *data_buf;
    size_t data_len;
    uint8_t expected_r1;
    uint8_t expected_crc[2];
    bool error;
    bool crc_ok;
}rx_param;

extern bool sd_initialized;
extern bool sd_is_sdhc;

void SDInit(void);
void sd_send(tx_param *pkg);
void sd_recieve(rx_param *pkg);
bool sd_read_block(uint32_t block_num, uint8_t *buf, size_t len);
bool sd_write_block(uint32_t block_num, const uint8_t *buf, size_t len);
bool sd_sector_count(uint32_t *sector_count);

#endif