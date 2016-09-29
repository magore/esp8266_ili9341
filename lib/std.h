/**
 @file stdlib.h

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

#ifdef ESP8266
    #include "user_config.h"
#endif

#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

#include <stdint.h>
#include <math.h>

#include "str.h"

#endif

/* stdlib.c */
MEMSPACE double iexp ( double num , int exp );
MEMSPACE double scale10 ( double num , int *exp );
MEMSPACE int atodigit ( int c , int radix );
MEMSPACE long aton ( char *str , int base );

// Skip if we have the linux strlib.h
#ifndef _STDLIB_H
MEMSPACE long strtol ( const char *nptr , char **endptr , int base );
MEMSPACE int atoi ( const char *str );
MEMSPACE long atol ( const char *str );
MEMSPACE double atof ( const char *str );
#endif	// ifndef _STDLIB_H
