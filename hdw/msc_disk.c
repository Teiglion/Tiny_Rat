#include <string.h>
#include "tusb.h"
#include "sd_driver.h"
#include "FatFS/ff.h"
#include "usb_mode.h"

#define MSC_BLOCK_SIZE 512

static void msc_set_no_medium(uint8_t lun)
{
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
}

static void msc_set_medium_changed(uint8_t lun)
{
    tud_msc_set_sense(lun, SCSI_SENSE_UNIT_ATTENTION, 0x28, 0x00);
}

static bool msc_backend_ready(uint8_t lun)
{
    if (!sd_initialized)
    {
        msc_set_no_medium(lun);
        return false;
    }

    return true;
}

bool tud_msc_test_unit_ready_cb(uint8_t lun)
{
    if (!usb_msc_is_enabled())
    {
        msc_set_no_medium(lun);
        return false;
    }

    if (!msc_backend_ready(lun))
    {
        return false;
    }

    if (usb_msc_should_fail_tur_once())
    {
        msc_set_medium_changed(lun);
        return false;
    }

    return true;
}

bool tud_msc_is_writable_cb(uint8_t lun)
{
    (void) lun;
    return usb_msc_is_ready();
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    (void) lun;

    if (!block_count || !block_size)
        return;

    *block_size = MSC_BLOCK_SIZE;

    if (!usb_msc_is_ready() || !sd_initialized)
    {
        *block_count = 0;
        return;
    }

    if (!sd_sector_count(block_count))
        *block_count = 0;
}

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    (void) lun;

    memset(vendor_id, ' ', 8);
    memset(product_id, ' ', 16);
    memset(product_rev, ' ', 4);

    memcpy(vendor_id, "Teiglion", 8);
    memcpy(product_id, "RP2040 SD", 8);
    memcpy(product_rev, "1.0", 3);
}

int32_t tud_msc_read10_cb(uint8_t lun, uint32_t lba, uint32_t offset, void *buffer, uint32_t bufsize)
{
    if (!usb_msc_is_ready())
    {
        msc_set_no_medium(lun);
        return -1;
    }

    if (!msc_backend_ready(lun))
        return -1;

    if (offset != 0) return -1;
    if ((bufsize % MSC_BLOCK_SIZE) != 0) return -1;

    uint8_t *dst = (uint8_t *) buffer;
    uint32_t count = bufsize / MSC_BLOCK_SIZE;

    for (uint32_t i = 0; i < count; i++)
    {
        if (sd_read_block(lba + i, dst + i * MSC_BLOCK_SIZE, MSC_BLOCK_SIZE))
            return -1;
    }

    return (int32_t) bufsize;
}

int32_t tud_msc_write10_cb(uint8_t lun, uint32_t lba, uint32_t offset, uint8_t *buffer, uint32_t bufsize)
{
    if (!usb_msc_is_ready())
    {
        msc_set_no_medium(lun);
        return -1;
    }

    if (!msc_backend_ready(lun))
        return -1;

    if (offset != 0) return -1;
    if ((bufsize % MSC_BLOCK_SIZE) != 0) return -1;

    uint32_t count = bufsize / MSC_BLOCK_SIZE;

    for (uint32_t i = 0; i < count; i++)
    {
        if (sd_write_block(lba + i, buffer + i * MSC_BLOCK_SIZE, MSC_BLOCK_SIZE))
            return -1;
    }

    return (int32_t) bufsize;
}

bool tud_msc_start_stop_cb(uint8_t lun, uint8_t power_condition, bool start, bool load_eject)
{
    (void) lun;
    (void) power_condition;
    (void) start;
    (void) load_eject;
    return true;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    (void) bufsize;

    switch (scsi_cmd[0])
    {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        return 0;

    case SCSI_CMD_START_STOP_UNIT:
        return 0;

    case SCSI_CMD_MODE_SENSE_6:
    {
        uint8_t *buf = (uint8_t *) buffer;
        if (!buf) return -1;

        buf[0] = 0x03;
        buf[1] = 0x00;
        buf[2] = 0x00;
        buf[3] = 0x00;
        return 4;
    }

    case SCSI_CMD_REQUEST_SENSE:
    {
        uint8_t *buf = (uint8_t *) buffer;
        if (!buf) return -1;

        memset(buf, 0, 18);
        buf[0] = 0x70;
        buf[7] = 10;

        if (!usb_msc_is_enabled())
        {
            buf[2] = SCSI_SENSE_NOT_READY;
            buf[12] = 0x3A;
            buf[13] = 0x00;
            return 18;
        }

        if (usb_msc_should_report_sense_once())
        {
            buf[2] = SCSI_SENSE_UNIT_ATTENTION;
            buf[12] = 0x28;
            buf[13] = 0x00;
            return 18;
        }

        buf[2] = 0x00;
        buf[12] = 0x00;
        buf[13] = 0x00;
        return 18;
    }

    default:
        if (!usb_msc_is_enabled())
        {
            msc_set_no_medium(lun);
            return -1;
        }

        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
        return -1;
    }
}