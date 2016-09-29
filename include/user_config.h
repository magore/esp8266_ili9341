/**
 @file user_config.h

 @brief Master include file for project
  Includes all project includes and defines here

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


#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define MEMSPACE ICACHE_FLASH_ATTR
#define MEMSPACE_RO ICACHE_RODATA_ATTR
//#define MEMSPACE_RO static const

#define ESP8266 1
#define NO_STDIO
#define FLOAT

#define _GNU_SOURCE

#include <stdarg.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>

#include <ets_sys.h>

// low level memory and flash reading code
#include "str.h"
#include "std.h"
#include "sys.h"

#include <ip_addr.h>
#include <espconn.h>
#include <gpio.h>
#include <user_interface.h>
#include <mem.h>

#define MMC_SLOW (80000000UL/500000UL)
#define MMC_FAST (80000000UL/2500000UL)


#define get_line(buf,size) uart0_gets(buf,size)

#include <uart_register.h>

// Simple queue reoutines
#include "queue.h"

// Hardware UART
#include "uart.h"

// Hardware SPI
#include "hspi.h"

#ifdef SCANF
	#include "scanf.h"
	#define sscanf t_sscanf
#endif

#include "printf.h"
#include "debug.h"

#ifdef YIELD_TASK
	#include "cont.h"
	#include "user_task.h"
#endif

// TIME and TIMER FUNCTION
#include "timer_hal.h"
#include "timer.h"
#include "time.h"

#ifdef FATFS_SUPPORT
// FATFS
#include "integer.h"
#include "ffconf.h"
#include "ff.h"
#include "diskio.h"
#include "disk.h"
#include "mmc_hal.h"

// FATFS POSIX WRAPPER
#include "posix.h"
// FATFS user tests and user interface
#include "fatfs_utils.h"
#endif

// TFT DISPLAY
#define MEMSPACE_FONT MEMSPACE
#include "font.h"
#include "ili9341_adafruit.h"
#include "ili9341.h"

// CORDIC math functions
#include "cordic2c_inc.h"
#include "cordic.h"

// Wireframe viewer functions
#include "wire_types.h"
#include "wire.h"


#include "network.h"

// Network client that displays messages on the TFT
#ifdef NETWORK_TEST
#include "server.h"
#endif

// Serial to/from telnet network task
#ifdef TELNET_SERIAL
#include "bridge.h"
#endif

#ifdef ADF4351
#include "adf4351.h"
#endif
// ram.c defines alternative safe functions
#define free safefree
#define calloc safecalloc
#define malloc safemalloc

// sys.c defines alternative safe functions
#ifndef free
    #define free(p) safefree(p)
#endif
#ifndef calloc
    #define calloc(n,s) safecalloc(n,s)
#endif
#ifndef malloc
    #define malloc(s) safemalloc(s)
#endif

#endif
