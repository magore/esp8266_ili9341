/**
 @file fatfs/mmc.c

 @brief MMC upper level interface to FatFS
   - From FatFs avr example project (C)ChaN, 2013-2016.
      - Specifically: avr/mmc.c from ffsample.zip.
   - Minor changes by Mike Gore.
*/
/*-----------------------------------------------------------------------*/
/* MMCv3/SDv1/SDv2 Controls via AVR SPI module                           */
/*-----------------------------------------------------------------------*/
/*
/  Copyright (C) 2016, ChaN, all right reserved.
/
/ * This software is a free software and there is NO WARRANTY.
/ * No restriction on use. You can use, modify and redistribute it for
/   any purpose as you like UNDER YOUR RESPONSIBILITY.
/ * Redistributions of source code must retain the above copyright notice.
/
/-------------------------------------------------------------------------*/

/* mmc.c */
#include "user_config.h"
#include "mmc_hal.h"

#ifdef AVR
#include <stdlib.h>
#endif

#include "printf/mathio.h"

#ifdef ESP8266
#include "esp8266/hspi.h"
#endif

#include "fatfs.sup/fatfs.h"

/* Peripheral controls (Platform dependent) */
#define CS_LOW()        mmc_spi_begin()  /* Set MMC_CS = low */
#define CS_HIGH()       mmc_spi_end()     /* Set MMC_CS = high */

#define MMC_CD          mmc_ins_status() /* Test if card detected.   yes:true, no:false, default:true */
#define MMC_WP          mmc_wp_status()  /* Test if write protected. yes:true, no:false, default:false */
#define FCLK_SLOW()     mmc_slow()      /* Set SPI slow clock (100-400kHz) */
#define FCLK_FAST()     mmc_fast()      /* Set SPI fast clock (20MHz max) */

/*--------------------------------------------------------------------------

   Module Private Functions

---------------------------------------------------------------------------*/

/* Definitions for MMC/SDC command */
#define CMD0    (0)         /* GO_IDLE_STATE */
#define CMD1    (1)         /* SEND_OP_COND (MMC) */
#define ACMD41  (0x80+41)   /* SEND_OP_COND (SDC) */
#define CMD8    (8)         /* SEND_IF_COND */
#define CMD9    (9)         /* SEND_CSD */
#define CMD10   (10)        /* SEND_CID */
#define CMD12   (12)        /* STOP_TRANSMISSION */
#define ACMD13  (0x80+13)   /* SD_STATUS (SDC) */
#define CMD16   (16)        /* SET_BLOCKLEN */
#define CMD17   (17)        /* READ_SINGLE_BLOCK */
#define CMD18   (18)        /* READ_MULTIPLE_BLOCK */
#define CMD23   (23)        /* SET_BLOCK_COUNT (MMC) */
#define ACMD23  (0x80+23)   /* SET_WR_BLK_ERASE_COUNT (SDC) */
#define CMD24   (24)        /* WRITE_BLOCK */
#define CMD25   (25)        /* WRITE_MULTIPLE_BLOCK */
#define CMD32   (32)        /* ERASE_ER_BLK_START */
#define CMD33   (33)        /* ERASE_ER_BLK_END */
#define CMD38   (38)        /* ERASE */
#define CMD48   (48)        /* READ_EXTR_SINGLE */
#define CMD49   (49)        /* WRITE_EXTR_SINGLE */
#define CMD55   (55)        /* APP_CMD */
#define CMD58   (58)        /* READ_OCR */


volatile
DSTATUS Stat = STA_NOINIT;   /* <Disk status */

static volatile
BYTE Timer1, Timer2;    	/* 100Hz decrement timer */

static
BYTE CardType;               /*< Card type flags */

/*-----------------------------------------------------------------------*/
/* Power Control  (Platform dependent)                                   */
/*-----------------------------------------------------------------------*/
/* When the target system does not support socket power control, there   */
/* is nothing to do in these functions and chk_power always returns 1.   */

