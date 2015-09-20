/**
 @file fatfs/mmc.c

 @brief MMC upper level interface to FatFS
   - Originally part of FatFs avr example project (C)ChaN, 2013.
   - Specifically: avr_complex/main.c from ffsample.zip.
   - Modifications by Mike Gore.
   - I added hardware abstraction layer or mmc.c to make porting easier.

 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par Copyright &copy; 2013 ChaN.
 @par Credit: part of FatFs avr example project (C)ChaN, 2013.
 @par You are free to use this code under the terms of GPL
   please retain a copy of this notice in any code you use it in.

This is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option)
any later version.

This software is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/



/*-----------------------------------------------------------------------*/
/* MMCv3/SDv1/SDv2 (in SPI mode) control module  (C)ChaN, 2010           */
/*-----------------------------------------------------------------------*/
/* mmc.c */

#include <user_config.h>
#include "mmc_hal.h"
#include "diskio.h"
#include "disk.h"
#include "mmc.h"


/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0    (0)          /*< GO_IDLE_STATE */
#define CMD1    (1)          /*< SEND_OP_COND (MMC) */
#define ACMD41  (0x80+41)    /*< SEND_OP_COND (SDC) */
#define CMD8    (8)          /*< SEND_IF_COND */
#define CMD9    (9)          /*< SEND_CSD */
#define CMD10   (10)         /*< SEND_CID */
#define CMD12   (12)         /*< STOP_TRANSMISSION */
#define ACMD13  (0x80+13)    /*< SD_STATUS (SDC) */
#define CMD16   (16)         /*< SET_BLOCKLEN */
#define CMD17   (17)         /*< READ_SINGLE_BLOCK */
#define CMD18   (18)         /*< READ_MULTIPLE_BLOCK */
#define CMD23   (23)         /*< SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0x80+23)    /*< SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)         /*< WRITE_BLOCK */
#define CMD25   (25)         /*< WRITE_MULTIPLE_BLOCK */
#define CMD32   (32)         /*< ERASE_ER_BLK_START */
#define CMD33   (33)         /*< ERASE_ER_BLK_END */
#define CMD38   (38)         /*< ERASE */
#define CMD55   (55)         /*< APP_CMD */
#define CMD58   (58)         /*< READ_OCR */

volatile
DSTATUS Stat = STA_NOINIT;   /* <Disk status */

static
BYTE CardType;               /*< Card type flags */


// ===========================================

MEMSPACE
int wait_ready (
UINT wt         /*< Timeout [ms] */
)
{
    mmc_set_ms_timeout(wt);  /* Wait for ready in timeout of 500ms */

    while(!mmc_test_timeout())
	{
		if( mmc_spi_TXRX(0xff) == 0xff)
			return 1;
	}
	return (0);
}


///@brief Deselect the card and release SPI bus

MEMSPACE
static
void mmc_deselect (void)
{
    mmc_cs_disable();
    mmc_spi_TX(0xFF);   /*< Dummy clock (force DO hi-z for multiple slave SPI) */
    mmc_spi_TX(0xFF);   /*< Dummy clock (force DO hi-z for multiple slave SPI) */
}


///@brief Select the card and wait for ready
///@return 1 Successful
///@return 0 Timeout
MEMSPACE
static
int mmc_select (void)
{
    mmc_cs_enable();
    mmc_spi_TX(0xFF);      /* Dummy clock (force DO enabled) */

    if (wait_ready(500)) 
		return 1;          /* OK */

printf("mmc_select failed!\n");

    mmc_deselect();
    return 0;                                     /* Timeout */
}


///@brief Receive a data packet from MMC.
///@return 1 Successful
///@return 0 Error

MEMSPACE
static
int rcvr_datablock (
BYTE *buff,     /*< Data buffer to store received data */
UINT btr        /*< Byte count (must be multiple of 4) */
)
{
    BYTE token;

    mmc_set_ms_timeout(400);
    do                                            /* Wait for data packet in timeout of 200ms */
    {
        token = mmc_spi_TXRX(0xFF);
    } while ((token == 0xFF) && !mmc_test_timeout());
    if (token != 0xFE) return 0;                  /* If not valid data token, retutn with error */

    mmc_spi_RX_buffer(buff, btr); /* Receive the data block into buffer */
    mmc_spi_TX(0xFF);                           /* Discard CRC */
    mmc_spi_TX(0xFF);

    return 1;                                     /* Return with success */
}


