
/**
 @file printf.c

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

#ifdef TEST_PRINTF
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#endif

#ifdef ESP8266
#include "user_config.h"
#endif

#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

#include <stdint.h>
#include <stdarg.h>
#include <math.h>

#include "printf.h"

// =======================================================================
// =======================================================================

// Below we included functions that are defined elsewhere
// They are included within printf to allow this to be standalone
// This is done by looking at header defines for the time being

// These functions are defined in our own str.c
#ifndef _STR_H_

// Skip if we have linux stdlib.h
#ifndef _STDLIB_H
/// @brief String Length
/// @param[in] str: string
/// @return string length
MEMSPACE
size_t strlen(const char *str)
{
    int len=0;
    // String length
    while(*str++)
        ++len;
    return(len);
}
#endif // _STDLIB_H

// Skip if we have linux ctype.h
#ifndef _CTYPE_H
/// @brief test if a character is a digit
/// @param[in] c: character
/// @return true or false
MEMSPACE 
int isdigit(int c)
{
	if(c >= '0' && c <= '9')
		return(1);
	return(0);
}
#endif 	// ifndef _CTYPE_H
#endif	// ifndef _STR_H_

// =======================================================================
// start of support functions
// =======================================================================

// These functions are defined in our own str.c
#ifndef _STR_H_
/// @brief Reverse a string in place
///  Example: abcdef -> fedcba
/// @param[in] str: string 
/// @return string length
MEMSPACE 
void reverse(char *str)
{
	char temp;
	int i;
	int len = strlen(str);
	// Reverse
	// We only exchange up to half way
	for (i = 0; i < (len >> 1); i++)
	{
		temp = str[len - i - 1];
		str[len - i - 1] = str[i];
		str[i] = temp;
	}
}

/// @brief UPPERCASE a string
/// @param[in] str: string 
/// @return void
MEMSPACE 
void strupper(char *str)
{

	while(*str)
	{
		if(*str >= 'a' && *str <= 'z')
			*str -= 0x20;
		++str;
	}
}
#endif // ifndef _STR_H_

// these functions are defined in our own stdlib.c
#ifndef _STD_H_
#ifdef FLOAT
/// @brief Raise number to integer exponent
/// @param[in] num: number
/// @param[in] exp:  integer exponent
/// @return num ** exp
MEMSPACE 
double iexp(double num, int exp)
{
	double a;
	if(exp==0)
		return(1.0);
	if(exp <0) 
	{
		a = 1.0 / num;
		exp = 1 - exp;
	}
	else 
	{
		exp = exp - 1;
		a = num;
	}
	while(exp) 
	{
		if(exp & 0x01)
			num *= a;
		if(exp >>= 1)
			a *= a;
	}
	return(num);
}

/// @brief Scale a number to 1.0 .. 9.99999...
/// @param[in] num: number
/// @param[out] *exp: interger power of 10 for scale factor
/// @return scaled number
MEMSPACE 
double scale10(double num, int *exp)
{
	int exp10,exp2;
	double scale;

	if(!num)
	{
		*exp = 0;
		return(0.0);
	}
	// extract exponent
	frexp(num, &exp2);
	// aproximate exponent in base 10
	exp10 = ((double) exp2) / (double) 3.321928095;

	// convert scale to 10.0**exp10
	scale = iexp((double)10.0, exp10);

	// remove scale
	num /= scale;

	// correct for over/under
	while(num >= (double)10.0) 
	{
		num /= (double) 10.0;
		++exp10;
	}
	while(num < (double) 1.0) 
	{
		num *= (double) 10.0;
		--exp10;
	}

	*exp = exp10;
	return(num);
}
#endif	// ifdef FLOAT
#endif	// ifndef _STD_H_

// ======================================================================
// end of support functions
// ======================================================================

/// @brief Convert number to ASCII
/// returns size of string after conversion
/// Note: It is allowed to have more digits converted then prec defines
/// Like itoa but can support leading '+/-/ ' or unsigned and zero padding
/// @param[in] num: number
/// @param[out] *str: string
/// @param[in] max: maximum length or string
/// @param[in] prec:  minumum number of digits, 0 padded if needed (can be zero)
/// @param[in] sign:  2: - or +, 1: -, 0: unsigned
///            NOT counted as part of prec length (just like printf)
/// @return long value
MEMSPACE 
int p_itoa(long num, char *str, int max, int prec, int sign)
{
		int digit,sign_ch;
		int ind;
        char *save = str;

		*str = 0;

//printf("itoa: num:%ld, max:%d, prec:%d, sign:%d\n", num, max,prec, sign);

		if(!max)		// make room for 0 eos at end of string
			return(0);

		++max; // add room for eos

		sign_ch = 0;
		// If the number we always have a leading chracter of '-', ' ' or '+'

		if(sign)
		{
			if((long)num < 0) 	// -
			{
				num = -num;
				sign_ch = '-';
				++max;
			}
			else 				// + or ' '
			{
				if(sign == 2)
					sign_ch = '+';
				else
					sign_ch = ' ';
				++max;
			}
		}
//printf("itoa: num:%ld, max:%d, prec:%d, sign:%d, sign_ch:%c\n", num, max,prec, sign, sign_ch);

		// Convert LSB to MSB
		ind  = 0;
		while(ind < max)
        {
			digit = num % 10;
			num /= 10;
			if(digit < 0)	// num = -num can fail for 2**n-1
				digit = -digit;
			digit += '0';
			*str++ = digit;
			++ind;
			if(!num && ind >= prec )
				break;
        }

//printf("itoa: ind:%d, max:%d\n", ind,max);
		if(sign_ch && ind < max)
		{
			*str++ = sign_ch;
			++ind;
		}
		*str = 0;

        reverse(save);
		ind = strlen(save);
		return(ind);
}

/// @brief Convert number to ASCII
/// Like itoa but can support leading '+/-/ ' or unsigned and zero padding
/// Note: It is allowed to have more digits converted then prec defines
/// @param[in] num: unsigned number
/// @param[out] *str: string
/// @param[in] max: maximum length or string
/// @param[in] radix:  Radix may only be 2,8,16
/// @param[in] prec:  minumum number of digits, zero padded if needed
///            NOT counted as part of prec length (just like printf)
/// @return returns size of string

MEMSPACE 
int p_ntoa(unsigned long num, char *str, int max, int radix, int prec)
{
		unsigned int mask,shift,digit;
		int ind;
        char *save = str;

		*str = 0;

		if(!max)		// make room for 0 eos at end of string
			return(0);

		++max; // add room for eos

		switch(radix)
		{
			case 2:
				mask = 1;
				shift = 1;
				break;
			case 8:
				mask = 7;
				shift = 3;
				break;
			case 16:
				mask = 15;
				shift = 4;
				break;
			default:
				return(0);
		}

		// convert LSB to MSB
		ind = 0;
		while(ind < max)
        {
			digit = num & mask;
			num >>= shift;
			// convert 
			if(digit < 10)
				digit += '0';
			else
				digit += ('a'  - 10);
			*str++ = digit;
			++ind;
			if(!num && ind >= prec )
				break;
        }

		*str = 0;
        reverse(save);
		return(strlen(save));
}



#ifdef FLOAT
/// @brief float to ASCII 
/// @param[in] val: value
/// @param[in] str: converted string
/// @param[in] intprec: integer precision
/// @param[in] prec: precision
/// @param[in] sign: sign
/// @return size of string
MEMSPACE 
int p_ftoa(double val, char *str, int intprec, int prec, int sign)
{
	char *save = str;
	double intpart, fraction, round, scale;
	int digit,digits,exp10,count;
	long num;

//printf("val:%f, intprec:%d, prec:%d, sign:%d\n", val, intprec, prec, sign);

	if(val < 0.0)
	{
		val = -val;
		*str++ = '-';
		if(intprec)
			--intprec;
	}
	else
	{
		if(sign)
		{
			if(sign == 2)
				*str++ = '+';
			if(sign == 1)
				*str++ = ' ';
		}
	}

	exp10 = 0;
	if( val )
	{
		// round number based on fraction prec
		round = 5.0 / iexp(10.0, prec+1);
		val += round;

		// fast reduce val to range 1.0 > val >= 0.1, and return adjusted base 10 exponent
		val = scale10(val,&exp10); /* 10.0 > val >= 1.0, val=val*10.0**exp */
		if(val >= 1.0) 
		{			/* CARRY ? */
			val /= 10.0;
			exp10++;
		}
	}
	else
	{
		if(intprec == 0)	// number was zero
			*str++ = '0';
	}