///@brief power on
///@return void
MEMSPACE
static
void power_on (void)
{
    /* Turn socket power on and wait for 10ms+ (nothing to do if no power controls) */
    /* Configure MOSI/MISO/SCLK/CS pins */
    /* Enable SPI module in SPI mode 0 */
    mmc_power_on();
}


///@brief power off
///@return void
MEMSPACE
static
void power_off (void)
{
    /* Disable SPI function */
    /* De-configure MOSI/MISO/SCLK/CS pins (set hi-z) */
    /* Turn socket power off (nothing to do if no power controls) */
	mmc_power_off();
}

/*-----------------------------------------------------------------------*/
/* Transmit/Receive data from/to MMC via SPI  (Platform dependent)       */
/*-----------------------------------------------------------------------*/

///@brief send/receive a SPI byte
///@param [in] dat: data to send
///@return Data read
static
BYTE xchg_spi (     /* Returns received data */
    BYTE dat        /* Data to be sent */
)
{
    dat = mmc_spi_TXRX(dat);
    return dat;
}

///@brief Receive a data block fast 
///@param [in] p: Data block to be read
///@param [in]  cnt: Bytes to read
///@return void
static
void rcvr_spi_multi (
    BYTE *p,    /* Data read buffer */
    UINT cnt    /* Size of data block */
)
{
	mmc_spi_RX_buffer((uint8_t *)p, cnt);
}

///@brief Send a data block fast
///@param [out] p: Data block to be sent
///@param [in]  cnt: Bytes to send
///@return void
static
void xmit_spi_multi (
    const BYTE *p,  /* Data block to be sent */
    UINT cnt        /* Size of data block */
)
{
	mmc_spi_TX_buffer((uint8_t *)p, cnt);
}




// =============================================
///@brief wait for card ready
///@param [in] wt: ms to wait
///@return 1 Ready
///@return 0 Timeout
MEMSPACE
int wait_ready (
UINT wt         /*< Timeout [ms] */
)
{
    BYTE d;
	// Timer2 = wt / 10;
	mmc_set_ms_timeout(wt);
    do
        d = xchg_spi(0xFF);
    while (d != 0xFF && !mmc_test_timeout());
    //while (d != 0xFF && Timer2);

    return (d == 0xFF) ? 1 : 0;
}


///@brief Deselect the card and release SPI bus
///@return void
MEMSPACE
static
void deselect (void)
{
    CS_HIGH();
    xchg_spi(0xFF);   /*< Dummy clock (force DO hi-z for multiple slave SPI) */
    xchg_spi(0xFF);   /*< Dummy clock (force DO hi-z for multiple slave SPI) */
}


///@brief Select the card and wait for ready
///@return 1 Successful
///@return 0 Timeout
MEMSPACE
static
int select (void)
{
    CS_LOW();
    xchg_spi(0xFF);      /* Dummy clock (force DO enabled) */

    if (wait_ready(1000)) 
		return 1;          /* OK */

	printf("select failed!\n");

    deselect();
    return 0;                                     /* Timeout */
}


///@brief Receive a data packet from MMC.
///@param [in] buff: Data buffer to read data into
///@param [in] btr:  Bytes to read
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

	//Timer1 = 40; 
    mmc_set_ms_timeout(1000);
    do                                            /* Wait for data packet in timeout of 400ms */
    {
        token = xchg_spi(0xFF);
    } while ((token == 0xFF) && !mmc_test_timeout());
    //while ((token == 0xFF) && Timer1);
    if (token != 0xFE) return 0;                  /* If not valid data token, retutn with error */

    rcvr_spi_multi(buff, btr); /* Receive the data block into buffer */
    xchg_spi(0xFF);                           /* Discard CRC */
    xchg_spi(0xFF);

    return 1;                                     /* Return with success */
}