///@brief Send a data packet to MMC 
///@return 1 Successful
///@return 0 Error
#if _USE_WRITE
MEMSPACE
static
int xmit_datablock (
const BYTE *buff, /*< 512 byte data block to be transmitted */
BYTE token        /*< Data/Stop token */
)
{
    BYTE resp;

    if (!wait_ready(500)) return 0;

    mmc_spi_TX(token);                            /* Xmit data token */
    if (token != 0xFD)                            /* Is data token */
    {
        mmc_spi_TX_buffer(buff, 512);                /* Xmit the data block to the MMC */

        mmc_spi_TX(0xFF);                       /* CRC (Dummy) */
        mmc_spi_TX(0xFF);
        resp = mmc_spi_TXRX(0xFF);                /* Reveive data response */
        if ((resp & 0x1F) != 0x05)                /* If not accepted, return with error */
            return 0;
    }

    return 1;
}
#endif

///@brief  Send a command packet to MMC
///@return R1 resp 
///@return bit7==1Send failed

MEMSPACE
static
BYTE send_cmd (
BYTE cmd,     /*< Command index */
DWORD arg     /*< Argument */
)
{
    BYTE n, res;

    if (cmd & 0x80)                               /* ACMD<n> is the command sequense of CMD55-CMD<n> */
    {
        cmd &= 0x7F;
        res = send_cmd(CMD55, 0);
        if (res > 1) return res;
    }

/* Select the card and wait for ready except to stop multiple block read */
    if (cmd != CMD12)
    {
        mmc_deselect();
        if (!mmc_select()) return 0xFF;
    }

/* Send command packet */
    mmc_spi_TX(0x40 | cmd);                     /* Start + Command index */
    mmc_spi_TX((BYTE)(arg >> 24));              /* Argument[31..24] */
    mmc_spi_TX((BYTE)(arg >> 16));              /* Argument[23..16] */
    mmc_spi_TX((BYTE)(arg >> 8));               /* Argument[15..8] */
    mmc_spi_TX((BYTE)arg);                      /* Argument[7..0] */
    n = 0x01;                                     /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;                    /* Valid CRC for CMD0(0) + Stop */
    if (cmd == CMD8) n = 0x87;                    /* Valid CRC for CMD8(0x1AA) Stop */

    mmc_spi_TX(n);

/* Receive command response */
    if (cmd == CMD12) mmc_spi_TX(0xFF);         /* Skip a stuff byte when stop reading */
    n = 10;                                       /* Wait for a valid response in timeout of 10 attempts */
    do
	{
		res = mmc_spi_TXRX(0xFF);
	}
    while ((res & 0x80) && --n);

    return res;                                   /* Return with the response value */
}


///@brief Public Functions


