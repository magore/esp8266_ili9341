
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


// Stand alone test
//#define TEST

// Float support
#define FLOAT

#ifdef TEST
	#define MEMSPACE /* */
	#include <stdio.h>
	#include <stdlib.h>
	typedef unsigned char uint8_t;
	typedef signed char int8_t;
	typedef unsigned short uint16_t;
	typedef unsigned int uint32_t;
	typedef int int32_t;
#else
	#include <user_config.h>
	#include "printf.h"
#endif

#include <string.h>
#include <stdarg.h>
#include <math.h>


/// @brief String Length
/// @param[in] str: string 
/// @return string length
MEMSPACE 
static int t_strlen(char *str)
{
	int len=0;
	// String length
	while(*str++)
		++len;
	return(len);
}

/// @brief Reverse a string
///  Example: abcdef -> fedcba
/// @param[in] str: string 
/// @return string length
MEMSPACE 
static void t_reverse(char *str)
{
        uint8_t temp;
        int i;
		int len = t_strlen(str);
        // Reverse
        // We only exchange up to half way
        for (i = 0; i < (len >> 1); i++)
        {
                temp = str[len - i - 1];
                str[len - i - 1] = str[i];
                str[i] = temp;
        }
}

/// @brief Upercase a string
/// @param[in] str: string 
/// @return void
MEMSPACE 
static void t_strupper(char *str)
{

	while(*str)
	{
		if(*str >= 'a' && *str <= 'z')
			*str -= 0x20;
		++str;
	}
}




/// @brief Convert ASCII character to radix based digit , or -1
/// @param[in] c: character
/// @param[in] radix: radix
/// @return radix based digit , or -1
MEMSPACE 
static int atod(int c,int radix)
{
        int ret = -1;
        if(c >= '0' && c <= '9')
                ret = c - '0';
        else if(c >= 'A' && c <= 'F')
                ret = c - 'A' + 10;
        else if(c >= 'a' && c <= 'f')
                ret = c - 'a' + 10;
		else return (-1);
        return((ret >= radix) ? -1 : ret);
}

/// @brief Convert ASCII string to number
/// @param[in] str: string
/// @param[in] base: radix
/// @return long value
MEMSPACE 
static long aton(uint8_t *str,int base)
{
	unsigned long num;
	int sign;
	int d;

	while(*str == ' ' || *str == '\t')
		++str;
	sign = 0;
	if(*str == '-' ) 
	{
		sign = 1;
		++str;
	} 
	else if(*str == '+' )
		{
			++str;
		}
	num = 0;

	while(*str)
	{
		d = atod(*str,base);
		if(d < 0)
			break;
		num = num*base;
		num += d;
		++str;
	}

	if(sign)
		return(-num);
	else
		return(num);
}


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
static int t_itoa(long num, uint8_t *str, int max, int prec, int sign)
{
		int digit,sign_ch;
		int ind;
        uint8_t *save = str;

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

        t_reverse(save);
		ind = strlen(save);
		return(ind);
}

/// @brief Convert number to ASCII
/// Like itoa but can support leading '+/-/ ' or unsigned and zero padding
/// Note: It is allowed to have more digits converted then prec defines
/// @param[in] num: number
/// @param[out] *str: string
/// @param[in] max: maximum length or string
/// @param[in] radix:  Radix may only be 2,8,16
/// @param[in] prec:  minumum number of digits, zero padded if needed
///            NOT counted as part of prec length (just like printf)
/// @return returns size of string

MEMSPACE 
static int t_ntoa(unsigned long num, uint8_t *str, int max, int radix, int prec)
{
		uint8_t mask,shift,digit;
		int ind;
        uint8_t *save = str;

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
        t_reverse(save);
		return(strlen(save));
}


#ifdef FLOAT

/// @brief Raise number to exponent power (exponent is integer)
/// @param[in] num: number
/// @param[in] exp:  interger exponent
/// @return num ** exp
MEMSPACE 
static double t_iexp(double num, int exp)
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
static double t_scale10(double num, int *exp)
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
	scale = t_iexp((double)10.0, exp10);

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
#endif

