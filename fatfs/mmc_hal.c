/**
 @file fatfs/posix.c

 @brief PRovides hardware abstraction layer to MMC.C
 @par Copyright &copy; 2015 Mike Gore, GPL License
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

#include <user_config.h>
#include "diskio.h"
#include "disk.h"
#include "ff.h"
#include "mmc_hal.h"

extern DSTATUS Stat;

/// @brief MMC timeout counter
uint16_t _mmc_timeout = 0;
/// @brief MMC SPI CLOCK cache
uint16_t _mmc_clock = 0;
/**
 @brief 1000HZ timer task
 @param[in] *arg: ignored
 @return void
*/
LOCAL void mmc_task(void)
{
    if(_mmc_timeout)
		_mmc_timeout--;

	if(_mmc_clock++  >= 10)
	{
		_mmc_clock = 0;
// FIXME our Micro SD card holder does not do WP or CD
// We assign STA_NODISK if we get a timeout
#if 0
		if (mmc_wp_status())                      /* Write protected */
			Stat |= STA_PROTECT;
		else                                      /* Write enabled */
			Stat &= ~STA_PROTECT;
		if (mmc_ins_status())                     /* Card inserted */
			Stat &= ~STA_NODISK;
		else                                      /* Socket empty */
			Stat |= (STA_NODISK | STA_NOINIT);
#endif
	}
}



/// @brief  Install MMC timer task: mmc_task() 
///
/// @see  mmc_task()
/// @return  void
MEMSPACE
void mmc_install_timer( void )
{
	_mmc_timeout = 0;
    if(set_timers(mmc_task,1) == -1)
        DEBUG_PRINTF("MMC Clock task init failed\n");
}

/// @brief SPI write buffer
/// @param[in] *data: transmit buffer
/// @param[in] count: number of bytes to write
/// @return  void
MEMSPACE
void mmc_spi_TX_buffer(const uint8_t *data, int count)
{
	hspi_TX((uint8_t *) data,count);
}
/// @brief SPI read buffer
/// @param[in] *data: transmit buffer
/// @param[in] count: number of bytes to write
/// @return  void
MEMSPACE
void mmc_spi_RX_buffer(const uint8_t *data, int count)
{
	hspi_RX((uint8_t *)data,count);
}

/// @brief SPI read 1 byte
/// @return  uint8_t value
uint8_t mmc_spi_RX()
{
	uint8_t data;
	hspi_RX(&data,1);
}

/// @brief SPI write 1 byte
/// @param[in] data: value to transmit
/// @return  void 
void mmc_spi_TX(uint8_t data)
{
	hspi_TX(&data,1);
}

/// @brief SPI read and write 1 byte
/// @param[in] data: value to transmit
/// @return  uint8_t value read
uint8_t mmc_spi_TXRX(uint8_t data)
{
	hspi_TXRX(&data,1);
	return(data);
}



/// @brief Set MMC timeout timer in Milliseconds
///
/// @param[in] ms: timeout in Milliseconds
///
/// @see mmc_test_timeout ( )
/// @return  void
MEMSPACE
void mmc_set_ms_timeout(uint16_t ms)
{
	_mmc_timeout = ms;
}

///@brief Wait for timeout
///@return 1 ready
///@return 0 timeout
int  mmc_test_timeout()
{
wdt_reset();

	if( Stat & STA_NODISK )
		return(1);

	if(!_mmc_timeout)
	{
		DEBUG_PRINTF("MMC TIMEOUT\n");
		Stat |= (STA_NODISK | STA_NOINIT);
		return(1);
	}
	return(0);
}

/// @brief has the MMC interface been initialized yet ?
static int mmc_init_flag = 0;
/// @brief Initialize MMC and FatFs interface, display diagnostics.
///
/// @param[in] cold: 1 also initailize MMC timer.
/// @return