//printf("val:%f, exp10:%d\n", val, exp10);

	// padding
	while(intprec > exp10)
	{
		--intprec;
		*str++ = '0';
	}
	
	// Display integer digits
	while(exp10--)
	{
		val = modf(val*10.0, &intpart);
		digit = intpart + 0.5;
		*str++ = (digit + '0');
	}
	*str = 0;
//printf("save:%s\n", save);

	// Display fractional parts
	if(prec)
	{
		*str++ = '.';
		while(prec)
		{
			val = modf(val*10.0, &intpart);
			digit = intpart + 0.5;
			*str++ = digit + '0';
			--prec;
		}
	}
	*str = 0;
	return(strlen(save));
}


/// @brief float to ASCII 
/// @param[in] x: value
/// @param[in] str: converted string
/// @param[in] prec: digits after decimal place
/// @param[in] sign: sign
/// @return size of string
MEMSPACE 
int p_etoa(double x,char *str, int prec, int sign)
{
    double scale;   	/* scale factor */
    int i,          /* counter */
        d,          /* a digit */
        expon;      /* exponent */
	char *base = str;

    scale = 1.0 ;       /* scale = 10 ** prec */
    i = prec ;
    while ( i-- )
		scale *= 10.0 ;
    if ( x == 0.0 ) 
	{
        expon = 0 ;
        scale *= 10.0 ;
    }
    else 
	{
        expon = prec ;
        if ( x < 0.0 ) 
		{
            *str++ = '-' ;
            x = -x ;
        }
		else
		{
			if(sign == 2)
				*str++ = '+' ;
			else if(sign == 1)	// FIXME
				*str++ = ' ' ;
		}
        if ( x > scale ) 
		{
			
            /* need: scale<x<scale*10 */
            scale *= 10.0 ;
            while ( x >= scale ) 
			{
                x /= 10.0 ;
                ++expon ;
            }
        }
        else 
		{
            while ( x < scale ) 
			{
                x *= 10.0 ;
                --expon ;
            }
            scale *= 10.0 ;
        }
        /* at this point, .1*scale <= x < scale */
        x += 0.5 ;          /* round */
        if ( x >= scale ) 
		{
            x /= 10.0 ;
            ++expon ;
        }
    }
    i = 0 ;
    while ( i <= prec ) 
	{
        scale = floor( 0.5 + scale * 0.1 ) ;
        /* now, scale <= x < 10*scale */
        d = (int) ( x / scale ) ;
        *str++ = d + '0' ;
        x -= (double)d * scale ;
        if ( i++ ) continue ;
        *str++ = '.' ;
    }
    *str++ = 'e' ;
    if ( expon < 0 ) 
	{ 
		expon = -expon; 
		*str++ = '-'; 
	}
	else	
	{
		*str++ = '+'; 
	}
    *str++ = '0' + expon/10 ;
    *str++ = '0' + expon % 10 ;
    *str = 0;
	return(str - base);
}
              