///@brief Send a data packet to MMC 
///@param [in] buff: Data buffer to write
///@param [in] btr:  Bytes to write
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

    if (!wait_ready(1000)) return 0;

    xchg_spi(token);                            /* Xmit data token */
    if (token != 0xFD)                            /* Is data token */
    {
        xmit_spi_multi(buff, 512);                /* Xmit the data block to the MMC */

        xchg_spi(0xFF);                       /* CRC (Dummy) */
        xchg_spi(0xFF);
        resp = xchg_spi(0xFF);                /* Reveive data response */
        if ((resp & 0x1F) != 0x05)                /* If not accepted, return with error */
            return 0;
    }

    return 1;
}
#endif // ifdef _USE_WRITE

///@brief  Send a command packet to MMC
///@param [in] send_cmd: Commend Index
///@param [in] cmd:      Argument
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
        deselect();
        if (!select()) return 0xFF;
    }

/* Send command packet */
    xchg_spi(0x40 | cmd);                     /* Start + Command index */
    xchg_spi((BYTE)(arg >> 24));              /* Argument[31..24] */
    xchg_spi((BYTE)(arg >> 16));              /* Argument[23..16] */
    xchg_spi((BYTE)(arg >> 8));               /* Argument[15..8] */
    xchg_spi((BYTE)arg);                      /* Argument[7..0] */
    n = 0x01;                                     /* Dummy CRC + Stop */
    if (cmd == CMD0) n = 0x95;                    /* Valid CRC for CMD0(0) + Stop */
    if (cmd == CMD8) n = 0x87;                    /* Valid CRC for CMD8(0x1AA) Stop */

    xchg_spi(n);

/* Receive command response */
    if (cmd == CMD12) xchg_spi(0xFF);         /* Skip a stuff byte when stop reading */
    n = 10;                                       /* Wait for a valid response in timeout of 10 attempts */
    do
	{
		res = xchg_spi(0xFF);
	}
    while ((res & 0x80) && --n);

    return res;                                   /* Return with the response value */
}


///@brief Public Functions


///@brief Initialize Disk Drive
///@return Stat
MEMSPACE
DSTATUS mmc_disk_initialize (void) 
{
    BYTE n, cmd, ty, ocr[4];

    //for (Timer1 = 10; Timer1; ) ;       	  	/* Wait for 100ms */
    if (Stat & STA_NODISK) return Stat;       	/* No card in the socket */

    FCLK_SLOW();

    for (n = 10; n; n--) xchg_spi(0xFF); 		/* 80 dummy clocks */

    ty = 0;
    if (send_cmd(CMD0, 0) == 1)                	/* Enter Idle state */
    {
        //Timer1=100;			 					/* Initialization timeout of 1000 msec */
        mmc_set_ms_timeout(2000);                   /* Initialization timeout of 1000 msec */
        if (send_cmd(CMD8, 0x1AA) == 1)           /* SDv2? */
        {
            for (n = 0; n < 4; n++)
                ocr[n] = xchg_spi(0xFF);      /* Get trailing return value of R7 resp */
            if (ocr[2] == 0x01 && ocr[3] == 0xAA) /* The card can work at vdd range of 2.7-3.6V */
            {
				/* Wait for leaving idle state (ACMD41 with HCS bit) */
                //while (Timer1 && send_cmd(ACMD41, 1UL << 30))
                while (!mmc_test_timeout() && send_cmd(ACMD41, 1UL << 30))
					;
				/* Check CCS bit in the OCR */
                //if (Timer1 && send_cmd(CMD58, 0) == 0)
                if (!mmc_test_timeout() && send_cmd(CMD58, 0) == 0)
                {
                    for (n = 0; n < 4; n++) 
						ocr[n] = xchg_spi(0xFF);
					/* Check if the card is SDv2 */
                    ty = (ocr[0] & 0x40) ? CT_SD2 | CT_BLOCK : CT_SD2;
                }
            }
        }
        else
        { 	/* SDv1 or MMCv3 */
			mmc_set_ms_timeout(2000);                   /* Initialization timeout of 1000 msec */
            if (send_cmd(ACMD41, 0) <= 1)
            {
                ty = CT_SD1; cmd = ACMD41;        /* SDv1 */
            }
            else
            {
                ty = CT_MMC; cmd = CMD1;          /* MMCv3 */
            }
			/* Wait for leaving idle state */
            //while (Timer1 && send_cmd(cmd, 0))
            while (!mmc_test_timeout() && send_cmd(cmd, 0))
				;
			/* Set R/W block length to 512 */
            //if (!Timer1 || send_cmd(CMD16, 512) != 0)
            if (mmc_test_timeout() || send_cmd(CMD16, 512) != 0)
                ty = 0;
        }
    }

    CardType = ty;
    deselect();

    if (ty)                                       /* Initialization succeded */
    {
        Stat &= ~STA_NOINIT;                      /* Clear STA_NOINIT */
        FCLK_FAST();
    }                                             /* Initialization failed */
    else
    {
        power_off();
    }

    return Stat;
}


