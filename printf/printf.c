
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

#ifdef USER_CONFIG
#include "user_config.h"
#else
#include <stdio.h>
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "mathio.h"

// =======================================================================

// Below we included functions that are defined elsewhere
// They are included within printf to allow this to be standalone
// This is done by looking at header defines for the time being

/// @brief String Length
/// @param[in] str: string
/// @return string length
MEMSPACE
size_t 
WEAK_ATR
strlen(const char *str)
{
    int len=0;
    // String length
    while(*str++)
        ++len;
    return(len);
}

// Skip if we have linux ctype.h
/// @brief test if a character is a digit
/// @param[in] c: character
/// @return true or false
#undef isdigit
MEMSPACE 
int
WEAK_ATR
isdigit(int c)
{
	if(c >= '0' && c <= '9')
		return(1);
	return(0);
}

// =======================================================================
// start of support functions
// =======================================================================

/// @brief Reverse a string in place
///  Example: abcdef -> fedcba
/// @param[in] str: string 
/// @return string length
MEMSPACE 
void
WEAK_ATR
reverse(char *str)
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
void 
WEAK_ATR
strupper(char *str)
{

	while(*str)
	{
		if(*str >= 'a' && *str <= 'z')
			*str -= 0x20;
		++str;
	}
}

// ======================================================================
/// @brief Data structure for character buffer with limits
typedef struct {
	char *str; 			///@brief base of string to write to
	int ind; 			///@brief current string index
	int max; 			///@brief maximum string size including EOS
} p_ch_t;

/// @brief Define data structure for character buffer with limits
p_ch_t _pch;

/// @brief Initialize character buffer with limits
/// @param[in] str: string
/// @param[in] max: maximum number of characters plus EOS
/// @return void
MEMSPACE
void pch_init(char *str, int max)
{
	_pch.str = str;		///@brief base of string to write to
	_pch.ind = 0;		///@brief current string index
	_pch.max = max-1;	///@brief maximum string size including EOS
	_pch.str[0] = 0;
}


/// @brief Put character in buffer with limits
/// @param[in] str: string
/// @param[in] max: maximum number of characters plus EOS
/// @See init_p_ch
/// @return void
MEMSPACE
int pch(char ch)
{
	// Add the character while ther is room
	if(_pch.ind < _pch.max)
		_pch.str[_pch.ind++] = ch;
	else
		_pch.str[_pch.ind] = 0;	// Add EOS when limit exceeded
	return(_pch.ind);
}

/// @brief Return current index of character buffer with limits
/// @return Buffer index
MEMSPACE
int pch_ind()
{
	return(_pch.ind);
}

/// @brief Return maximum valid index for character buffer 
/// @return Buffer index
MEMSPACE
int pch_max_ind()
{
	return(_pch.max);
}

// ======================================================================
// end of support functions
// ======================================================================
/// @brief print flags set in t_t structure
/// @param[in] f: f_t flags structure
/// @return void
MEMSPACE
void print_flags(f_t f)
{
	if(f.b.left)
		printf("left   flag\n");
	if(f.b.plus)
		printf("plus   flag\n");
	if(f.b.space)
		printf("space  flag\n");
	if(f.b.zero)
		printf("zero   flag\n");
	if(f.b.alt)
		printf("alt    flag\n");
	if(f.b.width)
		printf("width  flag\n");
	if(f.b.prec)
		printf("prec   flag\n");
	if(f.b.neg)
		printf("< 0    flag\n");
}


