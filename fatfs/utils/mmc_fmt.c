/**
 @file fatfs/utils/mmc_fmt.c

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/
/*** MMC Unlocker *************************************************************
 *
 *	File Name  : mmcfmt.c
 *	Title      : Multi-Media Card Unlocker
 *  Description: Reset locked MMC cards (and wipes all data) allowing them
 *                to be used again
 *	Author     : Muhammad J. A. Galadima
 *	Created    : 2004 / 01 / 27
 *	Modified   : 2005 / 08 / 31
 *               2008 / 03 / 05
 *			Dropped serial port speed to 9600 because it didn't
 *                      work at 115200 with the internal osc any more (thought
 *                      I broke my STK (haven't used it in a while). It still
 *                      works at 57600 but I'll leave it at 9600 to be safe
 *	Version    : 0.2
 *	Target MCU : STK500 (can be used w/any AVRs with enough pins for
                 the MMC card (4) plus any switches/LEDs you want)
 *
 *
 *	This program is free software; you can redistribute it and/or
 *	modify it under the terms of the GNU General Public License
 *	as published by the Free Software Foundation; either version 2
 *	of the License, or (at your option) any later version.
 *
 *	This program is distributed in the hope that it will be useful,
 *	but WITHOUT ANY WARRANTY; without even the implied warranty of
 *	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *	GNU General Public License for more details.
 *
 *****************************************************************************/

#include <hardware/hardware.h>

static uint8_t spi_command ( void );
static uint16_t get_slice ( uint8_t *ptr_reg , uint8_t start , uint8_t stop );
static void get_regs ( void );
static void init_spi ( void );
static void init_vars ( void );
void mmc_format ( void );

#define spi_response()	mmc_xchg_spi(0xFF)					// read response
#define spi_busywait()	while(mmc_xchg_spi(0xFF) == 0)		// wait for non-zero response to continue

static uint8_t spi_cmd[5];	// 6 is CRC, which we won't be using here (or should be generated indide spi_command())
static uint8_t reg_csd[16];
static uint8_t reg_cid[16];
static uint16_t block_len;
static uint32_t block_cnt;

/*** spi_command **************************************************************
 *	Send contents of command structure, get (first) response byte
 *****************************************************************************/
static uint8_t spi_command(void)
{
	uint8_t retval, i;

    mmc_cs_enable();

	mmc_xchg_spi(0xFF);
	// send command struct
	mmc_xchg_spi(spi_cmd[0] | 0x40);	// make 2 MSBs '01' (01xxxxxx, where x represents command bit)
	mmc_xchg_spi(spi_cmd[4]);			// address
	mmc_xchg_spi(spi_cmd[3]);			//    "
	mmc_xchg_spi(spi_cmd[2]);			//    "
	mmc_xchg_spi(spi_cmd[1]);			//    "
	mmc_xchg_spi(0x95);					// CRC (from spec; calculate?)

	i=0;
	do {							// Flush Ncr (1-8 bytes) before response
		retval = mmc_xchg_spi(0xff);
		i++;
	} while(i<8 && retval == 0xFF);

	return retval;	// return R1 response (or first byte of any other command)
}


/*** get_slice ****************************************************************
 *	get the requested portion of the given register
 *****************************************************************************/
static uint16_t get_slice(uint8_t *ptr_reg, uint8_t start, uint8_t stop)
{
	uint8_t upper, lower, lower_s, count, i;
	uint32_t rval=0;

	lower = stop / 8.0;
	upper = start / 8.0;

	// merge all used bytes into on large var
	count = upper - lower;
	for(i=0; i<=count; i++) {
		rval <<= 8;
		rval |= ptr_reg[upper-i];
	}

	// shift data down so it starts at zero
	lower_s = stop - (lower * 8);
	rval >>= lower_s;

	// clear upper bits (all bits before start)
	start -= stop;
	start++;
	//for(i=start; i<32; i++)
	for(i=start; i<16; i++)	// clear bits 15-start
		rval &= ~(1<<i);
	rval &= 0x0000FFFF;			// clear bits 31-16

	return (uint16_t)rval;
}

/*** get_regs *****************************************************************
 *	read the contents of the CSD + CID registers from the card
 *****************************************************************************/
