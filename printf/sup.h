/**
 @file sup.h

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


#ifndef _SUP_H_
#define _SUP_H_

#if defined(PRINTF_TEST)
	#define MEMSPACE /* */
	#include <stdio.h>
	#include <stdlib.h>
	#include <string.h>
	typedef unsigned char uint8_t;
	typedef signed char int8_t;
	typedef unsigned short uint16_t;
	typedef unsigned int uint32_t;
	typedef int int32_t;
#else
	#include "user_config.h"

	//MEMSPACE int strlen ( char *str );
	MEMSPACE long strtol ( const char *nptr , char **endptr , int base );
	MEMSPACE long atoi ( uint8_t *str );
	MEMSPACE double atof ( char *str );
#endif

MEMSPACE int atodigit ( int c , int radix );
MEMSPACE long aton ( uint8_t *str , int base );
MEMSPACE void reverse ( char *str );
MEMSPACE void strupper ( char *str );

#endif
