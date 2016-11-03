/**
 @file mathio.h

 @brief Math IO functions, and verious conversion code with floating point support

 @par Copyright &copy; 2016 Mike Gore, GPL License
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

#ifndef _MATHIO_H_
#define _MATHIO_H_

// Define what memory space the function is located in
// With ESP8266 we can use this for cached and non-cached code space
#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

// With AVR CPU types we can make this a 24bit pointer
#ifndef AVR
#define __memx /**/
#endif

// Weak attribute 
//   Allow functions defined here to be overridden by an external ones

#ifndef WEAK_ATR
#define WEAK_ATR __attribute__((weak))
#endif


///@brief  We let printf use user defined I/O functions
typedef struct _printf_t
{
    void (*put)(struct _printf_t *, char);
    void *buffer;
    int len;
	int sent;
} printf_t;


// ====================================================================
/* mathio.c */
MEMSPACE int atodigit ( int c , int radix );
MEMSPACE long atoh ( const char *p );
MEMSPACE long aton ( char *str , int base );
MEMSPACE long strtol ( const char *nptr , char **endptr , int base );
MEMSPACE int atoi ( const char *str );
MEMSPACE long atol ( const char *str );
MEMSPACE double iexp ( double num , int exp );
MEMSPACE double scale10 ( double num , int *exp );
MEMSPACE double atof ( const char *str );

// ====================================================================

#undef atof
/// @brief undefine any potential macro version of these functions
#undef strlen
#undef isdigit
#undef vnsprintf
#undef snprintf


/* io/printf.c */
MEMSPACE size_t WEAK_ATR strlen ( const char *str );
MEMSPACE int WEAK_ATR isdigit ( int c );
MEMSPACE void WEAK_ATR reverse ( char *str );
MEMSPACE void WEAK_ATR strupper ( char *str );
MEMSPACE int p_itoa ( long num , char *str , int max , int prec , int sign );
MEMSPACE int p_ntoa ( unsigned long num , char *str , int max , int radix , int prec );
MEMSPACE int p_ftoa ( double val , char *str , int intprec , int prec , int sign );
MEMSPACE int p_etoa ( double x , char *str , int prec , int sign );
MEMSPACE void _puts_pad ( printf_t *fn , char *s , int width , int count , int left );
MEMSPACE void _printf_fn ( printf_t *fn , __memx const char *fmt , va_list va );
void _putc_buffer_fn ( struct _printf_t *p , char ch );
MEMSPACE int vsnprintf ( char *str , size_t size , const char *format , va_list va );
MEMSPACE int snprintf ( char *str , size_t size , const char *format , ...);
int printf ( const char *format , ...);


/* sscanf.c */
int sscanf ( const char *strp , const char *fmt , ...);

#endif	// ifndef _MATHIO_H_