#ifdef TEST_PRINTF
#ifdef DEFINE_PRINTF
#error DEFINE_PRINNTF must not be defined when testing
#endif
#endif
/// @brief Convert number to ASCII
/// returns size of string after conversion
/// Note: It is allowed to have more digits converted then prec defines
/// Like itoa but can support leading '+/-/ ' or unsigned and zero padding
/// @param[in] num: number
/// @param[out] *str: string
/// @param[in] max: maximum length or string
/// @param[in] prec:  minumum number of digits, 0 padded if needed (can be zero)
/// @param[in] sign:  3: ' ' - or -, 2: - or +, 1: - - or '', 0: unsigned
///            NOT counted as part of prec length (just like printf)
/// @return long value
MEMSPACE 
int p_itoa(unsigned long num, char *str, int max, int width, int prec, f_t f)
{
		int digit,sign_ch;
		int ind;
		int digits = 0;	/* the minumum number of digits to display not including sign */
        char *save = str;

		digits = 0;

		pch_init(str,max);

		sign_ch = 0;
		// If the number we always have a leading chracter of '-', ' ' or '+'

		if(f.b.neg)
			sign_ch = '-';
		else if(f.b.plus)
			sign_ch = '+';
		else if(f.b.space)
			sign_ch = ' ';

//printf("itoa: num:%ld, max:%d, width:%d, prec:%d, digits:%d, sign_ch:%02x\n", num, max, width, prec, digits, sign_ch);
//print_flags(f);
		/* Some Combinations of flags are not permitted 
         * - or impact the interpretation of others */
		if(f.b.zero)
		{
			// 0 disabled if precision or left align
			if(f.b.left || f.b.prec)
				f.b.zero = 0;
		}

		if(f.b.prec)
		{
			digits = prec;
#if 0
			// displaying a sign character may reduce the number of digits
			if(!f.b.left && width <= prec)
			{
				/* make room for a sign ? */
				if(f.b.plus || f.b.neg || f.b.space)
					--digits;
			}
#endif
		}

		if(!f.b.width && !f.b.prec)
			digits = 1;	// at least one digit

		if(f.b.width)
		{
			if(!f.b.zero)
			{
				// Width and no leading zeros require at least one digit
				if(!f.b.prec)
					digits = 1;	// at least one digit
			}
			else	/* precision and 0 can not occur together - previously tested */
			{
				digits = width;
				/* make room for a sign ? */
				if(f.b.plus || f.b.neg || f.b.space)
					--digits;
			}
		}

		// Convert number LSB to MSB
		// FIXME if the number is bigger then the buffer size(max)
		// both digits and the sign may be lost
		while(num || digits > 0)
		{
			digit = num % 10;
			num /= 10;
			if(digit < 0)	// num = -num can fail for 2**n-1
				digit = -digit;
			pch(digit + '0');
			--digits;
		}

		if(sign_ch)
			pch(sign_ch);
		pch(0);
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
int p_ntoa(unsigned long num, char *str, int max, int radix, int width, int prec, f_t f)
{
		unsigned int sign_ch, mask,shift,digit;
		int digits;
        char *save = str;

		digits = 0;

		f.b.space = 0;
		f.b.plus = 0;
		f.b.neg = 0;

		pch_init(str,max);


		sign_ch = 0;
		// If the number we always have a leading chracter of '-', ' ' or '+'


		/* Some Combinations of flags are not permitted 
         * - or impact the interpretation of others */
		if(f.b.zero)
		{
			// 0 disabled if precision or left align
			if(f.b.left || f.b.prec)
				f.b.zero = 0;
		}


		if(f.b.prec)
		{
			digits = prec;
		}

		if(!f.b.width && !f.b.prec)
			digits = 1;	// at least one digit

		if(f.b.width)
		{
			if(!f.b.zero)
			{
				// Width and no leading zeros require at least one digit
				if(!f.b.prec)
					digits = 1;	// at least one digit
			}
			else	/* precision and 0 can not occur together - previously tested */
			{
				digits = width;
			}
		}
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
		// Convert number LSB to MSB
		// FIXME if the number is bigger then the buffer size(max)
		// both digits and the sign may be lost
		while(num || digits > 0)
		{
			digit = num & mask;
			num >>= shift;
			// convert 
			if(digit < 10)
				digit += '0';
			else
				digit += ('a'  - 10);
			pch(digit);
			--digits;
		}

		if(sign_ch)
			pch(sign_ch);
		pch(0);

        reverse(save);
		return(strlen(save));
}


#ifdef FLOATIO
/// @brief float to ASCII 
/// @param[in] val: value
/// @param[in] str: converted string
/// @param[in] width: field width is the minimum number of characters in converted string
/// @param[in] prec: number of digits emitted after after the radix point
/// @param[in] sign: sign type flag
/// @return size of string
MEMSPACE 
int p_ftoa(double val, char *str, int max, int width, int prec, f_t f)
{

	char *save = str;
	double iscale, fscale;
	double ival, fval;
	int idigits, fdigits, digits;
	int digit;

	pch_init(str,max);

/* FIXME 
Notice rounding - testing will be tricky
ERROR: [% 15.1f], [-10252956608208.250000]
    G[-10252956608208.2]
    B[-10252956608208.3]
    error:9.90567929632045986708e-15

*/
//printf("val:%.20e\n", val);
//print_flags(f);
	if(val < 0.0)
	{
		val = -val;
		f.b.neg = 1;
	}
	if(f.b.neg)
		pch('-');
	else if(f.b.plus)
		pch('+');
	else if(f.b.space)
		pch(' ');

	// prec only applies to fractional digits
	if(prec < 0)
		prec = 0;

	// NOTE: prec is anchored at the decimal point

	fscale = 0.0;
	idigits = 1;	// number of integer digits 
	if(val)	// if zero no rounding needed
	{
		// do rounding - IF precision specified
		// number of fractional digits to display
		fscale = 0.5;	// rounding value if prec = 0
		if(f.b.prec)
		{
			digits = prec;
			while(digits > 0)
			{
				fscale /= 10.0;	// adjust rounding value
				--digits;
			}
		}
//printf("val;:%.20e, fscale:%.20e\n", val, fscale);
		val += fscale;		// round to prec

		while(val >= 10.0)
		{
			++idigits;
			val /= 10.0;
		}
		// So we know that fval < 1.0;
	}

//printf("idigits:%d\n",idigits);

	if(f.b.zero && !f.b.left)
	{
		if(f.b.prec && prec)
			digits = width - idigits - pch_ind() - prec -1;	
		else
			digits = width - idigits - pch_ind();

		while(digits > 0)
		{
			pch('0');
			--digits;
		}
	}

	// Display integer part of number
	while(idigits > 0)
	{
		digit =	val;
//printf("ival:%.16e, int:%d\n", ival, digit);
		pch(digit + '0');
		val -= (double) digit;
		--idigits;
		val *= 10.0;
	}
	// display fractional part	
	if(f.b.prec && prec > 0 )
	{
		pch('.');
		while(prec > 0 )
		{
			digit =	val;
			val -= (double) digit;
			digit += '0';
			pch(digit);
			--prec;
			val *= 10.0;
		}
	}
	pch(0);
	return(strlen(save));
}



/// @brief float to ASCII 
/// @param[in] x: value
/// @param[in] str: converted string
/// @param[in] prec: digits after decimal place
/// @param[in] sign: sign
/// @return size of string
MEMSPACE 
int p_etoa(double val,char *str, int max, int width, int prec, f_t f)
{
	char *save = str;
	double fscale;
	int digits;
	int digit;
	int exp10 = 0;

	pch_init(str,max);

/* FIXME 
Notice rounding - testing will be tricky
ERROR: [% 15.1f], [-10252956608208.250000]
    G[-10252956608208.2]
    B[-10252956608208.3]
    error:9.90567929632045986708e-15

*/
//printf("val:%.20e\n", val);
//print_flags(f);
	if(val < 0.0)
	{
		val = -val;
		f.b.neg = 1;
	}
	if(f.b.neg)
		pch('-');
	else if(f.b.plus)
		pch('+');
	else if(f.b.space)
		pch(' ');

	// prec only applies to fractional digits
	if(prec < 0)
		prec = 0;

	// NOTE: prec is anchored at the decimal point

	fscale = 1.0;	// rounding value if prec = 0
	exp10 = 0;

	if(val)	// if zero no rounding needed
	{
		fscale = 0.5;	// rounding value if prec = 0
		// do rounding - IF precision specified
		// number of fractional digits to display
		if(f.b.prec)
		{
			digits = prec;
			while(digits > 0)
			{
				fscale /= 10.0;	// adjust rounding value
				--digits;
			}
		}

		// scale number  1.0 <= x < 10.0
		while(val < 1.0 )
		{
			val *= 10.0;
			exp10--;

		}
		while(val >= 10.0)
		{
			++exp10;
			val /= 10.0;
		}
		val += fscale;	
		while(val >= 10.0)
		{
			++exp10;
			val /= 10.0;
		}
		// round
		//printf("val;:%.20e, fscale:%.20e\n", val, fscale);
		//val += fscale;		// round to prec
	}

	// [+]N.FFe+00, where ".e+00" = 5 digits, pch_ind holds optional sign offset
	if(f.b.zero && !f.b.left)
	{
		if(f.b.prec && prec)
			digits = width - pch_ind() - prec - 6;	
		else
			digits = width - pch_ind() - 5;
		while(digits > 0)
		{
			pch('0');
			--digits;
		}
	}

	digit =	val;
//printf("ival:%.16e, int:%d\n", ival, digit);
	pch(digit + '0');
	val -= (double) digit;
	val *= 10.0;

	// display fractional part	
	if(f.b.prec && prec > 0 )
	{
		pch('.');
		while(prec > 0 )
		{
			digit =	val;
			val -= (double) digit;
			digit += '0';
			pch(digit);
			val *= 10.0;
			--prec;
		}
	}
    pch('e') ;
    if ( exp10 < 0 ) 
	{ 
		exp10 = -exp10; 
		pch('-'); 
	}
	else	
	{
		pch('+'); 
	}
    pch('0' + exp10 / 10);
    pch('0' + exp10 % 10);
    pch(0);
	return(strlen(str));
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
void _printf_fn(printf_t *fn, __memx const char *fmt, va_list va)
{
    int prec, width, intprec;
	int count;
    int spec;
	int size;
	long num = 0;
	int inum = 0;
	f_t f;
#ifdef FLOATIO
	double dnum = 0;
#endif
	char chartmp[2];
	char *ptr;
	__memx const char *fmtptr;

	// buff has to be at least as big at the largest converted number
	// in this case base 2 long long with sign and end of string
	char buff[sizeof( long ) * 2 * 8 + 2];

    while(*fmt) 
	{
		// emit up to %
        if(*fmt != '%') 
		{
            fn->put(fn,*fmt++);
			continue;
        }

		fmtptr = fmt;
		// process % specifier
		fmt++;

		prec = 0;	// minimum number of digits displayed 
		width = 0;	// padded width
		intprec = 0;// integer number of digits

		size = sizeof(int); // short=1,int=2,long=3 , int is default

		// we accept multiple flag combinations	and duplicates as does GLIBC printf
		// ['#']['-'][' '|'+']
		// [' '|'+']['-']['#']
		// ...

		// reset flags
		f.all = 0;
		while(*fmt == '#' || *fmt == '+' || *fmt == '-' || *fmt == ' ' || *fmt == '0')
		{
			if(*fmt == '#') 
				f.b.alt = 1;
			else if(*fmt == '+') 
				f.b.plus = 1;
			else if(!f.b.left && *fmt == '-') 
				f.b.left = 1;
			else if(!f.b.space && *fmt == ' ') 
				f.b.space = 1;
			else if(!f.b.zero && *fmt == '0') 
				f.b.zero = 1;
			// format error
			++fmt;
		}

		// width specifier 
		// Note: we permit zero as the first digit
		if(isdigit(*fmt))
		{
			// optional width
			width = 0;
			while(isdigit(*fmt))
				width = width*10 + *fmt++ - '0';
			f.b.width = 1;
		}

		// prec always impiles zero fill to prec digigits for ints and longs
		//      is the number of digits after the . for float and double
		// regardlles of sign
		if( *fmt == '.' ) 
		{
			fmt++;
			prec = 0;
			while(isdigit(*fmt))
				prec = prec*10 + *fmt++ - '0';
			f.b.prec = 1;
		}

#if 0
		// short
		if(*fmt == 'h') 
		{
			fmt++;
			size = sizeof(short);
		}
#endif
		// long
		if(*fmt == 'l') 
		{
			fmt++;
			size = sizeof(long);
		}
		// FIXME todo long long

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
			case 'p':
						if(f.b.zero && f.b.left)
							f.b.zero = 0;
						if(f.b.zero && f.b.prec)
							f.b.zero = 0;
						if(f.b.zero && f.b.width)
						{
							if(width > prec)
								prec = width;
						}
						if(f.b.zero && f.b.width && f.b.prec)
						{
							if(width > prec)
								prec = width;
						}
			case 'u':
			case 'U':
			case 'D':
			case 'd':
				// only reached if sizeof short != sizeof int
				if(size == sizeof(int))
					num = va_arg(va, int);
				else if(size == sizeof(long))
					num = va_arg(va, long);
#if 0
				if(size == sizeof(short))
					num = va_arg(va, short);
#endif

				if(spec == 'd' || spec =='D')
				{
					if(num < 0)
					{
						f.b.neg = 1;
						if(size == sizeof(int))
							num = (int) -num;
						else
							num = -num;
					}
				}
				else
				{
					// mask resolution 
					if(size == sizeof(int) )
						num &= (unsigned int) -1;
#if 0
					if(size == sizeof(short))
						num &= (unsigned short) -1;
#endif
				}
				++fmt;
				break;
#ifdef FLOATIO
			case 'f':
			case 'F':
			case 'e':
			case 'E':
				// FIXME K&R defines 'f' type as 6 - and matches GNU printf
				if(!f.b.prec)
				{
					prec = 6;
					f.b.prec = 1;
				}
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
		case 'u':
		case 'U':
			f.b.space = 0;
			f.b.plus = 0;
			// FIXME sign vs FILL
			count = p_itoa(num, buff, sizeof(buff), width, prec, f);
			_puts_pad(fn,buff, width, count, f.b.left);
			break;
			// FIXME sign vs FILL
		case 'd':
		case 'D':
//printf("<%ld, width:%d, prec:%d, sizeof(buff):%d, left:%d>\n", num, width, prec, (int)sizeof(buff), f.b.left);
			count = p_itoa(num, buff, sizeof(buff), width, prec, f);
//printf("[%s, width:%d, count:%d, left:%d]\n", buff, width, count, f.b.left);
			_puts_pad(fn,buff, width, count, f.b.left);
			break;
		case 'b':
		case 'B':
			count = p_ntoa(num, buff, sizeof(buff), 2, width, prec,f);
			if(spec == 'X' || spec == 'P')
				strupper(buff);
			_puts_pad(fn,buff, width, count, f.b.left);
			break;
		case 'o':
		case 'O':
			count = p_ntoa(num, buff, sizeof(buff), 8, width, prec,f);
			if(spec == 'X' || spec == 'P')
				strupper(buff);
			_puts_pad(fn,buff, width, count, f.b.left);
			break;
		case 'x':
		case 'X':
		case 'p':
		case 'P':
			count = p_ntoa(num, buff, sizeof(buff), 16, width, prec,f);
			if(spec == 'X' || spec == 'P')
				strupper(buff);
			_puts_pad(fn,buff, width, count, f.b.left);
			break;
#ifdef FLOATIO
		case 'f':
		case 'F':
			count = p_ftoa(dnum, buff, sizeof(buff), width, prec, f);
			if(spec == 'E')
				strupper(buff);
			_puts_pad(fn,buff, width, count, f.b.left);
			break;

		case 'e':
		case 'E':
			count = p_etoa(dnum, buff, sizeof(buff), width, prec, f);
			if(spec == 'E')
				strupper(buff);
			_puts_pad(fn,buff, width, count, f.b.left);
			break;
#endif
		case 's':
		case 'c':
			ptr = NULL; // stops bogus error that ptr may be uninitalized
			if(spec == 's')
			{
				ptr = va_arg(va, char *);
				if(!ptr)
					ptr = "(null)";
			}
			else // 'c'
			{
				chartmp[0] = (char) va_arg(va, int);
				chartmp[1] = 0;
				ptr = chartmp;
			}
			count = strlen(ptr);
			if(prec)
				count = prec;
			if(count > width && width != 0)
				count = width;
//printf("width:%d,count:%d,left:%d\n", width, count, f.b.left);
			_puts_pad(fn,ptr, width, count, f.b.left);
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
/// @brief _putc_buffer_fn - character output to a string buffer
/// Used by snprintf and vsnprintf
/// You can make _printf_fn call this helper for each character
/// @param[in] *p: structure with pointers and buffer to be written to
/// @param[in] ch: character to place in buffer
/// @return void
MEMSPACE
void _putc_buffer_fn(struct _printf_t *p, char ch)
{
	char *str;
	if (p->len )
	{
		if(ch)
		{
			p->len--;
			p->sent++;
			str = (char *) p->buffer;
			*str++ = ch;
			p->buffer = (void *) str;
		}
	}
	*((char *)p->buffer) = 0;
}   

#ifndef PRINTF_TEST
// ====================================================================
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
    fn.sent = 0;
    fn.buffer = (void *) str;

    _printf_fn(&fn, format, va);

    // FIXME check size should == fn.size on exit
    len = strlen(save);
    return( len );
}

// ====================================================================
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
    len = vsnprintf(str, size, format, va);
    va_end(va);

    return len;
}


// ====================================================================
/// @brief sprintf function is not recommended because it can overflow
// ====================================================================

#ifdef DEFINE_PRINTF
// ====================================================================
/// @brief _putc_fn - character output to fputs(c,stdout)
/// You can make _printf_fn call this helper for each character
/// @param[in] *p: structure function pointer, buffer, len and size 
/// @param[in] ch: character to write
/// @return void
MEMSPACE
static void _putc_fn(struct _printf_t *p, char ch)
{
	p->sent++;
	putchar(ch);
}

/// @brief printf function
///  Example user defined printf function using fputc for I/O
///  This method allows I/O to devices and strings without typical C++ overhead
/// @param[in] fmt: printf forat string
/// @param[in] va_list: vararg list or arguments
/// @return size of printed string
/// TODO create a devprintf using an array of function prointers ?
MEMSPACE 
int 
printf(const char *format, ...)
{
	int i;
	printf_t fn;
	va_list va;

	fn.put = _putc_fn;
	fn.sent = 0;

	va_start(va, format);
	_printf_fn(&fn, format, va);
	va_end(va);

	return ((int)fn.sent);
}
#endif
#endif