/// @brief atof ASCII to float
/// @param[in] str: string
/// @return number
#ifdef FLOAT
MEMSPACE 
static double t_atof(str)
char *str;
{
	double num;
	double frac;

	int i,j,power,sign;

	while(*str == ' ' || *str == '\t' || *str == '\n')
		++str;
	sign = (*str == '-') ? -1 : 1;
	if(sign == -1 || *str == '+') 
		str++;
	num=0.0; 
	while(isdigit(*str)) 
	{
		num = num * 10.0 + *str - '0';
		str++;
	}
	if(*str == '.') 
	{
		++str;
		1.0; 
		while(isdigit(*str)) 
		{
			num = num * 10.0 + *str - '0';
			frac *= 10.0;
			str++;
		}
		num /= frac;
	}
	num *= sign;
	if(*str == 'E' || *str == 'e') 
	{
		str++;
		sign = (*str == '-') ? -1 : 1;
		if(sign == -1 || *str == '+') 
			str++;
		power=0;
		while(isdigit(*str)) 
		{
			power = power * 10 + *str - '0';
			str++;
		}
		if(num == 0.0)
			return(num);
		if(sign<0)
			power = -power;
		return(num * t_iexp(10.0, power));
	}
	return(num);
}
#endif

#ifdef FLOAT
/// @brief ftoa float to ASCII 
/// @param[in] val: value
/// @param[in] str: converted string
/// @param[in] intprec: integer precision
/// @param[in] prec: precision
/// @param[in] sign: sign
/// @return size of string
MEMSPACE 
static int ftoa(double val, char *str, int intprec, int prec, int sign)
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
			++intprec;
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
		round = 5.0 / t_iexp(10.0, prec+1);
		val += round;

		// fast reduce val to range 1.0 > val >= 0.1, and return adjusted base 10 exponent
		val = t_scale10(val,&exp10); /* 10.0 > val >= 1.0, val=val*10.0**exp */
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

// FIXME- use t_itoa - scale number and limit digits
#ifdef JUNK
	fraction = modf(val, &intpart);
	num = intpart;
	count = t_itoa(num, str, 64, intprec, 0);
	str += count;
	*str++ = '.';
	fraction *= t_iexp(10.0, prec);
	fraction += .5;
	num = fraction;
// FIXME - do not use ITOA
	count = t_itoa(fraction, str, 64, prec, 0);
	str += count;
#endif

	*str = 0;
	return(t_strlen(save));
}


/// @brief etoa float to ASCII 
/// @param[in] x: value
/// @param[in] str: converted string
/// @param[in] prec: digits after decimal place
/// @param[in] sign: sign
/// @return size of string
MEMSPACE 
static int etoa(double x,char *str, int prec, int sign)
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
			else
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

