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

#include "esp8266/cpu.h"

#ifndef _SIZE_T
#define _SIZE_T
typedef unsigned long int size_t;
#endif

#ifndef _BOOL_T
#define _BOOL_T
#undef true
#undef false
#undef bool
typedef enum { false, true } bool;
#endif


/// @brief user task rate for software timers
#define SYSTEM_TASK_HZ 1000L

// FIXME move to std.h or some other header
/// @brief macros to simplify filling buffers
#define Mem_Clear(a) memset(a, 0, sizeof(a))
#define Mem_Set(a,b) memset(a, (int) b, sizeof(a))

#define SHARED_FILINFO

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

// low level memory and flash reading code
#include "esp8266/system.h"

#include "stringsup.h"

// Simple queue reoutines
#include "queue.h"

// Hardware UART
#include <uart_register.h>
#include "uart.h"

// Hardware SPI
#include "esp8266/hspi.h"
// Hardware HAL
#include "esp8266/hal.h"

//#include <math.h>
#undef atof
// scanf,printf and math i/o functions
#include "printf/mathio.h"
#include "esp8266/debug.h"

#ifdef YIELD_TASK
	#include "cont.h"
	#include "user_task.h"
#endif

// TIME and TIMER FUNCTION
//#include "lib/time.h"
//#include "lib/timer.h"

// FATFS
#ifdef FATFS_SUPPORT
//#include "fatfs.h"
#endif

// TFT DISPLAY
//#define MEMSPACE_FONT ICACHE_FLASH_ATTR
//#include "font.h"
//#include "ili9341_adafruit.h"
//#include "ili9341.h"

// CORDIC math functions
//#include "cordic2c_inc.h"
//#include "cordic.h"

// Wireframe viewer functions
//#include "wire_types.h"
//#include "wire.h"

//#include "network.h"

// Network client that displays messages on the TFT
#ifdef NETWORK_TEST
//#include "server.h"
#endif

// Serial to/from telnet network task
#ifdef TELNET_SERIAL
//#include "bridge.h"
#endif

#ifdef ADF4351
//#include "adf4351.h"
#endif

#endif // __USER_CONFIG_H__
