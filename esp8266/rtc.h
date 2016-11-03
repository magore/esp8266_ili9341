/**
 @file hardware/rtc.h

 @brief DS1307 RTC Driver AVR8.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _RTC_H_
#define _RTC_H_

#define  DS1307  0xd0

/* rtc.c */
MEMSPACE uint8_t BINtoBCD ( uint8_t data );
MEMSPACE uint8_t BCDtoBIN ( uint8_t data );
MEMSPACE int8_t rtc_run_test ( void );
MEMSPACE int rtc_run ( int run );
MEMSPACE uint8_t rtc_init ( int force , time_t seconds );
MEMSPACE uint8_t rtc_write ( tm_t *t );
MEMSPACE uint8_t rtc_read ( tm_t *t );

#endif