#endif



// ====================================================================
// _puts_pad
//   Put string count bytes long, padded up to width, left or right aligned
// Padding is always done with spaces
//
// count number of characters to copy from buff
// width number of characters to pad up to - if needed
// left string is left aligned
//_puts(buff, width, count, left);
MEMSPACE
void _puts_pad(printf_t *fn, char *s, int width, int count, int left)
{
	int size = 0;
	int pad = 0;

	// note - if width > count we pad
	//        if width <= count we do not pad
	if(width > count)
	{
		pad = width - count;
	}

//printf("_puts_pad:(%s) width:%d, count:%d, left:%d, pad:%d, len:%d\n", s, width, count, left, pad, len);

	// left padding ?
	if(!left)
	{
//printf("_puts_pad:pad:%d\n", pad);
		while(pad--)
		{
			fn->put(fn,' ');
			++size;
		}
	}
//printf("_puts_pad:count:%d\n", count);

	// string
	while(*s && count--)
	{
		fn->put(fn,*s);
		++s;
		++size;
	}
	// right padding
	if(left)
	{
//printf("_puts_pad:pad:%d\n", pad);

		while(pad--) 
		{
			fn->put(fn,' ');
			++size;
		}
	}
//printf("_puts_pad:size:%d\n", size);
}	// _puts_pad()

