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

/// @brief MMC timeout counter in ms
uint16_t _mmc_timeout = 0;

/// @brief MMC SPI CLOCK cache
uint32_t _mmc_clock = 0;

// @brief MMC media status prescaler
uint16_t _mmc_pre = 0;

/**
 @brief 1000HZ timer task
 @return void
*/
static void mmc_task(void)
{
	if(_mmc_timeout)
		_mmc_timeout--;

	// 100HZ
    if(_mmc_pre++  < 10)
		return;

	_mmc_pre = 0;
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

/// @brief  Install MMC timer task: mmc_task()
///
/// @see  mmc_task()
/// @return  void
MEMSPACE
void mmc_install_timer( void )
{
    _mmc_timeout = 0;
    if(set_timers(mmc_task,1) == -1)
        printf("MMC Clock task init failed\n");
}

/// @brief SPI write buffer
/// @param[in] *data: transmit buffer
/// @param[in] count: number of bytes to write
/// @return  void
MEMSPACE
void mmc_spi_TX_buffer(const uint8_t *data, int count)
{
#ifdef ESP8266
    hspi_TX((uint8_t *) data,count);
#else
    SPI0_TX((uint8_t *) data,count);
#endif
}

/// @brief SPI read buffer
/// @param[in] *data: transmit buffer
/// @param[in] count: number of bytes to write
/// @return  void
MEMSPACE
void mmc_spi_RX_buffer(const uint8_t *data, int count)
{
#ifdef ESP8266
    hspi_RX((uint8_t *) data,count);
#else
    SPI0_RX((uint8_t *)data,count);
#endif
}

/// @brief SPI read 1 byte
/// @return  uint8_t value
MEMSPACE
uint8_t mmc_spi_RX()
{
    uint8_t data;
#ifdef ESP8266
    hspi_RX(&data,1);
#else
    SPI0_RX(&data,1);
#endif
	return(data);
}

/// @brief SPI write 1 byte
/// @param[in] data: value to transmit
/// @return  void
MEMSPACE
void mmc_spi_TX(uint8_t data)
{
#ifdef ESP8266
    hspi_TX(&data,1);
#else
    SPI0_TX(&data,1);
#endif
}


/// @brief SPI read and write 1 byte
/// @param[in] data: value to transmit
/// @return  uint8_t value read
MEMSPACE
uint8_t mmc_spi_TXRX(uint8_t data)
{
#ifdef ESP8266
    hspi_TXRX(&data,1);
#else
    SPI0_TXRX(&data,1);
#endif
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
	mmc_cli();
    _mmc_timeout = ms;
	mmc_sei();
}

///@brief Wait for timeout
///@return 1 ready
///@return 0 timeout
MEMSPACE
int  mmc_test_timeout()
{

    if( Stat & STA_NODISK )
        return(1);

    if(!_mmc_timeout)
    {
        printf("MMC TIMEOUT\n");
        Stat |= (STA_NODISK | STA_NOINIT);
        return(1);
    }
#ifdef ESP8266
    optimistic_yield(1000);
#endif
    return(0);
}

/// @brief has the MMC interface been initialized yet ?
static int mmc_init_flag = 0;
/// @brief Initialize MMC and FatFs interface, display diagnostics.
///
/// @param[in] verbose: display initialisation messages
/// @return
MEMSPACE
int mmc_init(int verbose)
{
    int rc;

    Stat = 0;

    if( verbose)
    {
        printf("==============================\n");
        printf("START MMC INIT\n");
    }

    mmc_slow();

    // we only install timers once!
    if(!mmc_init_flag)
        mmc_install_timer();

    if( verbose)
    {
#if defined (_USE_LFN)
        printf("LFN Enabled");
#else
        printf("LFN Disabled");
#endif
        printf(", Code page: %u\n", _CODE_PAGE);
    }

    rc = disk_initialize(0);	// aliased to mmc_disk_initialize()

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
                printf("MMC Block Size - read failed\n");
            }
            else
            {
                printf("MMC Block Size: %ld\n", blksize);
            }
            if( rc == RES_OK)
            {
                fatfs_status("/");
            }
        }
        printf("END MMC INIT\n");
        printf("==============================\n");
	}
    mmc_init_flag = 1;

    return( rc ) ;
}



/// @brief hs the mmc SPI bus been initialized yet ?
uint8_t _mmc_spi_init_flag = 0;
/// @brief  MMC SPI bus init
/// @return  void
MEMSPACE
void mmc_spi_init(int32_t clock)
{
#ifdef ESP8266
    SD_CS_INIT;
    hspi_init(clock,0);
    (void)mmc_spi_TX(0xff);
    hspi_waitReady();
#else
	IO_HI(MMC_CS);
    SPI0_Init(clock);	//< Initialize the SPI bus
    SPI0_Mode(0);		//< Set the clocking mode, etc
    mmc_spi_TX(0xff);
#endif
    _mmc_spi_init_flag=1;
}


/// @brief  MMC set slow SPI bus speed
///
/// - Used during card detect phase
/// @return  void
MEMSPACE
void mmc_slow()
{
	mmc_spi_init(_mmc_clock = MMC_SLOW);  //< In HZ 100khz..400khz
}



/// @brief  MMC fast SPI bus speed
///
/// - Used during normal file IO phases
/// @return  void
MEMSPACE
void mmc_fast()
{
	mmc_spi_init(_mmc_clock = MMC_FAST);  
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
    for(i=0;i<20;++i)
    {
#ifdef ESP8266
        os_delay_us(1000);
        optimistic_yield(1000);
        wdt_reset();
#else
        _delay_us(1000);
#endif
    }
}

/// @brief  MMC power off
///
/// - We do not control power to the MMC device in this project.
/// @return  void

MEMSPACE
void mmc_power_off()
{
#ifdef ESP8266
    hspi_waitReady();
    (void)mmc_spi_TX(0xff);
    hspi_waitReady();
#else
    (void)mmc_spi_TX(0xff);
#endif
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
	// restore clock speed in case another device changed it
    mmc_spi_init(_mmc_clock);
#ifdef ESP8266
    hspi_cs_enable(SD_CS_PIN);
#else
	IO_LOW(MMC_CS);
#endif
}


/// @brief MMC CS disable
///
/// - MMC SPI chip select
/// @return  void

MEMSPACE
void mmc_cs_disable()
{
#ifdef ESP8266
    hspi_cs_disable(SD_CS_PIN);
#else
	IO_HI(MMC_CS);
#endif
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