/// @brief Get Disk Status
/// @return Stat
/// @return STA_NOINIT if no drive
MEMSPACE
DSTATUS mmc_disk_status ( void )
{
    return Stat;
}


///@brief Read Sector(s)
///@param [in] buff:   read buffer
///@param [in] sector: start sector number
///@param [in] count:  sector count
/// @return 0 ok
/// @return non zero error
MEMSPACE
DRESULT mmc_disk_read (
BYTE *buff,    /*< Pointer to the data buffer to store read data */
DWORD sector,  /*< Start sector number (LBA) */
UINT count     /*< Sector count (1..128) */
)
{
    BYTE cmd;

    if (!count) 
	{
		deselect();
		return RES_PARERR;
	}
    if (Stat & STA_NOINIT) 
	{
		deselect();
		return RES_NOTRDY;
	}

    if (!(CardType & CT_BLOCK)) sector *= 512;	/* Convert to byte address if needed */

    cmd = count > 1 ? CMD18 : CMD17; 			/*  READ_MULTIPLE_BLOCK : READ_SINGLE_BLOCK */
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
    deselect();

    return count ? RES_ERROR : RES_OK;
}


///@brief Write Sector(s)
///@param [out] buff:  write buffer
///@param [in] sector: start sector number
///@param [in] count:  sector count
/// @return 0 ok
/// @return non zero error
#if _USE_WRITE
MEMSPACE
DRESULT mmc_disk_write (
const BYTE *buff, /*< Pointer to the data to be written */
DWORD sector,     /*< Start sector number (LBA) */
UINT count                                        /* Sector count (1..128) */
)
{
    if (!count) 
	{
		deselect();
		return RES_PARERR;
	}
    if (Stat & STA_NOINIT) 
	{
		deselect();
		return RES_NOTRDY;
	}
    if (Stat & STA_PROTECT) 
	{
		deselect();
		return RES_WRPRT;
	}

    if (!(CardType & CT_BLOCK)) sector *= 512; /* Convert to byte address if needed */

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
    deselect();

    return count ? RES_ERROR : RES_OK;
}
#endif