/// @brief vsnprintf function
/// @param[out] fn: output character function pointer 
/// @param[in] fmt: printf forat string
/// @param[in] va: va_list arguments
/// @return size of string
MEMSPACE 
void _printf_fn(printf_t *fn, const char *fmt, va_list va)
{
	char ch;
    int prec, width, intprec, sign, left, fill;
	int precf, widthf;
	int count;
    int spec;
	int size;
	long num;
	double dnum;
	char *ptr;
	char *fmtptr;

	// buff has to be at least as big at the largest converted number
	// in this case base 2 long long with sign and end of string
	char buff[sizeof( long long int) * 8 + 2];


    while(*fmt) 
	{
		// emit up to %
        if(*fmt != '%') 
		{
            fn->put(fn,*fmt++);
			continue;
        }

		fmtptr = (char *) fmt;
		// process % specifier
		fmt++;

		left = 0;	// alignment
		fill = 0;	// fill character flag, space or 0
		size = 1;	// length short/log or float/double
		prec = 0;	// minimum number of digits displayed 
		precf = 0;	// prec was defined
		width = 0;	// padded width
		widthf = 0;	// width was defined
		sign = 0;	// 0 unsigned, 1 signed or space, 2 leading + or minus
		intprec = 0;// integer number of digits

		// ['-'][' '|'+']
		// [' '|'+']['-']
		// ...

		// we accept multiple flag combinations	
		// we do not care if flags are used more then once (ie. user error)
		while(*fmt == '-' || *fmt == ' ' || *fmt == '+')
		{
			if(*fmt == '-') 
				left = 1;
			if(*fmt == ' ') 
				sign = 1;
			if(*fmt == '+') 
				sign = 2;
			fmt++;
		}

		// 0 fill must proceed number
		if(*fmt == '0') 
		{
			fmt++;
			fill = '0';
		}

		if(isdigit(*fmt))
		{
			// optional width
			width = 0;
			widthf = 1;
			while(isdigit(*fmt))
				width = width*10 + *fmt++ - '0';
		}

//printf("width:%d, prec:%d, fill:%c\n", width,prec, fill);
		// prec always impiles zero fill to prec digigits for ints and longs
		//      is the number of digits after the . for float and double
		// regardlles of sign
		if( *fmt == '.' ) 
		{
			fmt++;
			prec = 0;
			precf = 1;
			while(isdigit(*fmt))
				prec = prec*10 + *fmt++ - '0';
		}
//printf("width:%d, prec:%d, fill:%c\n", width,prec, fill);

		if(*fmt == 'l') 
		{
			fmt++;
			size = 2;
		}

		spec = *fmt;


		// process integer arguments
		switch(spec) 
		{
			// Unsigned numbers
			case 'b':
			case 'B':
			case 'o':
			case 'O':
			case 'x':
			case 'X':
			case 'u':
			case 'U':
					sign = 0;
			case 'D':
			case 'd':
			case 'p':
				if(width && !prec)
				{
					if(fill == '0')
					{
						// always convert prec to width

						prec = width;
						width=0;
						if(sign)
						{
							if(prec)
								--prec;
						}
					}
				}
				if(size == 1)
					num = va_arg(va, int);
				else if(size == 2)
					num = va_arg(va, long);
				++fmt;
				break;
#ifdef FLOAT
			case 'f':
			case 'F':
//printf("width:%d, intprec:%d, prec:%d\n", width, intprec, prec);
				// FIXME K&R defines this as 6
				if(!precf)
					prec = 6;
				intprec = 0;
				if(fill == '0')
				{
					if(prec)
						intprec = width-prec-1;
					else
						intprec = width;
				}
				if(sign)
					intprec--;
				if(intprec < 0)
					intprec = 0;
				// floats are converted to double by va arg
			case 'e':
			case 'E':
				// FIXME K&R defines 'f' type as 6 - what is this default ?
				if(!precf)
					prec = 6;
				dnum = va_arg(va, double);
				++fmt;
				break;
#endif
			case 's':
				++fmt;
				break;
			case 'c':
				++fmt;
				break;
			default:
				break;
		}

		switch(spec) 
		{
#ifdef FLOAT
		case 'f':
			count = p_ftoa(dnum, buff, intprec, prec, sign);
			_puts_pad(fn,buff, width, count, left);
			break;

		case 'e':
			count = p_etoa(dnum, buff, prec, sign);
			_puts_pad(fn,buff, width, count, left);
			break;
#endif
//int p_itoa(unsigned long num, char *str, int prec, int sign)
		case 'u':
		case 'U':
			count = p_itoa(num, buff, sizeof(buff), prec, 0);
			_puts_pad(fn,buff, width, count, left);
			break;
//int p_itoa(unsigned long num, char *str, int prec, int sign)
		case 'd':
		case 'D':
//printf("<%ld, width:%d, prec:%d, sizeof(buff):%d, left:%d>\n", num, width, prec, (int)sizeof(buff), left);
			count = p_itoa(num, buff, sizeof(buff), prec, sign);
//printf("[%s, width:%d, count:%d, left:%d]\n", buff, width, count, left);
			_puts_pad(fn,buff, width, count, left);
			break;
//int p_ntoa(unsigned long num, char *str, int radix, int pad)
		case 'o':
		case 'O':
			count = p_ntoa(num, buff, sizeof(buff), 8, prec);
			_puts_pad(fn,buff, width, count, left);
			break;
		case 'x':
		case 'X':
		// pointers
		case 'p':
		case 'P':
			count = p_ntoa(num, buff, sizeof(buff), 16, prec);
			if(spec == 'X' || spec == 'P')
				strupper(buff);
			_puts_pad(fn,buff, width, count, left);
			break;
		case 'b':
		case 'B':
			count = p_ntoa(num, buff, sizeof(buff), 2, prec);
			_puts_pad(fn,buff, width, count, left);
			break;
		case 's':
			if(spec == 's')
			{
				ptr = va_arg(va, char *);
				if(!ptr)
					ptr = "(NULL)";
			}
		case 'c':
			// FIXME
			if(spec == 'c')
			{
				char tmp[2];
				tmp[0] = (char) va_arg(va, int);
				tmp[1] = 0;
				ptr = tmp;
			}
			count = strlen(ptr);
			if(prec)
				count = prec;
			if(count > width && width != 0)
				count = width;
//printf("width:%d,count:%d,left:%d\n", width, count, left);
			_puts_pad(fn,ptr, width, count, left);
			break;
		default:
			while(fmtptr <= fmt && *fmtptr)
				fn->put(fn, *fmtptr++);
			break;
		}
//printf("fmt:(%s)\n", fmt);
    }
//printf("fmt exit:(%s)\n", fmt);

}


