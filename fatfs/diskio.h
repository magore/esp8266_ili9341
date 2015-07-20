/**
 @file fatfs/diskio.h

 @brief FafFs disk interface modlue include file   (C) ChaN, 2013
  - Minor documenation updates by Mike Gore 2014

 @par Copyright &copy; ChaN 2014.

*/

/*-----------------------------------------------------------------------/
/  Low level disk interface modlue include file   (C)ChaN, 2013          /
/
/-----------------------------------------------------------------------*/
///@brief Formating changes for Doxygen by Mike Gore 2014

#ifndef _DISKIO_DEFINED
#define _DISKIO_DEFINED

#ifdef __cplusplus
extern "C"
{
#endif

#define _USE_WRITE  1  /*< 1: Enable disk_write function */
#define _USE_IOCTL  1  /*< 1: Enable disk_ioctl fucntion */

#include "integer.h"

/** @brief Status of Disk Functions */
    typedef BYTE    DSTATUS;

/** @brief  Results of Disk Functions */
    typedef enum disk_function_results
    {
        RES_OK = 0,  /*< 0: Successful */
        RES_ERROR,   /*< 1: R/W Error */
        RES_WRPRT,   /*< 2: Write Protected */
        RES_NOTRDY,  /*< 3: Not Ready */
        RES_PARERR   /*< 4: Invalid Parameter */
    } DRESULT;

/*---------------------------------------*/
/* Prototypes for disk control functions */

    DSTATUS disk_initialize (BYTE pdrv);
    DSTATUS disk_status (BYTE pdrv);
    DRESULT disk_read (BYTE pdrv, BYTE* buff, DWORD sector, UINT count);
#if _USE_WRITE
    DRESULT disk_write (BYTE pdrv, const BYTE* buff, DWORD sector, UINT count);
#endif
#if _USE_IOCTL
    DRESULT disk_ioctl (BYTE pdrv, BYTE cmd, void* buff);
#endif
    void disk_timerproc (void);

/* Disk Status Bits (DSTATUS) */

#define STA_NOINIT      0x01  /*< Drive not initialized */
#define STA_NODISK      0x02  /*< No medium in the drive */
#define STA_PROTECT     0x04  /*< Write protected */

/* Command code for disk_ioctrl fucntion */

/** @brief Generic command (used by FatFs) */
#define CTRL_SYNC           0 /*< Flush disk cache (for write functions)
 */
#define GET_SECTOR_COUNT    1 /*< Get media size (for only f_mkfs()) */ 
#define GET_SECTOR_SIZE     2 /*< Get sector size (for multiple sector size (_MA
X_SS >= 1024)) */
#define GET_BLOCK_SIZE      3 /*< Get erase block size (for only f_mkfs()) */
#define CTRL_ERASE_SECTOR   4 /*< Force erased a block of sectors (for only _USE
_ERASE) */

/** brief Generic command (not used by FatFs) */
#define CTRL_ERASE_SECTOR   4  /*< Force erased a block of sectors (for only _USE_ERASE) */
#define CTRL_FORMAT         5  /*< Create physical format on the media */
#define CTRL_POWER_IDLE     6  /*< Put the device idle state */
#define CTRL_POWER_OFF      7  /*< Put the device off state */
#define CTRL_LOCK           8  /*< Lock media removal */
#define CTRL_UNLOCK         9  /*< Unlock media removal */
#define CTRL_EJECT          10 /*< Eject media */

/** @brief MMC/SDC specific ioctl command */
#define MMC_GET_TYPE        50 /*< Get card type */
#define MMC_GET_CSD         51 /*< Get CSD */
#define MMC_GET_CID         52 /*< Get CID */
#define MMC_GET_OCR         53 /*< Get OCR */
#define MMC_GET_SDSTAT      54 /*< Get SD status */

/*@brief ATA/CF specific ioctl command */
#define ATA_GET_REV         60 /*< Get F/W revision */
#define ATA_GET_MODEL       61 /*< Get model name */
#define ATA_GET_SN          62 /*< Get serial number */

/*@brief MMC card type flags (MMC_GET_TYPE) */
#define CT_MMC      		0x01             /*< MMC ver 3 */
#define CT_SD1      		0x02             /*< SD ver 1 */
#define CT_SD2      		0x04             /*< SD ver 2 */
#define CT_SDC      		(CT_SD1|CT_SD2)  /*< SD */
#define CT_BLOCK    		0x08             /*< Block addressing */

#ifdef __cplusplus
}
#endif
#endif
