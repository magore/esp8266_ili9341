/**
 @file sys.h

 @brief System utilities

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

#ifndef _SYS_H_
#define _SYS_H_

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#ifdef USER_CONFIG
#include "user_config.h"
#endif

// Named address space
#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

extern void * _heap_start;
#define HEAP_START  ((uint32_t) & (_heap_start))
#define HEAP_END    ((uint32_t) (0x3FFFC000U - 1U))

/* utils/sys.c */
MEMSPACE void *malloc ( size_t size );
MEMSPACE void *calloc ( size_t nmemb , size_t size );
MEMSPACE void free ( void *p );
MEMSPACE uint32_t freeRam ( void );
MEMSPACE void PrintRam ( void );
MEMSPACE void *safecalloc ( size_t nmemb , size_t size );
MEMSPACE void *safemalloc ( size_t size );
MEMSPACE void safefree ( void *p );
MEMSPACE void reset ( void );
MEMSPACE void wdt_reset ( void );


#endif
