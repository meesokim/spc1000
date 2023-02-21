/*-----------------------------------------------------------------------*/
/* Low level disk I/O module SKELETON for FatFs     (C)ChaN, 2019        */
/*-----------------------------------------------------------------------*/
/* If a working storage control module is available, it should be        */
/* attached to the FatFs via a glue function rather than modifying it.   */
/* This is an example of glue functions to attach various exsisting      */
/* storage control modules to the FatFs module with a defined API.       */
/*-----------------------------------------------------------------------*/

#include "ff.h"     /* Obtains integer types */
#include "diskio.h" /* Declarations of disk functions */
#include "emmc.h"

/* Definitions of physical drive number for each drive */
#define DEV_RAM 0 /* Example: Map Ramdisk to physical drive 0 */
#define DEV_MMC 1 /* Example: Map MMC/SD card to physical drive 1 */
#define DEV_USB 2 /* Example: Map USB MSD to physical drive 2 */

static struct emmc_block_dev *emmc_dev;
int MMC_disk_status() {
    if (emmc_dev == NULL)
    {
        return STA_NOINIT;
    }
    return 0;
}

DSTATUS MMC_disk_initialize()
{
    if (emmc_dev == NULL)
    {
        if (sd_card_init((struct block_device **)&emmc_dev) != 0)
        {
            return STA_NOINIT;
        }
    }
    return 0;
}

DRESULT MMC_disk_read(BYTE *buff, LBA_t sector, UINT count) {
    size_t buf_size = count * emmc_dev->bd.block_size;
    size_t len = sd_read(buff, buf_size, sector);
    // printf("sd_read:bufsize=%d,sector=%d,ret=%d\n", buf_size, sector, len);
    if (len < buf_size)
    {
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT MMC_disk_write(BYTE *buff, LBA_t sector, UINT count)
{
    size_t buf_size = count * emmc_dev->bd.block_size;
    if (sd_write(buff, buf_size, sector) < buf_size)
    {
        return RES_ERROR;
    }
    return RES_OK;
}

DRESULT MMC_disk_ioctl(BYTE cmd, void *buff) {
    if (cmd == CTRL_SYNC)
    {
        return RES_OK;
    }
    if (cmd == GET_SECTOR_COUNT)
    {
        *(DWORD *)buff = emmc_dev->bd.num_blocks;
        return RES_OK;
    }
    if (cmd == GET_SECTOR_SIZE)
    {
        *(DWORD *)buff = emmc_dev->bd.block_size;
        return RES_OK;
    }
    if (cmd == GET_BLOCK_SIZE)
    {
        *(DWORD *)buff = emmc_dev->bd.block_size;
        return RES_OK;
    }
    return RES_PARERR;
}

int RAM_disk_status()
{
    return STA_NOINIT;
}

DSTATUS RAM_disk_initialize() {
    return STA_NOINIT;
}

DRESULT RAM_disk_read(BYTE *buff, LBA_t sector, UINT count) {
    return RES_ERROR;
}

DRESULT RAM_disk_write(BYTE *buff, LBA_t sector, UINT count)
{
    return RES_ERROR;
}

int USB_disk_status() {
#ifdef HAVE_USPI
    int nDeviceIndex = pdrv - USB;
    if (nDeviceIndex < 0 || nDeviceIndex >= USPiMassStorageDeviceAvailable())
    {
        return STA_NODISK;
    }
    return 0;
#else
    return STA_NODISK;
#endif
}

DSTATUS USB_disk_initialize()
{
#ifdef HAVE_USPI
    int nDeviceIndex = pdrv - USB;
    if (nDeviceIndex < 0 || nDeviceIndex >= USPiMassStorageDeviceAvailable())
    {
        return STA_NODISK;
    }
    return 0;
#else
    return STA_NODISK;
#endif
}

DRESULT USB_disk_read(BYTE * buff, LBA_t sector, UINT count)
{
#ifdef HAVE_USPI
    // int nDeviceIndex = pdrv - USB;
    // if (nDeviceIndex < 0 || nDeviceIndex >= USPiMassStorageDeviceAvailable())
    // {
    //     return RES_PARERR;
    // }
    unsigned buf_size = count * USPI_BLOCK_SIZE;
    if (USPiMassStorageDeviceRead(sector * USPI_BLOCK_SIZE, buff, buf_size, 0) < buf_size)
    {
        return RES_ERROR;
    }
    return RES_OK;
#else
    return RES_ERROR;
#endif 
}

DRESULT USB_disk_write(BYTE * buff, LBA_t sector, UINT count)
{
#ifdef HAVE_USPI
    // int nDeviceIndex = pdrv - USB;
    // if (nDeviceIndex < 0 || nDeviceIndex >= USPiMassStorageDeviceAvailable())
    // {
    //     return RES_PARERR;
    // }
    unsigned buf_size = count * USPI_BLOCK_SIZE;
    if (USPiMassStorageDeviceWrite(sector * USPI_BLOCK_SIZE, buff, buf_size, 0) < buf_size)
    {
        return RES_ERROR;
    }
    return RES_OK;
#else
    return RES_ERROR;
#endif
}