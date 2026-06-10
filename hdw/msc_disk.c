#include <string.h>
#include "tusb.h"
#include "hdw/sd_driver.h"

#define MSC_BLOCK_SIZE       512
#define EXPOSED_BLOCK_COUNT  65536u  // 32 МБ

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8],
                        uint8_t product_id[16], uint8_t product_rev[4])
{
    (void) lun;

    memset(vendor_id, ' ', 8);
    memset(product_id, ' ', 16);
    memset(product_rev, ' ', 4);

    memcpy(vendor_id, "SDCARD", 6);
    memcpy(product_id, "RP2040 MSC", 10);
    memcpy(product_rev, "1.0", 3);
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    (void) lun;
    if (!sd_initialized) {
        tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
        return false;
    }
    return true;
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    (void) lun;
    return true;
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t* block_count, uint16_t* block_size)
{
    (void) lun;
    *block_size  = MSC_BLOCK_SIZE;
    *block_count = EXPOSED_BLOCK_COUNT;
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                          void* buffer, uint32_t bufsize)
{
    (void) lun;

    if (bufsize == 0) return 0;

    if (offset != 0 || (bufsize % MSC_BLOCK_SIZE) != 0) {
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x24, 0x00);
        return -1;
    }

    uint32_t blocks = bufsize / MSC_BLOCK_SIZE;

    if (lba + blocks > EXPOSED_BLOCK_COUNT) {
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x21, 0x00);
        return -1;
    }

    uint8_t* dst = (uint8_t*) buffer;

    for (uint32_t i = 0; i < blocks; i++) {
        if (sd_read_block(lba + i, dst + i * MSC_BLOCK_SIZE, MSC_BLOCK_SIZE)) {
            tud_msc_set_sense(lun, SCSI_SENSE_MEDIUM_ERROR, 0x11, 0x00);
            return -1;
        }
    }

    return (int32_t) bufsize;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset,
                           uint8_t* buffer, uint32_t bufsize)
{
    (void) lun;

    if (bufsize == 0) return 0;

    if (offset != 0 || (bufsize % MSC_BLOCK_SIZE) != 0) {
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x24, 0x00);
        return -1;
    }

    uint32_t blocks = bufsize / MSC_BLOCK_SIZE;

    if (lba + blocks > EXPOSED_BLOCK_COUNT) {
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x21, 0x00);
        return -1;
    }

    for (uint32_t i = 0; i < blocks; i++) {
        if (sd_write_block(lba + i, buffer + i * MSC_BLOCK_SIZE, MSC_BLOCK_SIZE)) {
            tud_msc_set_sense(lun, SCSI_SENSE_MEDIUM_ERROR, 0x0C, 0x00);
            return -1;
        }
    }

    return (int32_t) bufsize;
}

void tud_msc_write10_complete_cb(uint8_t lun)
{
    (void) lun;
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    (void) lun;
    (void) power_condition;
    (void) start;
    (void) load_eject;
    return true;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16],
                        void* buffer, uint16_t bufsize)
{
    (void) bufsize;

    switch (scsi_cmd[0]) {
        case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        case SCSI_CMD_START_STOP_UNIT:
            return 0;

        case SCSI_CMD_MODE_SENSE_6: {
            uint8_t *buf = (uint8_t*) buffer;
            memset(buf, 0, 4);
            buf[0] = 3;
            return 4;
        }

        case SCSI_CMD_REQUEST_SENSE: {
            uint8_t *buf = (uint8_t*) buffer;
            memset(buf, 0, 18);
            buf[0] = 0x70;
            buf[7] = 10;
            return 18;
        }

        default:
            tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
            return -1;
    }
}