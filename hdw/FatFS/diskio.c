/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2025        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Basic definitions of FatFs */
#include "diskio.h"		/* Declarations FatFs MAI */

/* Example: Declarations of the platform and disk functions in the project */
#include "sd_driver.h"

/* Example: Mapping of physical drive number for each drive */
#define DEV_SD		0	/* Map MMC/SD card to physical drive 1 */



/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	if(pdrv!=DEV_SD)return STA_NOINIT;
	return sd_initialized ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	if(pdrv!=DEV_SD)return STA_NOINIT;
	SDInit();
	return sd_initialized ? 0 : STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Read Sector(s)                                                        */
/*-----------------------------------------------------------------------*/

DRESULT disk_read (
	BYTE pdrv,		/* Physical drive nmuber to identify the drive */
	BYTE *buff,		/* Data buffer to store read data */
	LBA_t sector,	/* Start sector in LBA */
	UINT count		/* Number of sectors to read */
)
{
	if(pdrv!=DEV_SD)return RES_PARERR;
	if(buff == NULL || count == 0)return RES_PARERR;
	if(!sd_initialized)return RES_NOTRDY;
	for(int i=0;i<count;i++)
	{
		if(sd_read_block((uint32_t)(sector + i),buff + i * 512,512))
		return RES_ERROR;
	}
	return RES_OK;
}



/*-----------------------------------------------------------------------*/
/* Write Sector(s)                                                       */
/*-----------------------------------------------------------------------*/

#if FF_FS_READONLY == 0

DRESULT disk_write (
	BYTE pdrv,			/* Physical drive nmuber to identify the drive */
	const BYTE *buff,	/* Data to be written */
	LBA_t sector,		/* Start sector in LBA */
	UINT count			/* Number of sectors to write */
)
{
    if (pdrv != DEV_SD) return RES_PARERR;
    if (buff == NULL || count == 0) return RES_PARERR;
    if (!sd_initialized) return RES_NOTRDY;

    for (UINT i = 0; i < count; i++) {
        if (sd_write_block((uint32_t)(sector + i), (uint8_t*)buff + i * 512, 512)) {
            return RES_ERROR;
        }
    }
    return RES_OK;
}
#endif


/*-----------------------------------------------------------------------*/
/* Miscellaneous Functions                                               */
/*-----------------------------------------------------------------------*/

DRESULT disk_ioctl (
	BYTE pdrv,		/* Physical drive nmuber (0..) */
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
    if (pdrv != DEV_SD) return RES_PARERR;
    if (!sd_initialized) return RES_NOTRDY;

    switch (cmd) {
    case CTRL_SYNC:
        return RES_OK;

	case GET_SECTOR_COUNT:
    	if (buff == NULL) return RES_PARERR;
    	if (!sd_sector_count((uint32_t *)buff)) return RES_ERROR;
    	return RES_OK;

    case GET_SECTOR_SIZE:
        if (buff == NULL) return RES_PARERR;
        *(WORD*)buff = 512;
        return RES_OK;

    case GET_BLOCK_SIZE:
        if (buff == NULL) return RES_PARERR;
        *(DWORD*)buff = 1;
        return RES_OK;

    default:
        return RES_PARERR;
    }
}

