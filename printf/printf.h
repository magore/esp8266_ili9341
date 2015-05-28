#ifndef __PRINTF__
#define __PRINTF__
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
	#include <ets_sys.h>
	#include <os_type.h>
	#include <osapi.h>
	#include "user_config.h"
	#include "printf.h"
#endif

#endif

/* printf.c */
MEMSPACE static void t_reverse ( char *str );
MEMSPACE static void t_strupper ( char *str );
MEMSPACE static int atod ( int c , int radix );
MEMSPACE static long aton ( uint8_t *str , int base );
MEMSPACE static int t_itoa ( long num , uint8_t *str , int max , int prec , int sign );
MEMSPACE static int t_ntoa ( unsigned long num , uint8_t *str , int max , int radix , int prec );
MEMSPACE static double t_iexp ( double num , int exp );
MEMSPACE static double t_scale10( double num , int *exp );
MEMSPACE static double t_atof ( char *str );
MEMSPACE static int ftoa ( double val , char *str , int intprec , int prec , int sign );
MEMSPACE static int etoa ( double x , char *str , int prec , int sign );
MEMSPACE int t_vsnprintf ( char *buffer , int len , const char *fmt , va_list va );
MEMSPACE int t_snprintf ( char *buffer , int buffer_len , const char *fmt , ...);
MEMSPACE int t_printf ( const char *fmt , ...);

