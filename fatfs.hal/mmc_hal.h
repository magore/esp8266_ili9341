/**
 @file fatfs/mmc_hal.h

 @brief MMC Hardware Layer for FatFs.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _MMC_HAL_H_
#define _MMC_HAL_H_

// Named address space
#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

// Weak attribute
#ifndef WEAK_ATR
#define WEAK_ATR __attribute__((weak))
#endif

#ifdef ESP8266
	#define SD_CS_PIN		MMC_CS
	#define SD_CS_ACTIVE   chip_enable(SD_CS_PIN)
	#define SD_CS_DEACTIVE chip_disable()
	// FIXME
	#define mmc_cli() /*< interrupt disable */
	// FIXME
	#define mmc_sei() /*< interrupt enable */
#else
	#define mmc_cli() cli() /*< interrupt disable */
	#define mmc_sei() sei() /*< interrupt enable */
#endif


/* mmc_hal.c */
MEMSPACE void mmc_install_timer ( void );
MEMSPACE void mmc_spi_TX_buffer ( const uint8_t *data , int count );
MEMSPACE void mmc_spi_RX_buffer ( const uint8_t *data , int count );
MEMSPACE uint8_t mmc_spi_RX ( void );
MEMSPACE void mmc_spi_TX ( uint8_t data );
MEMSPACE uint8_t mmc_spi_TXRX ( uint8_t data );
MEMSPACE void mmc_set_ms_timeout ( uint16_t ms );
MEMSPACE int mmc_test_timeout ( void );
MEMSPACE void mmc_ms_wait ( int ms );
MEMSPACE int mmc_init ( int verbose );
MEMSPACE void mmc_spi_init ( int32_t clock );
MEMSPACE void mmc_slow ( void );
MEMSPACE void mmc_fast ( void );
MEMSPACE void mmc_power_on ( void );
MEMSPACE void mmc_power_off ( void );
MEMSPACE int mmc_power_status ( void );
MEMSPACE void mmc_cs_enable ( void );
MEMSPACE void mmc_cs_disable ( void );
MEMSPACE int mmc_ins_status ( void );
MEMSPACE int mmc_wp_status ( void );


#endif                                            // _MMC_HAL_H_
