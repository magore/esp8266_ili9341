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

#if defined(PRINTF_TEST)
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

#include "sup.h"
// ====================================================================

typedef struct _printf_t
{
    void (*put)(struct _printf_t *, char);
    void *buffer;
    int len;
	int size;
} printf_t;

// ====================================================================

/* printf.c */
MEMSPACE int t_itoa ( long num , uint8_t *str , int max , int prec , int sign );
MEMSPACE int t_ntoa ( unsigned long num , uint8_t *str , int max , int radix , int prec );
MEMSPACE static double t_iexp ( double num , int exp );
MEMSPACE static double t_scale10 ( double num , int *exp );
MEMSPACE int t_ftoa ( double val , char *str , int intprec , int prec , int sign );
MEMSPACE int t_etoa ( double x , char *str , int prec , int sign );
MEMSPACE static void _puts_pad ( printf_t *fn , char *s , int width , int count , int left );
MEMSPACE void _printf_fn ( printf_t *fn , const char *fmt , va_list va );
static void _putc_buffer ( struct _printf_t *p , char ch );
MEMSPACE int vsnprintf ( char *str , size_t size , const char *format , va_list va );
MEMSPACE int snprintf ( char *str , size_t size , const char *format , ...);

#endif

