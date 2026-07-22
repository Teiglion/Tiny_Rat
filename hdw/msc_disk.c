#include <string.h>
#include "tusb.h"
#include "sd_driver.h"
#include "usb_mode.h"

#define MSC_BLOCK_SIZE 512

// Tell the host that no storage medium is currently available.
static void msc_set_no_medium(uint8_t lun)
{
    tud_msc_set_sense(lun, SCSI_SENSE_NOT_READY, 0x3A, 0x00);
}

// Tell the host that media state has changed and it should refresh cached info.
static void msc_set_medium_changed(uint8_t lun)
{
    tud_msc_set_sense(lun, SCSI_SENSE_UNIT_ATTENTION, 0x28, 0x00);
}

// Shared backend check for callbacks that need a mounted and initialized SD card.
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
    // If MSC mode is not active, behave like there is no medium attached.
    if (!usb_msc_is_ready())
    {
        msc_set_no_medium(lun);
        return false;
    }

    if (!msc_backend_ready(lun))
    {
        return false;
    }

    // Fail TUR once after media change so the host asks for sense data
    // and invalidates its cached view of the filesystem.
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
    // Writable only while MSC mode is exposed to the host.
    return usb_msc_is_ready();
}

void tud_msc_capacity_cb(uint8_t lun, uint32_t *block_count, uint16_t *block_size)
{
    (void) lun;

    if (!block_count || !block_size)
        return;

    // Expose the device as a plain 512-byte block storage.
    *block_size = MSC_BLOCK_SIZE;

    if (!usb_msc_is_ready() || !sd_initialized)
    {
        *block_count = 0;
        return;
    }

    // If sector count cannot be read, report empty capacity instead of garbage.
    if (!sd_sector_count(block_count))
        *block_count = 0;
}

void tud_msc_inquiry_cb(uint8_t lun, uint8_t vendor_id[8], uint8_t product_id[16], uint8_t product_rev[4])
{
    (void) lun;
    // SCSI INQUIRY strings are fixed-width and should be space-padded.
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

    // This backend supports only whole-sector transfers.
    if (offset != 0) return -1;
    if ((bufsize % MSC_BLOCK_SIZE) != 0) return -1;

    uint8_t *dst = (uint8_t *) buffer;
    uint32_t count = bufsize / MSC_BLOCK_SIZE;

    // TinyUSB gives us an LBA and transfer size in bytes;
    // we forward it as a sequence of single-block SD reads.
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
    
    // Reject partial-sector writes: SD backend is block-based.
    if (offset != 0) return -1;
    if ((bufsize % MSC_BLOCK_SIZE) != 0) return -1;

    uint32_t count = bufsize / MSC_BLOCK_SIZE;

    // Write request is split into consecutive 512-byte SD blocks.
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
    // No special power, spin-up, or eject handling here.
    return true;
}

int32_t tud_msc_scsi_cb(uint8_t lun, uint8_t const scsi_cmd[16], void *buffer, uint16_t bufsize)
{
    (void) bufsize;

    switch (scsi_cmd[0])
    {
    case SCSI_CMD_PREVENT_ALLOW_MEDIUM_REMOVAL:
        // Host may send this even for simple removable media; nothing to enforce.
        return 0;

    case SCSI_CMD_START_STOP_UNIT:
        // We do not physically start, stop, or eject anything.
        return 0;

    case SCSI_CMD_MODE_SENSE_6:
    {
        uint8_t *buf = (uint8_t *) buffer;
        if (!buf) return -1;

        // Minimal MODE SENSE(6) response is enough for common hosts.
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

        // Report "not ready" while MSC is unavailable.
        if (!usb_msc_is_ready())
        {
            buf[2] = SCSI_SENSE_NOT_READY;
            buf[12] = 0x3A;
            buf[13] = 0x00;
            return 18;
        }

        // Report media change once so the host refreshes filesystem state.
        if (usb_msc_should_report_sense_once())
        {
            buf[2] = SCSI_SENSE_UNIT_ATTENTION;
            buf[12] = 0x28;
            buf[13] = 0x00;
            return 18;
        }

        // Otherwise return "no additional sense information".
        buf[2] = 0x00;
        buf[12] = 0x00;
        buf[13] = 0x00;
        return 18;
    }

    default:
        if (!usb_msc_is_ready())
        {
            msc_set_no_medium(lun);
            return -1;
        }

        // Unsupported SCSI command.
        tud_msc_set_sense(lun, SCSI_SENSE_ILLEGAL_REQUEST, 0x20, 0x00);
        return -1;
    }
}