///@brief Initialize Disk Drive
///@return Stat
MEMSPACE
DSTATUS mmc_disk_initialize (
BYTE pdrv   /*< Physical drive nmuber (0) */
)
{
    BYTE n, cmd, ty, ocr[4];

    if (pdrv) return STA_NOINIT;                  /* Supports only single drive */
    mmc_power_off();                              /* Turn off the sock et power to reset the card */
    if (Stat & STA_NODISK) return Stat;           /* No card in the socket */
    mmc_power_on();                               /* Turn on the socket power */
    mmc_slow();
    for (n = 10; n; n--) mmc_spi_TX(0xFF);      /* 80 dummy clocks */

    ty = 0;
    if (send_cmd(CMD0, 0) == 1)                   /* Enter Idle state */
    {
        mmc_set_ms_timeout(250);                 /* Initialization timeout of 1000 msec */
        if (send_cmd(CMD8, 0x1AA) == 1)           /* SDv2? */
        {
            for (n = 0; n < 4; n++)
			{
                ocr[n] = mmc_spi_TXRX(0xFF);      /* Get trailing return value of R7 resp */
			}
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) /* The card can work at vdd range of 2.7-3.6V */
            {
/* Wait for leaving idle state (ACMD41 with HCS bit) */
                while (!mmc_test_timeout() && send_cmd(ACMD41, 1UL << 30))
				{
				}
/* Check CCS bit in the OCR */
                if (!mmc_test_timeout() && send_cmd(CMD58, 0) == 0)
                {
                    for (n = 0; n < 4; n++) ocr[n] = mmc_spi_TXRX(0xFF);
/* SDv2 */
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        }
        else
        {
/* SDv1 or M
MCv3 */
            if (send_cmd(ACMD41, 0) <= 1)
            {
                ty = CT_SD1; cmd = ACMD41;        /* SDv1 */
            }
            else
            {
                ty = CT_MMC; cmd = CMD1;          /* MMCv3 */
            }
/* Wait for leaving idle state */
            while (!mmc_test_timeout() && send_cmd(cmd, 0));
/* Set R/W block length to 512 */
            if (mmc_test_timeout() || send_cmd(CMD16, 512) != 0)
                ty = 0;
        }
    }

    CardType = ty;
    mmc_deselect();

    if (ty)                                       /* Initialization succeded */
    {
        Stat &= ~STA_NOINIT;                      /* Clear STA_NOINIT */
        mmc_fast();
    }                                             /* Initialization failed */
    else
    {
        mmc_power_off();
    }

    if (Stat & STA_NODISK) return Stat;           /* No card in the socket */

    return Stat;
}


/// @brief Get Disk Status
/// @return Stat
/// @return STA_NOINIT if no drive

MEMSPACE
DSTATUS mmc_disk_status (
BYTE pdrv  /*< Physical drive nmuber (0) */
)
{
    if (pdrv) return STA_NOINIT;                  /* Supports only single drive */
    return Stat;
}


///@brief Read Sector(s)
/// @return 0 ok
/// @return non zero error

MEMSPACE
DRESULT mmc_disk_read (
BYTE pdrv,     /*< Physical drive nmuber (0) */
BYTE *buff,    /*< Pointer to the data buffer to store read data */
DWORD sector,  /*< Start sector number (LBA) */
UINT count     /*< Sector count (1..128) */
)
{
    BYTE cmd;

    if (pdrv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;

    if (!(CardType & CT_BLOCK)) sector *= 512;
/* Convert to byte address if needed
 */

    cmd = count > 1 ? CMD18 : CMD17;
/*  READ_MULTIPLE_BLOCK : RE
AD_SINGLE_BLOCK */
    if (send_cmd(cmd, sector) == 0)
    {
        do
        {
            if (!rcvr_datablock(buff, 512)) 
				break;
            buff += 512;
        } while (--count);
        if (cmd == CMD18) send_cmd(CMD12, 0);     /* STOP_TRANSMISSION */
    }
    mmc_deselect();

    return count ? RES_ERROR : RES_OK;
}


///@brief Write Sector(s)
/// @return 0 ok
/// @return non zero error

#if _USE_WRITE
MEMSPACE
DRESULT mmc_disk_write (
BYTE pdrv,        /*< Physical drive nmuber (0) */
const BYTE *buff, /*< Pointer to the data to be written */
DWORD sector,     /*< Start sector number (LBA) */
UINT count                                        /* Sector count (1..128) */
)
{
    if (pdrv || !count) return RES_PARERR;
    if (Stat & STA_NOINIT) return RES_NOTRDY;
    if (Stat & STA_PROTECT) return RES_WRPRT;

    if (!(CardType & CT_BLOCK)) sector *= 512;
/* Convert to byte address if needed
 */

    if (count == 1)                               /* Single block write */
    {
        if ((send_cmd(CMD24, sector) == 0)        /* WRITE_BLOCK */
            && xmit_datablock(buff, 0xFE))
            count = 0;
    }
    else                                          /* Multiple block write */
    {
        if (CardType & CT_SDC) send_cmd(ACMD23, count);
        if (send_cmd(CMD25, sector) == 0)         /* WRITE_MULTIPLE_BLOCK */
        {
            do
            {
                if (!xmit_datablock(buff, 0xFC)) break;
                buff += 512;
            } while (--count);
            if (!xmit_datablock(0, 0xFD))         /* STOP_TRAN token */
                count = 1;
        }
    }
    mmc_deselect();

    return count ? RES_ERROR : RES_OK;
}
#endif

///@brief Miscellaneous Functions

#if _USE_IOCTL
///@brief Disk IOCTL
///
/// - Extended disk access functions.
/// @return 0 ok
/// @return non zero error

MEMSPACE
DRESULT mmc_disk_ioctl (
BYTE pdrv,                                        /* Physical drive nmuber (0) */
BYTE cmd,                                         /* Control code */
void *buff                                        /* Buffer to send/receive control data */
)
{
    DRESULT res;
    BYTE n, csd[16], *ptr = buff;
    DWORD csize;

    if (pdrv) return RES_PARERR;

    res = RES_ERROR;

    if (Stat & STA_NOINIT) return RES_NOTRDY;

    switch (cmd)
    {
        case CTRL_SYNC :     /* Make sure that no pending write process. Do not remove this or written sector might not left updated. */
            if (mmc_select()) res = RES_OK;
            break;

        case GET_SECTOR_COUNT :                   /* Get number of sectors on the disk (DWORD) */
            if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
            {
                if ((csd[0] >> 6) == 1)           /* SDC ver 2.00 */
                {
                    csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
                    *(DWORD*)buff = csize << 10;
                }                                 /* SDC ver 1.XX or M MC*/
                else
                {
                    n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
                    csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
                    *(DWORD*)buff = csize << (n - 9);
                }
                res = RES_OK;
            }
            break;

        case GET_BLOCK_SIZE :                     /* Get erase block size in unit of sector (DWORD) */
            if (CardType & CT_SD2)                /* SDv2? */
            {
                if (send_cmd(ACMD13, 0) == 0)     /* Read SD status */
                {
                    mmc_spi_TX(0xFF);
                    if (rcvr_datablock(csd, 16))  /* Read partial block */
                    {
/* Purge trailing data */
                        for (n = 64 - 16; n; n--) mmc_spi_TX(0xFF);
                        *(DWORD*)buff = 16UL << (csd[10] >> 4);
                        res = RES_OK;
                    }
                }
            }                                     /* SDv1 or MMCv3 */
            else
            {
/* Read CSD */
                if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16))
                {
                    if (CardType & CT_SD1)        /* SDv1 */
                    {
                        *(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
                    }                             /* MMCv3 */
                    else
                    {
                        *(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
                    }
                    res = RES_OK;
                }
            }
            break;

/* Following commands are never used by FatFs module */

        case MMC_GET_TYPE :                       /* Get card type flags (1 byte) */
            *ptr = CardType;
            res = RES_OK;
            break;

        case MMC_GET_CSD :                        /* Receive CSD as a data block (16 bytes) */
            if (send_cmd(CMD9, 0) == 0            /* READ_CSD */
                && rcvr_datablock(ptr, 16))
                res = RES_OK;
            break;

        case MMC_GET_CID :                        /* Receive CID as a data block (16 bytes) */
            if (send_cmd(CMD10, 0) == 0           /* READ_CID */
                && rcvr_datablock(ptr, 16))
                res = RES_OK;
            break;

        case MMC_GET_OCR :                        /* Receive OCR as an R3 resp (4 bytes) */
            if (send_cmd(CMD58, 0) == 0)          /* READ_OCR */
            {
                for (n = 4; n; n--) *ptr++ = mmc_spi_TXRX(0xFF);
                res = RES_OK;
            }
            break;

        case MMC_GET_SDSTAT :                     /* Receive SD statsu as a data block (64 bytes) */
            if (send_cmd(ACMD13, 0) == 0)         /* SD_STATUS */
            {
                mmc_spi_TX(0xFF);
                if (rcvr_datablock(ptr, 64))
                    res = RES_OK;
            }
            break;

        case CTRL_POWER_OFF :                     /* Power off */
            mmc_power_off();
            Stat |= STA_NOINIT;
            break;

        default:
            res = RES_PARERR;
    }

    mmc_deselect();

    return res;
}
#endif