static void get_regs(void)
{
	uint8_t i, c_size_mult, read_bl_len;

	spi_cmd[0] = 9;
	myprintf("\nResponse to CMD9: 0 / %d", spi_command());
	while(mmc_xchg_spi(0xFF) != 0xFE);		// wait for data start token; add time-out?
	for(i=0; i<16; i++)
		reg_csd[15-i] = mmc_xchg_spi(0xFF);	// fill in from MSB

	myprintf("\n Contents of CSD Register:");	// defaults from SanDisk 32MB card / read values
	myprintf("\n  CSD_STRUCTURE:	1 / %x", get_slice(reg_csd,127,126) );
	myprintf("\n  MMC_PROT:		1 / %x", get_slice(reg_csd,125,122) );
	myprintf("\n  Reserved:		0 / %x", get_slice(reg_csd,121,120) );
	myprintf("\n  TAAC:			26 / %x", get_slice(reg_csd,119,112) );
	myprintf("\n  NSAC:			0 / %x", get_slice(reg_csd,111,104) );
	myprintf("\n  TRAN_SPEED:		2a / %x", get_slice(reg_csd,103,96) );
	myprintf("\n  CCC:			1ff / %x", get_slice(reg_csd,95,84) );
	read_bl_len=get_slice(reg_csd,83,80);
	myprintf("\n  READ_BL_LEN:		9 / %x", read_bl_len );
	myprintf("\n  READ_BL_PARTIAL:	1 / %x", get_slice(reg_csd,79,79) );
	myprintf("\n  WRITE_BLK_MISALIGN:	0 / %x", get_slice(reg_csd,78,78) );
	myprintf("\n  READ_BLK_MISALIGN:	0 / %x", get_slice(reg_csd,77,77) );
	myprintf("\n  DSR_IMP:		0 / %x", get_slice(reg_csd,76,76) );
	myprintf("\n  Reserved:		0 / %x", get_slice(reg_csd,75,74) );
	block_cnt=get_slice(reg_csd,73,62);	// this var is c_size, not block count; just using the block cnt to save space (instead of creating new var)
	myprintf("\n  C_SIZE:		- / %x", block_cnt );
	myprintf("\n  VDD_R_CURR_MIN:	4 / %x", get_slice(reg_csd,61,59) );
	myprintf("\n  VDD_R_CURR_MAX:	4 / %x", get_slice(reg_csd,58,56) );
	myprintf("\n  VDD_W_CURR_MIN:	5 / %x", get_slice(reg_csd,55,53) );
	myprintf("\n  VDD_W_CURR_MAX:	5 / %x", get_slice(reg_csd,52,50) );
	c_size_mult=get_slice(reg_csd,49,47);
	myprintf("\n  C_SIZE_MULT:		- / %x", c_size_mult );
	myprintf("\n  SECTOR_SIZE:		0 / %x", get_slice(reg_csd,46,42) );
	myprintf("\n  ERASE_GRP_SIZE:	- / %x", get_slice(reg_csd,41,37) );
	myprintf("\n  WP_GRP_SIZE:		1f / %x", get_slice(reg_csd,36,32) );
	myprintf("\n  WP_GRP_ENABLE:	1 / %x", get_slice(reg_csd,31,31) );
	myprintf("\n  DEFAULT_ECC:		0 / %x", get_slice(reg_csd,30,29) );
	myprintf("\n  R2W_FACTOR:		4 / %x", get_slice(reg_csd,28,26) );
	myprintf("\n  WRITE_BL_LEN:		9 / %x", get_slice(reg_csd,25,22) );
	myprintf("\n  WRITE_BL_PARTIAL:	0 / %x", get_slice(reg_csd,21,21) );
	myprintf("\n  Reserved:		0 / %x", get_slice(reg_csd,20,16) );
	myprintf("\n  Reserved:		0 / %x", get_slice(reg_csd,15,15) );
	myprintf("\n  COPY:			1 / %x", get_slice(reg_csd,14,14) );
	myprintf("\n  PERM_WRITE_PROTECT:	0 / %x", get_slice(reg_csd,13,13) );
	myprintf("\n  TEMP_WRITE_PROTECT:	0 / %x", get_slice(reg_csd,12,12) );
	myprintf("\n  Reserved:		0 / %x", get_slice(reg_csd,11,10) );
	myprintf("\n  ECC:			0 / %x", get_slice(reg_csd,9,8) );
	myprintf("\n  CRC:			- / %x", get_slice(reg_csd,7,1) );
	myprintf("\n  Not Used (Always 1):	1 / %x", get_slice(reg_csd,0,0) );

	block_cnt++;
	block_cnt *= (1<<(c_size_mult+2));
	block_len = (1<<read_bl_len);
	myprintf("\n Card Size:  %ld blocks @ %db = %ldmb\n",
		block_cnt, block_len, block_cnt*block_len/1000000 );


	spi_cmd[0] = 10;
	myprintf("\nResponse to CMD10: 0 / %d", spi_command());
	while(mmc_xchg_spi(0xFF) != 0xFE);		// wait for data start token; add time-out?
	for(i=0; i<16; i++)
		reg_cid[15-i] = mmc_xchg_spi(0xFF);	// fill in from MSB
	myprintf("\n Contents of CID Register:");
	myprintf("\n  Product Name:	");
	for(i=0; i<7; i++)
		myprintf("%c", reg_cid[12-i] & 0xff);
	myprintf("\n  Mfg Month:		%d", get_slice(reg_cid,15,12));		// ??? card returns 12/2005
	myprintf("\n  Mfg Year:		%d", 1996+get_slice(reg_cid,11,8));	// ??? .. unless it's 12/2004?
}

