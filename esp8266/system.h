/**
 @file system.h h

 @brief System memory and reset utilities

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

#ifndef _SYSTEM_H_
#define _SYSTEM_H_

#undef malloc
#undef calloc
#undef free

// sys.c defines alternative safe functions
#define free(p) safefree(p)
#define calloc(n,s) safecalloc(n,s)
#define malloc(s) safemalloc(s)

#ifdef ESP8266
MEMSPACE void reset ( void );
MEMSPACE void wdt_reset ( void );
#endif

MEMSPACE size_t freeRam ( void );
MEMSPACE void PrintRam ( void );
MEMSPACE void *safecalloc ( size_t nmemb , size_t size );
MEMSPACE void *safemalloc ( size_t size );
MEMSPACE void safefree ( void *p );
MEMSPACE void reset ( void );
MEMSPACE void wdt_reset ( void );


#endif // _SYSTEM_H_