MEMSPACE
int mmc_init(int verbose)
{
    int rc;

    Stat = 0;

    if( verbose)
    {
		DEBUG_PRINTF("==============================\n");
        DEBUG_PRINTF("START MMC INIT\n");
    }

    mmc_spi_init();

	// we only install timers once!
    if(!mmc_init_flag)
        mmc_install_timer();

    if( verbose)
    {
#if defined (_USE_LFN)
        DEBUG_PRINTF("LFN Enabled");
#else
        DEBUG_PRINTF("LFN Disabled");
#endif
        DEBUG_PRINTF(", Code page: %u\n", _CODE_PAGE);
    }

    rc = disk_initialize(0);                      // aliased to mmc_disk_initialize()
    if( rc != RES_OK )
    {
        put_rc(rc);
    }

    if( rc == RES_OK)
    {
        rc = f_mount(&Fatfs[0],"/", 0);
    }

    if( rc != RES_OK)
    {
        put_rc( rc );
    }

	if (verbose )
	{
        DWORD blksize = 0;
		if(rc == RES_OK)
		{
			rc = mmc_disk_ioctl ( Fatfs[0].drv, GET_BLOCK_SIZE, (void *) &blksize);
			if( rc != RES_OK)
			{
				put_rc( rc );
				DEBUG_PRINTF("MMC Block Size - read failed\n");
			}
			else
			{
				DEBUG_PRINTF("MMC Block Size: %ld\n", blksize);
			}
			if( rc == RES_OK)
			{
				fatfs_status("/");
			}
		}
		DEBUG_PRINTF("END MMC INIT\n");
		DEBUG_PRINTF("==============================\n");
	}

	mmc_init_flag = 1;

    return( rc ) ;
}



/// @brief hs the mmc SPI bus been initialized yet ?
uint8_t _mmc_spi_init_flag = 0;
/// @brief  MMC SPI bus init
/// @return  void
MEMSPACE
void mmc_spi_init()
{
	SD_CS_INIT;
	mmc_slow();  //< Set the speed to "slow" the slowest speed an MMC device might need.
	_mmc_spi_init_flag=1;
}


/// @brief  MMC set slow SPI bus speed 
///
/// - Used during card detect phase
/// @return  void
MEMSPACE
void mmc_slow()
{
	// FIXME need to computer that actual devisor with more thought
	hspi_waitReady();
	// This is actually just over 300K
    _mmc_clock = 80000000UL/500000UL;
	hspi_init(_mmc_clock,0);
}


/// @brief  MMC fast SPI bus speed 
///
/// - Used during normal file IO phases
/// @return  void
MEMSPACE
void mmc_fast()
{
	// 2.5M
	hspi_waitReady();
    _mmc_clock= 80000000UL/2500000UL;
	hspi_init(_mmc_clock,0);
}


/// @brief  MMC power ON initialize
///
/// - We do not control power to the MMC device in this project.
/// @return  void
MEMSPACE
void mmc_power_on()
{
	int i;
	// SD CS should be off
	mmc_slow();
    (void)mmc_spi_TX(0xff);
	for(i=0;i<20;++i)
	{
		os_delay_us(1000);
wdt_reset();
	}
}


/// @brief  MMC power off 
///
/// - We do not control power to the MMC device in this project.
/// @return  void

MEMSPACE
void mmc_power_off()
{
}


/// @brief  MMC power status
///
/// - We do not control power to the MMC device in this project.
/// @return  1 power is alwasy on

MEMSPACE
int mmc_power_status()
{
    return (1);
}



/// @brief  MMC CS enable
///
/// - MMC SPI chip select 
/// @return  void

MEMSPACE
void mmc_cs_enable()
{
    hspi_init(_mmc_clock,0);
// FIXME we have both macros for SD_CS and whe are passing a value here
// Unify the method
// hspi_cs_enable checks bus speed required for each device and changes it if required
	hspi_cs_enable(SD_CS_PIN);
    //SD_CS_ACTIVE;
}


/// @brief MMC CS disable
///
/// - MMC SPI chip select 
/// @return  void

MEMSPACE
void mmc_cs_disable()
{
	hspi_cs_disable(SD_CS_PIN);
    //SD_CS_DEACTIVE;
}


/// @brief  MMC Card Inserted status
///
/// - We do not detect card insert status in this project.
/// @return 1 card inserted

MEMSPACE
int mmc_ins_status()
{
    return (1);
}


/// @brief  MMC Card Write Protect status
///
/// - We do not detect card write protect status in this project.
/// @return 0 == not write protected

MEMSPACE
int mmc_wp_status()
{
    return (0);
}