/// @brief vsnprintf function
/// @param[out] buffer: string buffer for result
/// @param[in] len: maximum length of converted string
/// @param[in] fmt: printf forat string
/// @param[in] va: va_list arguments
/// @return size of string
MEMSPACE 
int t_vsnprintf(char *buffer, int len, const char *fmt, va_list va)
{
	char ch;
    int prec, width, intprec, sign, left, fill;
	int precf, widthf;
	int count;
    int size;
    int spec;
	long num;
	double dnum;
	char *ptr;
	char *save= buffer;
	char *fmtptr;

	*buffer = 0;
	
	// buff has to be at least as big at the largest converted number
	// in this case base 2 long long with sign and end of string
	char buff[sizeof( long long int) * 8 + 2];

	// ====================================================================
	int _putc(char ch)
	{
		if (!len )
		{
			*buffer=0;
			return 0;
		}
		if(ch)
		{
			--len;
			*buffer++ = ch;
		}
		*buffer=0;
		return(1);
	}	// _putc()
	// ====================================================================

	// ====================================================================
	// _puts 
	//   Put string count bytes long, padded up to width, left or right aligned
	// Padding is always done with spaces
	//
	// count number of characters to copy from buff
	// width number of characters to pad up to - if needed
	// left string is left aligned
	//_puts(buff, width, count, left);
	void _puts(char *s, int width, int count, int left)
	{
		int size = 0;
		int pad = 0;

		// note - if width > count we pad
		//        if width <= count we do not pad
		if(width > count)
		{
			pad = width - count;
		}

//printf("_puts:(%s) width:%d, count:%d, left:%d, pad:%d, len:%d\n", s, width, count, left, pad, len);

		// left padding ?
		if(!left)
		{
//printf("_puts:pad:%d\n", pad);
			while(pad--)
			{
				_putc(' ');
				++size;
			}
		}
//printf("_puts:count:%d\n", count);

		// string
		while(*s && count--)
		{
			_putc(*s);
			++s;
			++size;
		}
		// right padding
		if(left)
		{
//printf("_puts:pad:%d\n", pad);

			while(pad--) 
			{
				_putc(' ');
				++size;
			}
		}
//printf("_puts:size:%d\n", size);
		_putc(0);

	}	// _puts()
	// ====================================================================


    while(*fmt) 
	{
		// emit up to %
        if(*fmt != '%') 
		{
            _putc(*fmt++);
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
		while(*fmt == '-' || *fmt == ' ' | *fmt == '+')
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
			count = ftoa(dnum, buff, intprec, prec, sign);
			_puts(buff, width, count, left);
			break;
		case 'e':
			count = etoa(dnum, buff, prec, sign);
			_puts(buff, width, count, left);
			break;
#endif
//int t_itoa(unsigned long num, uint8_t *str, int prec, int sign)
		case 'u':
		case 'U':
			count = t_itoa(num, buff, sizeof(buff), prec, 0);
			_puts(buff, width, count, left);
			break;
//int t_itoa(unsigned long num, uint8_t *str, int prec, int sign)
		case 'd':
		case 'D':
//printf("<%ld, width:%d, prec:%d, sizeof(buff):%d, left:%d>\n", num, width, prec, (int)sizeof(buff), left);
			count = t_itoa(num, buff, sizeof(buff), prec, sign);
//printf("[%s, width:%d, count:%d, left:%d]\n", buff, width, count, left);
			_puts(buff, width, count, left);
			break;
//int t_t_ntoa(unsigned long num, uint8_t *str, int radix, int pad)
		case 'o':
		case 'O':
			count = t_ntoa(num, buff, sizeof(buff), 8, prec);
			_puts(buff, width, count, left);
			break;
		case 'x':
		case 'X':
			count = t_ntoa(num, buff, sizeof(buff), 16, prec);
			if(spec == 'X')
				t_strupper(buff);
			_puts(buff, width, count, left);
			break;
		case 'b':
		case 'B':
			count = t_ntoa(num, buff, sizeof(buff), 2, prec);
			_puts(buff, width, count, left);
			break;
		case 's':
			if(spec == 's')
			{
				ptr = va_arg(va, char *);
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
			count = t_strlen(ptr);
			if(prec)
				count = prec;
			if(count > width && width != 0)
				count = width;
//printf("width:%d,count:%d,left:%d\n", width, count, left);
			_puts(ptr, width, count, left);
			break;
		default:
			while(fmtptr <= fmt && *fmtptr)
				_putc(*fmtptr++);
			break;
		}
//printf("fmt:(%s)\n", fmt);
    }
//printf("fmt exit:(%s)\n", fmt);
    size = t_strlen(save);
//printf("buffer:(%s), size:%d\n", save, size);
    return( size );

}

/// @brief snprintf function
/// @param[out] buffer: string buffer for result
/// @param[in] buffer_len: maximum length of converted string
/// @param[in] fmt: printf forat string
/// @param[in] ...: vararg list or arguments
/// @return string size
MEMSPACE int
t_snprintf(char* buffer, int buffer_len, const char *fmt, ...)
{
	int ret;
	va_list va;
	va_start(va, fmt);
	ret = t_vsnprintf(buffer, buffer_len, fmt, va);
	va_end(va);

	return ret;
}



#ifdef TEST
/// @brief printf function
/// @param[in] fmt: printf forat string
/// @param[in] va_list: vararg list or arguments
/// @return size of printed string 
MEMSPACE int
t_printf(const char *fmt, ...)
{
	int ret;

	char buff[512];

	va_list va;
	va_start(va, fmt);
	ret = t_vsnprintf(buff, 510, fmt, va);
	va_end(va);

	printf("%s",buff);

	return ret;
}

/// @brief printf tests
/// Compare printf results from gcc printf and this printf
/// @return void
void tests()
{
puts("[%c]\\n, 'a'");
 t_printf("    [%c]\n", 'a');
   printf("    [%c]\n", 'a');
puts("\n");

puts("[%-20.2s]\\n, abc");
 t_printf("    [%-20.2s]\n", "abc");
   printf("    [%-20.2s]\n", "abc");
puts("\n");

puts("[%10.5s]\\n, abcdefg");
 t_printf("    [%10.5s]\n", "abcdefg");
   printf("    [%10.5s]\n", "abcdefg");
puts("\n");

puts("[%-+15.4e]\\n, 314.159265358979");
 t_printf("    [%-+15.4e]\n", 314.159265358979);
   printf("    [%-+15.4e]\n", 314.159265358979);
puts("\n");

puts("[%20.5e]\\n, 314.159265358979");
 t_printf("    [%20.5e]\n", 314.159265358979);
   printf("    [%20.5e]\n", 314.159265358979);
puts("\n");

puts("[%08.0f], 1.0");
 t_printf("    [%08.0f]\n", 1.0);
   printf("    [%08.0f]\n", 1.0);
puts("\n");

puts("[%08.4f], 0.0");
 t_printf("    [%08.4f]\n", 0.0);
   printf("    [%08.4f]\n", 0.0);
puts("\n");

puts("[%f], 0.0");
 t_printf("    [%f]\n", 0.0);
   printf("    [%f]\n", 0.0);
puts("\n");

puts("[%8.2f], 0.0");
 t_printf("    [%8.2f]\n", 0.0);
   printf("    [%8.2f]\n", 0.0);
puts("\n");

puts("[%08.4f], 12.89");
 t_printf("    [%08.4f]\n", 12.89);
   printf("    [%08.4f]\n", 12.89);
puts("\n");

puts("[%.2f], 1234567.89");
 t_printf("    [%.2f]\n", 1234567.89);
   printf("    [%.2f]\n", 1234567.89);
puts("\n");

puts("[%10.5f]\\n, 314.159265358979");
 t_printf("    [%10.5f]\n", 314.159265358979);
   printf("    [%10.5f]\n", 314.159265358979);
puts("\n");

puts("[%10.5f]\\n, 123456789012345678901234567890.159265358979");
 t_printf("    [%10.5f]\n", 123456789012345678901234567890.159265358979);
   printf("    [%10.5f]\n", 123456789012345678901234567890.159265358979);
puts("\n");


puts("[%+014.8f]\\n, 3.14159265358979");
 t_printf("    [%+014.8f]\n", 3.14159265358979);
   printf("    [%+014.8f]\n", 3.14159265358979);
puts("\n");

puts("[%14.8f]\\n, 3.141");
 t_printf("    [%14.8f]\n", 3.141);
   printf("    [%14.8f]\n", 3.141);
puts("\n");

puts("[%-+15d]\\n, 1234");
 t_printf("    [%-+15d]\n", 1234);
   printf("    [%-+15d]\n", 1234);
puts("\n");

puts("[%+15.8d]\\n, 1234");
 t_printf("    [%+15.8d]\n", 1234);
   printf("    [%+15.8d]\n", 1234);
puts("\n");

puts("[% -15.8d]\\n, 1234");
 t_printf("    [% -15.8d]\n", 1234);
   printf("    [% -15.8d]\n", 1234);
puts("\n");

puts("[%-+15.8d]\\n, 1234");
 t_printf("    [%-+15.8d]\n", 1234);
   printf("    [%-+15.8d]\n", 1234);
puts("\n");

puts("[%.3d]\\n, 12345");
 t_printf("    [%.3d]\n", 12345);
   printf("    [%.3d]\n", 12345);
puts("\n");

puts("[% 15.8d]\\n, 1234");
 t_printf("    [% 15.8d]\n", 1234);
   printf("    [% 15.8d]\n", 1234);
puts("\n");

puts("[%+015d]\\n, 1234567890");
 t_printf("    [%+015d]\n", 1234567890);
   printf("    [%+015d]\n", 1234567890);
puts("\n");

puts("[%+020d]\\n, 1234567890");
 t_printf("    [%+020d]\n", 1234567890);
   printf("    [%+020d]\n", 1234567890);
puts("\n");

puts("[%020d]\\n, 1234567890");
 t_printf("    [%020d]\n", 1234567890);
   printf("    [%020d]\n", 1234567890);
puts("\n");

puts("[% -20d]\\n, 1234567890");
 t_printf("    [% -20d]\n", 1234567890);
   printf("    [% -20d]\n", 1234567890);
puts("\n");

puts("[%- 20d]\\n, 1234567890");
 t_printf("    [%- 20d]\n", 1234567890);
   printf("    [%- 20d]\n", 1234567890);
puts("\n");

puts("[%-+20d]\\n, 1234567890");
 t_printf("    [%-+20d]\n", 1234567890);
   printf("    [%-+20d]\n", 1234567890);
puts("\n");

puts("[%-20d]\\n, 1234567890");
 t_printf("    [%-20d]\n", 1234567890);
   printf("    [%-20d]\n", 1234567890);
puts("\n");

puts("[%+-20d]\\n, 1234567890");
 t_printf("    [%+-20d]\n", 1234567890);
   printf("    [%+-20d]\n", 1234567890);
puts("\n");

puts("[%+20d]\\n, 1234567890");
 t_printf("    [%+20d]\n", 1234567890);
   printf("    [%+20d]\n", 1234567890);
puts("\n");

puts("[%20d]\\n, 1234567890");
 t_printf("    [%20d]\n", 1234567890);
   printf("    [%20d]\n", 1234567890);
puts("\n");

puts("[%-20.5d]\\n, 123");
 t_printf("    [%- 20.5d]\n", 123);
   printf("    [%- 20.5d]\n", 123);
puts("\n");

puts("[%- 10.5d]\\n, 123");
 t_printf("    [%- 10.5d]\n", 123);
   printf("    [%- 10.5d]\n", 123);
puts("\n");

puts("[%10.5d]\\n, 123");
 t_printf("    [% 10.5d]\n", 123);
   printf("    [% 10.5d]\n", 123);
puts("\n");

puts("[% - .5d]\\n, 123");
 t_printf("    [% -.5d]\n", 123);
   printf("    [% -.5d]\n", 123);
puts("\n");

puts("[% -5d]\\n, 123");
 t_printf("    [% -5d]\n", 123);
   printf("    [% -5d]\n", 123);
puts("\n");

puts("[%+-6d]\\n, 123");
 t_printf("    [%+06d]\n", 123);
   printf("    [%+06d]\n", 123);
puts("\n");

}

/// @brief main printf test programe
/// Run a number of conversion tests
/// @return 0
#define MAXSTR 256
int main(int argc, char *argv[])
{

	uint8_t str[MAXSTR+1];
	long num, num2, mask;
	int i;
	mask = 1;
	num = 0;


	tests();

	printf("1's\n");
	num = ~num;
	for(i=0;i<64;++i)
	{
		num &= ~mask;
		mask <<= 1;

		t_printf("[%+020ld]\n", num);
		printf("[%+020ld]\n", num);
		sprintf(str,"%+020ld", num);

		num2 = aton(str,10);
		if(num2 != num)
			printf("**:%ld\n",num2);

		t_ntoa(num, str, sizeof(str), 16, 16);
		printf("[%s]\n",str);
		t_ntoa(num, str, sizeof(str), 8, 22);
		printf("[%s]\n",str);
		t_ntoa(num, str, sizeof(str), 2, 64);
		printf("[%s]\n",str);

		printf("\n");
		
	}
	printf("=================================\n");
	printf("1's\n");
	num = 0;
	mask = 1;
	for(i=0;i<64;++i)
	{
		num |= mask;
		mask <<= 1;
		t_printf("[%+020ld]\n", num);
		printf("[%+020ld]\n", num);

		sprintf(str,"%+020ld", num);
		num2 = aton(str,10);
		if(num2 != num)
			printf("***:%ld\n",num2);

		t_ntoa(num, str, sizeof(str), 16, 16);
		printf("[%s]\n",str);
		t_ntoa(num, str, sizeof(str), 8, 22);
		printf("[%s]\n",str);
		t_ntoa(num, str, sizeof(str), 2, 64);
		printf("[%s]\n",str);

		printf("\n");
	}

	printf("=================================\n");
	printf("9's\n");
	num = 9;
	for(i=0;i<32;++i)
	{
		t_printf("[%+020ld]\n", num);
		printf("[%+020ld]\n", num);

		sprintf(str,"%+020ld", num);
		num2 = aton(str,10);
		if(num2 != num)
			printf("***:%ld\n",num2);

		t_ntoa(num, str, sizeof(str), 16, 16);
		printf("[%s]\n",str);
		t_ntoa(num, str, sizeof(str), 8, 22);
		printf("[%s]\n",str);
		t_ntoa(num, str, sizeof(str), 2, 64);
		printf("[%s]\n",str);

		printf("\n");

		if(num &  (1L << ((sizeof(num)*8)-1)))
			break;
		num *= 10;
		num += 9;
		
	}
	printf("\n");
	printf("=================================\n");


	printf("-1\n");
	num = -1;
	t_printf("[%+020ld]\n", num);
	printf("[%+020ld]\n", num);
	sprintf(str,"%+020ld", num);

	num2 = aton(str,10);
	if(num2 != num)
		printf("***:%ld\n",num2);

	t_ntoa(num, str, sizeof(str), 16, 0);
	printf("[%s]\n",str);
	t_ntoa(num, str, sizeof(str), 8, 0);
	printf("[%s]\n",str);
	t_ntoa(num, str, sizeof(str), 2, 0);
	printf("[%s]\n",str);

	printf("\n");
	printf("=================================\n");

	printf("1 << 63\n");
	num = 1UL;
	num <<= 63;

	t_printf("[%020lu]\n", num);
	printf("[%020lu]\n", num);
	sprintf(str,"%+020ld", num);

	num2 = aton(str,10);
	if(num2 != num)
		printf("***:%ld\n",num2);

	t_ntoa(num, str, sizeof(str), 16, 0);
	printf("[%s]\n",str);
	t_ntoa(num, str, sizeof(str), 8, 0);
	printf("[%s]\n",str);
	t_ntoa(num, str, sizeof(str), 2, 0);
	printf("[%s]\n",str);

	printf("\n");
	printf("=================================\n");

	printf("0\n");
	num = 0;
	t_printf("[%+020ld]\n", num);
	printf("[%+020ld]\n", num);
	sprintf(str,"%+020ld", num);

	num2 = aton(str,10);
	if(num2 != num)
		printf("***:%ld\n",num2);

	t_ntoa(num, str, sizeof(str), 16, 0);
	printf("[%s]\n",str);
	t_ntoa(num, str, sizeof(str), 8, 0);
	printf("[%s]\n",str);
	t_ntoa(num, str, sizeof(str), 2, 0);
	printf("[%s]\n",str);

	printf("\n");
	printf("=================================\n");

	printf("0\n");
	num = 0;
	t_printf("[%+ld]\n", num);
	printf("[%+ld]\n", num);
	sprintf(str,"%+020ld", num);

	num2 = aton(str,10);
	if(num2 != num)
		printf("***:%ld\n",num2);

	t_ntoa(num, str, sizeof(str), 16, 0);
	printf("[%s]\n",str);
	t_ntoa(num, str, sizeof(str), 8, 0);
	printf("[%s]\n",str);
	t_ntoa(num, str, sizeof(str), 2, 0);
	printf("[%s]\n",str);

	printf("\n");
	printf("=================================\n");

	return(0);
}
#endif	
