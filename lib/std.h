/**
 @file std.h

 @brief Part of Small printf, and verious conversion code with floating point support

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


#ifndef _STD_H_
#define _STD_H_

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

// Weak attribute
#ifndef WEAK_ATR
#define WEAK_ATR __attribute__((weak))
#endif

/* std.c */
MEMSPACE int WEAK_ATR atodigit ( int c , int radix );
MEMSPACE long WEAK_ATR aton ( char *str , int base );
MEMSPACE long WEAK_ATR strtol ( const char *nptr , char **endptr , int base );
MEMSPACE int WEAK_ATR atoi ( const char *str );
MEMSPACE long WEAK_ATR atol ( const char *str );
MEMSPACE double WEAK_ATR iexp ( double num , int exp );
MEMSPACE double WEAK_ATR scale10 ( double num , int *exp );
MEMSPACE double atof ( const char *str );


#endif
