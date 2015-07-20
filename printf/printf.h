/**
 @file printf.h

 @brief Small printf, and verious conversion code with floating point support

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



#ifndef _PRINTF_H_
#define _PRINTF_H_
#include <stdarg.h>

#ifdef PRINTF_TEST
	#define MEMSPACE /* */
	#include <stdio.h>
	#include <stdlib.h>
	typedef unsigned char uint8_t;
	typedef signed char int8_t;
	typedef unsigned short uint16_t;
	typedef unsigned int uint32_t;
	typedef int int32_t;
#else
	#include "user_config.h"
#endif
/* printf.c */
MEMSPACE static void t_reverse ( char *str );
MEMSPACE static void t_strupper ( char *str );
MEMSPACE static int atod ( int c , int radix );
MEMSPACE static long aton ( uint8_t *str , int base );
MEMSPACE static int t_itoa ( long num , uint8_t *str , int max , int prec , int sign );
MEMSPACE MEMSPACE static int t_ntoa ( unsigned long num , uint8_t *str , int max , int radix , int prec );
MEMSPACE static double t_iexp ( double num , int exp );
MEMSPACE static double t_scale10 ( double num , int *exp );
MEMSPACE static double t_atof ( char *str );
MEMSPACE static int ftoa ( double val , char *str , int intprec , int prec , int sign );
MEMSPACE static int etoa ( double x , char *str , int prec , int sign );
MEMSPACE MEMSPACE int t_vsnprintf ( char *buffer , int len , const char *fmt , va_list va );
MEMSPACE int t_snprintf ( char *buffer , int buffer_len , const char *fmt , ...);
MEMSPACE int t_printf ( const char *fmt , ...);


void tests ( void );

#endif

