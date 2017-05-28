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

extern int putchar(int c);

// =============================================
/* mathio.c */
MEMSPACE int atodigit ( int c , int radix );
MEMSPACE long atoh ( const char *p );
MEMSPACE long aton ( char *str , int base );
MEMSPACE int mul10str ( uint8_t *str , int size );
MEMSPACE long strtol ( const char *nptr , char **endptr , int base );
MEMSPACE long long strtoll ( const char *nptr , char **endptr , int base );
#ifdef __SIZEOF_INT128__
MEMSPACE __uint128_t strto128 ( const char *nptr , char **endptr , int base );
#endif
MEMSPACE int atoi ( const char *str );
MEMSPACE long atol ( const char *str );
MEMSPACE double iexp ( double num , int exp );
MEMSPACE double scale10 ( double num , int *exp );
MEMSPACE double strtod ( const char *nptr , char **endptr );
MEMSPACE double atof ( const char *str );


// =============================================

#undef atof
/// @brief undefine any potential macro version of these functions
#undef strlen
#undef isdigit
#undef vnsprintf
#undef snprintf

// =============================================
/* printf.c */
///@brief  We let printf use user defined I/O functions
typedef struct _printf_t
{
    void (*put)(struct _printf_t *, char);
    void *buffer;
    int len;
	int sent;
} printf_t;

///@brief format specifier flags
typedef union {
    struct {
      unsigned short width : 1;
      unsigned short prec : 1;
      unsigned short plus : 1;
      unsigned short left : 1;
      unsigned short space : 1;
      unsigned short zero : 1;
      unsigned short neg: 1;
      unsigned short alt: 1;
    } b;
    unsigned short all;
} f_t;

/* printf.c */
MEMSPACE size_t WEAK_ATR strlen ( const char *str );
MEMSPACE int WEAK_ATR isdigit ( int c );
MEMSPACE void WEAK_ATR reverse ( char *str );
MEMSPACE void WEAK_ATR strupper ( char *str );
MEMSPACE int bin2num ( uint8_t *str , int strmax , int nummin , int base , uint8_t *nump , int numsize , int sign_ch );
MEMSPACE void pch_init ( char *str , int max );
MEMSPACE int pch ( char ch );
MEMSPACE int pch_ind ( void );
MEMSPACE int pch_max_ind ( void );
MEMSPACE void print_flags ( f_t f );
MEMSPACE int p_ntoa ( uint8_t *nump , int numsize , char *str , int strmax , int radix , int width , int prec , f_t f );
MEMSPACE int p_ftoa ( double val , char *str , int max , int width , int prec , f_t f );
MEMSPACE int p_etoa ( double val , char *str , int max , int width , int prec , f_t f );
MEMSPACE void _puts_pad ( printf_t *fn , char *s , int width , int count , int left );
MEMSPACE void _printf_fn ( printf_t *fn , __memx const char *fmt , va_list va );
MEMSPACE void _putc_buffer_fn ( struct _printf_t *p , char ch );
MEMSPACE int vsnprintf ( char *str , size_t size , const char *format , va_list va );
MEMSPACE int snprintf ( char *str , size_t size , const char *format , ...);
MEMSPACE int printf ( const char *format , ...);
#ifdef AVR
MEMSPACE int vsnprintf_P ( char *str , size_t size , __memx const char *format , va_list va );
MEMSPACE int snprintf_P ( char *str , size_t size , __memx const char *format , ...);
MEMSPACE int sprintf_P ( char *str , __memx const char *format , ...);
MEMSPACE int printf_P ( __memx const char *format , ...);
#endif


/* sscanf.c */
int sscanf ( const char *strp , const char *fmt , ...);

#endif	// ifndef _MATHIO_H_
