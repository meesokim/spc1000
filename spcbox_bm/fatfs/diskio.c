/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"			/* Obtains integer types */
#include "diskio.h"		/* Declarations of disk functions */

/* Definitions of physical drive number for each drive */
#define DEV_MMC 0		/* Example: Map MMC/SD card to physical drive 0 */
#define DEV_RAM 1		/* Example: Map Ramdisk to physical drive 1 */
#define DEV_USB		2	/* Example: Map USB MSD to physical drive 2 */


/*-----------------------------------------------------------------------*/
/* Get Drive Status                                                      */
/*-----------------------------------------------------------------------*/

DSTATUS disk_status (
	BYTE pdrv		/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;

	switch (pdrv) {
	case DEV_RAM :

		return RAM_disk_status();

	case DEV_MMC :

		return MMC_disk_status();

	case DEV_USB :

		return USB_disk_status();
	}
	return STA_NOINIT;
}



/*-----------------------------------------------------------------------*/
/* Inidialize a Drive                                                    */
/*-----------------------------------------------------------------------*/

DSTATUS disk_initialize (
	BYTE pdrv				/* Physical drive nmuber to identify the drive */
)
{
	DSTATUS stat;
	int result;
	switch (pdrv) {
	case DEV_RAM :

		return RAM_disk_initialize();

	case DEV_MMC :

		return MMC_disk_initialize();

	case DEV_USB :

		return USB_disk_initialize();

	}
	return STA_NOINIT;
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
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :

		return RAM_disk_read(buff, sector, count);

	case DEV_MMC :

		return MMC_disk_read(buff, sector, count);

	case DEV_USB :

		return USB_disk_read(buff, sector, count);

	}
	return RES_PARERR;
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
	DRESULT res;
	int result;

	switch (pdrv) {
	case DEV_RAM :

		return RAM_disk_write(buff, sector, count);

	case DEV_MMC :

		return MMC_disk_write(buff, sector, count);

	case DEV_USB :

		return USB_disk_write(buff, sector, count);

	}

	return RES_PARERR;
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
	DRESULT res = FR_DISK_ERR;
	int result;

	switch (pdrv) {
	case DEV_RAM :

		// Process of the command for the RAM drive

		return res;

	case DEV_MMC :

		return MMC_disk_ioctl(cmd, buff);

	case DEV_USB :
#ifdef HAVE_USPI
        int nDeviceIndex = pdrv - USB;
        if (nDeviceIndex < 0 || nDeviceIndex >= USPiMassStorageDeviceAvailable())
        {
            return RES_PARERR;
        }
        if (cmd == CTRL_SYNC)
        {
            return RES_OK;
        }
        if (cmd == GET_SECTOR_COUNT)
        {
            *(DWORD *)buff = USPiMassStorageDeviceGetCapacity(nDeviceIndex);
            return RES_OK;
        }
        if (cmd == GET_SECTOR_SIZE)
        {
            *(DWORD *)buff = USPI_BLOCK_SIZE;
            return RES_OK;
        }
        if (cmd == GET_BLOCK_SIZE)
        {
            *(DWORD *)buff = USPI_BLOCK_SIZE;
            return RES_OK;
        }
        return RES_PARERR;
#else
		return res;
#endif
	}

	return RES_PARERR;
}