/*** init_spi *****************************************************************
 *	init card
 *****************************************************************************/
static void init_spi(void)
{
	uint8_t tries, retval;

	mmc_cs_disable();
	mmc_xchg_spi(0xFF);				// spec says to clock at least 74 high bits into the
	mmc_xchg_spi(0xFF);				//	MMC; we'll do 80 here, it's easier ;)
	mmc_xchg_spi(0xFF);
	mmc_xchg_spi(0xFF);
	mmc_xchg_spi(0xFF);
	mmc_xchg_spi(0xFF);
	mmc_xchg_spi(0xFF);
	mmc_xchg_spi(0xFF);
	mmc_xchg_spi(0xFF);
	mmc_xchg_spi(0xFF);
	mmc_cs_enable();

	// send CMD0 (reset/idle)

	// send CMD0 (reset) continuously until out of idle
	// loop until return == 00 0001 (idle); ?after timeout there's an error
	tries = 0xFF;
	while(( (retval=spi_command())==0) && tries!=0)	// (less space than checking !=1) all vals in array already 0, no need to mod spi_command
		tries--;								// all vals in array already 0, no need to mod
	myprintf("\nResponse to CMD0: 1 / %d", retval);	// get rid of retval assignment in while when done debugging
	if(tries == 0)
		return;

	// send CMD1 (init) continuously until ready
	// loop until return == 00 0000 (ready); ?after timeout there's an error
	spi_cmd[0] = 1;
	tries = 0xFF;
	while(( (retval=spi_command())!=0) && tries!=0)			//
		tries--;
	myprintf("\nResponse to CMD1: 0 / %d", retval);
	if(tries == 0)
		return;

	spi_cmd[0] = 13;
	retval = spi_command();
	myprintf("\nResponse to CMD13: 0,0 / %d,%d", retval,mmc_xchg_spi(0xff));
	    // show 1st and 2nd bytes of R1b response

	get_regs();
}


/*** init_vars ****************************************************************
 *	init vars
 *****************************************************************************/
static void init_vars(void)
{
	spi_cmd[0] = 0;		// command
	spi_cmd[4] = 0;		// 31-24	(MSB)
	spi_cmd[3] = 0;		// 23-16
	spi_cmd[2] = 0;		// 15-08
	spi_cmd[1] = 0;		// 07-00	(LSB)
}

void mmc_fmt(void)
{
	init_vars();

	myprintf("\n\n\nLocked Card Fixer (build 080305003), by Muhammad J. A. Galadima.\n\n");

	init_spi();

	// now do something (ie. erase card!)

	// set block length to 1
	spi_cmd[0] = 16;	// set block length / R1
	spi_cmd[4] = 0;		// set size to 1
	spi_cmd[3] = 0;		// 		"
	spi_cmd[2] = 0;		// 		"
	spi_cmd[1] = 1;		// 		"
	spi_command();

	// send the send forced erase command
	spi_cmd[0] = 42;	// LOCK_UNLOCK / R1b
	spi_cmd[4] = 0;		//
	spi_cmd[3] = 0;		// 		"
	spi_cmd[2] = 0;		// 		"
	spi_cmd[1] = 0;		// 		"
	spi_command();
	spi_busywait();		// wait for busy

	// send the data byte (1 byte, as defined in block length)
	mmc_xchg_spi(0b11111110);	// data start token
	mmc_xchg_spi(0b00001000);	// forced erase
	mmc_xchg_spi(0xFF);			//
	mmc_xchg_spi(0xFF);			// CRC

	myprintf("\n\nData Response: 0x%x", 0b00011111 & mmc_xchg_spi(0xFF));	// read data response
	spi_busywait();
	myprintf("\nCard Erased!");

}