///@brief Miscellaneous Functions
///@param [in] cmd:    Control code
///@param [in|out] buff:  Send/Receive buffer
///@return result
#if _USE_IOCTL
MEMSPACE
DRESULT mmc_disk_ioctl (
	BYTE cmd,		/* Control code */
	void *buff		/* Buffer to send/receive control data */
)
{
	DRESULT res = RES_NOTRDY;
	BYTE n, csd[16], *ptr = buff;
	DWORD *dp, st, ed, csize;
#if _USE_ISDIO
	SDIO_CTRL *sdi;
	BYTE rc, *bp;
	UINT dc;
#endif

	if (Stat & STA_NOINIT) 
	{
		return RES_NOTRDY;
	}

	res = RES_ERROR;
	switch (cmd) {
	case CTRL_SYNC :		/* Make sure that no pending write process. Do not remove this or written sector might not left updated. */
		if (select()) res = RES_OK;
//MG
deselect();
		break;

	case GET_SECTOR_COUNT :	/* Get number of sectors on the disk (DWORD) */
		if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {
			if ((csd[0] >> 6) == 1) {	/* SDC ver 2.00 */
				csize = csd[9] + ((WORD)csd[8] << 8) + ((DWORD)(csd[7] & 63) << 16) + 1;
				*(DWORD*)buff = csize << 10;
			} else {					/* SDC ver 1.XX or MMC*/
				n = (csd[5] & 15) + ((csd[10] & 128) >> 7) + ((csd[9] & 3) << 1) + 2;
				csize = (csd[8] >> 6) + ((WORD)csd[7] << 2) + ((WORD)(csd[6] & 3) << 10) + 1;
				*(DWORD*)buff = csize << (n - 9);
			}
			res = RES_OK;
		}
		deselect();
		break;

	case GET_BLOCK_SIZE :	/* Get erase block size in unit of sector (DWORD) */
		if (CardType & CT_SD2) {	/* SDv2? */
			if (send_cmd(ACMD13, 0) == 0) {	/* Read SD status */
				xchg_spi(0xFF);
				if (rcvr_datablock(csd, 16)) {				/* Read partial block */
					for (n = 64 - 16; n; n--) xchg_spi(0xFF);	/* Purge trailing data */
					*(DWORD*)buff = 16UL << (csd[10] >> 4);
					res = RES_OK;
				}
			}
		} else {					/* SDv1 or MMCv3 */
			if ((send_cmd(CMD9, 0) == 0) && rcvr_datablock(csd, 16)) {	/* Read CSD */
				if (CardType & CT_SD1) {	/* SDv1 */
					*(DWORD*)buff = (((csd[10] & 63) << 1) + ((WORD)(csd[11] & 128) >> 7) + 1) << ((csd[13] >> 6) - 1);
				} else {					/* MMCv3 */
					*(DWORD*)buff = ((WORD)((csd[10] & 124) >> 2) + 1) * (((csd[11] & 3) << 3) + ((csd[11] & 224) >> 5) + 1);
				}
				res = RES_OK;
			}
		}
		deselect();
		break;

	case CTRL_TRIM:		/* Erase a block of sectors (used when _USE_TRIM in ffconf.h is 1) */
		if (!(CardType & CT_SDC)) break;				/* Check if the card is SDC */
		if (mmc_disk_ioctl(MMC_GET_CSD, csd)) break;	/* Get CSD */
		if (!(csd[0] >> 6) && !(csd[10] & 0x40)) break;	/* Check if sector erase can be applied to the card */
		dp = buff; st = dp[0]; ed = dp[1];				/* Load sector block */
		if (!(CardType & CT_BLOCK)) {
			st *= 512; ed *= 512;
		}
		if (send_cmd(CMD32, st) == 0 && send_cmd(CMD33, ed) == 0 && send_cmd(CMD38, 0) == 0 && wait_ready(30000))	/* Erase sector block */
			res = RES_OK;	/* FatFs does not check result of this command */
//MG
deselect();
		break;

	/* Following commands are never used by FatFs module */

	case MMC_GET_TYPE :		/* Get card type flags (1 byte) */
		*ptr = CardType;
		res = RES_OK;
		break;

	case MMC_GET_CSD :		/* Receive CSD as a data block (16 bytes) */
		if (send_cmd(CMD9, 0) == 0 && rcvr_datablock(ptr, 16))		/* READ_CSD */
			res = RES_OK;
		deselect();
		break;

	case MMC_GET_CID :		/* Receive CID as a data block (16 bytes) */
		if (send_cmd(CMD10, 0) == 0 && rcvr_datablock(ptr, 16))		/* READ_CID */
			
			res = RES_OK;
		deselect();
		break;

	case MMC_GET_OCR :		/* Receive OCR as an R3 resp (4 bytes) */
		if (send_cmd(CMD58, 0) == 0) {	/* READ_OCR */
			for (n = 4; n; n--) *ptr++ = xchg_spi(0xFF);
			res = RES_OK;
		}
		deselect();
		break;

	case MMC_GET_SDSTAT :	/* Receive SD statsu as a data block (64 bytes) */
		if (send_cmd(ACMD13, 0) == 0) {	/* SD_STATUS */
			xchg_spi(0xFF);
			if (rcvr_datablock(ptr, 64)) res = RES_OK;
		}
		deselect();
		break;

	case CTRL_POWER_OFF :	/* Power off */
		power_off();
		Stat |= STA_NOINIT;
		res = RES_OK;
		break;
#if _USE_ISDIO
	case ISDIO_READ:
		sdi = buff;
		if (send_cmd(CMD48, 0x80000000 | (DWORD)sdi->func << 28 | (DWORD)sdi->addr << 9 | ((sdi->ndata - 1) & 0x1FF)) == 0) {
			//for (Timer1 = 100; (rc = xchg_spi(0xFF)) == 0xFF && Timer1; ) ;
			mmc_set_ms_timeout(1000);
			while( (rc = xchg_spi(0xFF)) == 0xFF && !mmc_test_timeout() ) 
				;
			if (rc == 0xFE) {
				for (bp = sdi->data, dc = sdi->ndata; dc; dc--) *bp++ = xchg_spi(0xFF);
				for (dc = 514 - sdi->ndata; dc; dc--) xchg_spi(0xFF);
				res = RES_OK;
			}
		}
		deselect();
		break;

	case ISDIO_WRITE:
		sdi = buff;
		if (send_cmd(CMD49, 0x80000000 | (DWORD)sdi->func << 28 | (DWORD)sdi->addr << 9 | ((sdi->ndata - 1) & 0x1FF)) == 0) {
			xchg_spi(0xFF); xchg_spi(0xFE);
			for (bp = sdi->data, dc = sdi->ndata; dc; dc--) xchg_spi(*bp++);
			for (dc = 514 - sdi->ndata; dc; dc--) xchg_spi(0xFF);
			if ((xchg_spi(0xFF) & 0x1F) == 0x05) res = RES_OK;
		}
		deselect();
		break;

	case ISDIO_MRITE:
		sdi = buff;
		if (send_cmd(CMD49, 0x84000000 | (DWORD)sdi->func << 28 | (DWORD)sdi->addr << 9 | sdi->ndata >> 8) == 0) {
			xchg_spi(0xFF); xchg_spi(0xFE);
			xchg_spi(sdi->ndata);
			for (dc = 513; dc; dc--) xchg_spi(0xFF);
			if ((xchg_spi(0xFF) & 0x1F) == 0x05) res = RES_OK;
		}
		deselect();
		break;
#endif
	default:
		res = RES_PARERR;
	}

	return res;
}
#endif

/*-----------------------------------------------------------------------*/
/* Device Timer Interrupt Procedure                                      */
/*-----------------------------------------------------------------------*/
/* This function must be called in period of 10ms                        */

///@brief mmc timer processes
void mmc_disk_timerproc (void)
{
    BYTE n;
#ifdef DETECT_WP
    BYTE s;
#endif

    n = Timer1;             /* 100Hz decrement timer */
    if (n) Timer1 = --n;
    n = Timer2;
    if (n) Timer2 = --n;

// FIXME our Micro SD card holder does not do WP or CD
// We assign STA_NODISK if we get a timeout
#ifdef DETECT_WP
    s = Stat;

    if (MMC_WP)             /* Write protected */
        s |= STA_PROTECT;
    else                    /* Write enabled */
        s &= ~STA_PROTECT;

    if (MMC_CD)             /* Card inserted */
        s &= ~STA_NODISK;
    else                    /* Socket empty */
        s |= (STA_NODISK | STA_NOINIT);

    Stat = s;               /* Update MMC status */
#endif
}

