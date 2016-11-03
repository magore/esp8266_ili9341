/**
 @file cpu.h

 @brief Master Include for FatFs, RTC, Timers AVR8 - Part of HP85 disk emulator.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

*/

#ifndef _CPU_H_
#define _CPU_H_

#if !defined(F_CPU)
#define F_CPU 80000000UL
#endif

#ifndef ESP8266
#define ESP8266
#endif

#ifndef MEMSPACE
#define MEMSPACE ICACHE_FLASH_ATTR
//#define MEMSPACE /* */
#endif
#ifndef MEMSPACE_RO
#define MEMSPACE_RO ICACHE_RODATA_ATTR
//#define MEMSPACE_RO static const
#endif

#ifndef MEMSPACE_FONT
#define MEMSPACE_FONT ICACHE_FLASH_ATTR
#endif

#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include "spi_register.h"  // from 0.9.4 IoT_Demo
#include <ets_sys.h>
#include <ip_addr.h>
#include <espconn.h>
#include <gpio.h>
#include <user_interface.h>
#include <mem.h>
#include <stdbool.h>

#include <stdint.h>
#include <stdarg.h>
#include "esp8266/bits.h"

#ifndef NULL
#define NULL        ((void *) 0)
#endif

//typedef uint8_t bool;
#define true 1
#define false 0

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long int size_t;
#endif

/// @brief user task rate for software timers
#define SYSTEM_TASK_HZ 1000L

// FIXME move to std.h or some other header
/// @brief macros to simplify filling buffers
#define Mem_Clear(a) memset(a, 0, sizeof(a))
#define Mem_Set(a,b) memset(a, (int) b, sizeof(a))

#ifdef AVR
#undef printf
#define printf(format, args...) rs232_printf(PSTR(format), ##args)
#endif

#undef snprintf
#define snprintf(s, size, format, args...) rs232_snprintf(s, size, PSTR(format), ##args)

// FIXME size
#undef sprintf
#define sprintf(s, format, args...) rs232_snprintf(s, 80, PSTR(format), ##args)

#include "esp8266/system.h"

#endif  // ifndef _CPU_H_