// ====================================================================

/// @brief _putc_buffer_fn low level function writes a character to a buffer
/// @param[in] *p: structure with pointers and buffer to be written to
/// @param[in] ch: character to place in buffer
/// @return void
void _putc_buffer_fn(struct _printf_t *p, char ch)
{
	char *str;
	if (p->len )
	{
		if(ch)
		{
			p->len--;
			p->size++;
			str = (char *) p->buffer;
			*str++ = ch;
			p->buffer = (void *) str;
		}
	}
	*((char *)p->buffer) = 0;
}   



/// @brief vsnprintf function
/// @param[out] str: string buffer for result
/// @param[in] size: maximum length of converted string
/// @param[in] format: printf forat string
/// @param[in] va: va_list list of arguments
/// @return string size
MEMSPACE 
int vsnprintf(char* str, size_t size, const char *format, va_list va)
{

    int len;
    char *save = str;
    printf_t fn;

    *str = 0;

    fn.put = _putc_buffer_fn;
    fn.len = size;
    fn.size = 0;
    fn.buffer = (void *) str;

    _printf_fn(&fn, format, va);

    // FIXME check size should == fn.size on exit
    len = strlen(save);
    return( len );
}

/// @brief snprintf function
/// @param[out] str: string buffer for result
/// @param[in] size: maximum length of converted string
/// @param[in] format: printf forat string
/// @param[in] ...: list of arguments
/// @return string size
MEMSPACE 
int snprintf(char* str, size_t size, const char *format, ...)
{
    int len;
    va_list va;

    va_start(va, format);
    len= vsnprintf(str, size, format, va);
    va_end(va);

    return len;
}

