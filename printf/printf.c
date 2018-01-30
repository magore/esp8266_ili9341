
/**
 @file printf.c

 @brief Small printf, and verious conversion code with floating point support

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL  License
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for specific Copyright details

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
//#include <stdio.h>
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "mathio.h"

// =============================================

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

// =============================================
// start of support functions
// =============================================

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
            *str -= ( 'a' - 'A');
        ++str;
    }
}

#if __BYTE_ORDER__ != __ORDER_LITTLE_ENDIAN__
#error bin2num ASSUME little endian
#endif

/// @brief Convert an unsigned number (numsize bytes in size) to ASCII in specified base
/// Notes: No limit except available memory on number size
///   - Does not use divide or of multiply instructions - great on 8bit CPU's
/// How it works: Say you have a table of powers of 2 in a given base base
///   - To get the result add each table entry, in the base, that corresponds 
///   to each 1 bit in the binary number.
///   - Now instead of a table we can start with 1 and multiple by 2 contraining each
///   digit into the base we want - this is the same as using the table except we build it on the fly
/// @param[out] str: ASCII number string result
/// @param[in] strmax: maximum size of str in bytes
/// @param[in] nummin: minimum number of digits to display
/// @param[in] *nump: Pointer to binary number 
/// @param[in] numsize: size of binary number in bytes
/// @param[in] sign_ch: sign of binary number 
/// return converted number string numsize in bytes
MEMSPACE
int bin2num(uint8_t *str, int strmax, int nummin, int base, uint8_t *nump, int numsize, int sign_ch)
{
    int i,j,carry;
    uint8_t data;

    int numbits = numsize * 8;

    for(i=0;i<=nummin;++i)
        str[i] = 0; // initial string starts out empty

    // FIXME Little/Big-endian
    // Loop for all bits (numsize in total) in the binary number (bin)
    // Examine each bit MSB to LSB order
    for(i = numbits - 1; i>= 0; --i)
    {
        // We extract 1 bit at a time from the binary number (MSB to LSB order) 
        //  FIXME Little/Big-endian
        data = nump[i>>3];
        // If extracted bit was set carry = 1, else 0
        //  FIXME Little/Big-endian
        carry = ( data & (1 << (i & 7)) ) ? 1 : 0;

        // Multiply base number by two and add the previously extracted bit
        // Next do base digit to digit carries as needed
        // Note: nummin is the current string size that can grow as needed
        // Carry test in the loop can optionally extend base strings size 
        for(j=0;(j<nummin || carry) ;++j)
        {
            if(j >= (strmax - 2))
                break;

            data = str[j];
            data = (data<<1) | carry;
            // process base carry
            carry = 0;
            if(data >= base)
            {
                data -= base;
                carry = 1;
            }
            str[j] = data;
        }
        str[j] = 0;     // zero next digit if carry extends size
        nummin = j;     // update nummin if carry extends size
    }

    // Add ASCII '0' or 'a' offsets to base string
    for(i=0;i<nummin;++i)
    {
        if(str[i] < 10)
            str[i] += '0';
        else str[i] += 'a'-10;
    }

    // Add optional sign character
    if(sign_ch && i <= (strmax - 2))
    {
        str[i++] = sign_ch;
        ++nummin;
    }
    str[i] = 0;     // Terminate string eith EOS
        
    reverse((char *)str);   // Reverse in place to correct order

    return(nummin); // Return string size
}

// =============================================
/// @brief Data structure for character buffer with limits
typedef struct {
    char *str;          ///@brief base of string to write to
    int ind;            ///@brief current string index
    int max;            ///@brief maximum string size including EOS
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
    _pch.str = str;     ///@brief base of string to write to
    _pch.ind = 0;       ///@brief current string index
    _pch.max = max-1;   ///@brief maximum string size including EOS
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
        _pch.str[_pch.ind] = 0; // Add EOS when limit exceeded
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

// =============================================
// end of support functions
// =============================================
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

/// @brief Convert number an base 2 .. 16 to ASCII with optional sign
/// Notes:
/// 1) Numbers can be any number of digits long - limited only by memory available
///      To print negative numbers convert to positive before calling this function and set f.b.neg 
/// 2) Warning: as with printf width and prec are only minimum sizes - results can be bigger
/// We assume all numbers are positive:
/// @param[in] nump: number pointer
/// @param[in] numsize: number size in bytes 
/// @param[out] *str: string result
/// @param[in] strmax: strmaximum length of string result
/// @param[in] radix:  Radix may be 2 .. 16
/// @param[in] width:  Width of result padded if needed
/// @param[in] prec:  minumum number of digits, zero padded if needed
/// @param[in] f:  flags
///     f.b.left   justify left 
///     f.b.plus   display + for positive number, - for negative numbers
///     f.b.space  display ' ' for positive, - for negative
///     f.b.zero   pad with zeros if needed
///     f.b.alt    Alternate display form - work in progress
///     f.b.width  Width of result - pad if required
///     f.b.prec   Zero padd to prec if sepcified
///     f.b.neg    Sign of number is negative

MEMSPACE 
int p_ntoa(uint8_t *nump, int numsize, char *str, int strmax, int radix, int width, int prec, f_t f)
{
        unsigned int sign_ch;
        int ind;
        int digits;

        digits = 0;

        // Unsigned, hex,octal,binary should leave these flasg zero
            //f.b.space = 0;
            //f.b.plus = 0;
            //f.b.neg = 0;

        sign_ch = 0;
        if(f.b.neg)
            sign_ch = '-';
        else if(f.b.plus)
            sign_ch = '+';
        else if(f.b.space)
            sign_ch = ' ';

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
            digits = prec;

        if(!f.b.width && !f.b.prec)
            digits = 1; // at least one digit

        if(f.b.width)
        {
            if(!f.b.zero)
            {
                // Width and no leading zeros require at least one digit
                if(!f.b.prec)
                    digits = 1; // at least one digit
            }
            else    /* precision and 0 can not occur together - previously tested */
            {
                digits = width;
                /* make room for a sign ? */
                if(f.b.plus || f.b.neg || f.b.space)
                    --digits;
            }
        }
        ind = bin2num((uint8_t *)str, strmax, digits, radix, nump, numsize, sign_ch);
        return(ind);
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
    double fscale;
    int idigits, digits;
    int digit;

    pch_init(str,max);

/* 
FIXME 
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
    idigits = 1;    // number of integer digits 
    if(val) // if zero no rounding needed
    {
        // do rounding - IF precision specified
        // number of fractional digits to display
        fscale = 0.5;   // rounding value if prec = 0
        if(f.b.prec)
        {
            digits = prec;
            while(digits > 0)
            {
                fscale /= 10.0; // adjust rounding value
                --digits;
            }
        }
//printf("val;:%.20e, fscale:%.20e\n", val, fscale);
        val += fscale;      // round to prec

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
        digit = val;
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
            digit = val;
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
    double fscale;
    int digits;
    int digit;
    int exp10;
    uint8_t exp10_str[7];   // +E123  and EOS
    int  expsize;
    int i;
    int sign_ch;

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

    fscale = 1.0;   // rounding value if prec = 0
    exp10 = 0;

    if(val) // if zero no rounding needed
    {
        fscale = 0.5;   // rounding value if prec = 0
        // do rounding - IF precision specified
        // number of fractional digits to display
        if(f.b.prec)
        {
            digits = prec;
            while(digits > 0)
            {
                fscale /= 10.0; // adjust rounding value
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
        //val += fscale;        // round to prec
    }

    // ====================
    // Exponent
    exp10_str[0] = 'e';
    if ( exp10 < 0 ) 
    { 
        sign_ch = '-';
        exp10 = -exp10; 
    }
    else    
    {
        sign_ch = '+';
    }
    // result is +NN[N] if we have three digits shorten digits by one
    expsize = bin2num(exp10_str+1, sizeof(exp10_str)-1-1, 2, 10, (uint8_t *) &exp10, sizeof(int), sign_ch);

#if 0
    if ( exp10 < 0 ) 
    { 
        exp10_str[1] = '-';
        exp10 = -exp10; 
    }
    else    
    {
        exp10_str[1] = '+';
    }
    if(exp >= 100)
        ind = 4;
    else
        ind = 3;

    exp10_str[ind] = '0' + (exp % 10);
    exp10 /= 10;
    exp10_str[ind] = '0' + (exp % 10);
    exp10 /= 10;
    exp10_str[ind] = '0' + (exp % 10);
#endif
    
    // ====================

    // [+]N.FFe+00, where ".e+00" = 5 digits, pch_ind holds optional sign offset
    if(f.b.zero && !f.b.left)
    {
        if(f.b.prec && prec)
            digits = width - pch_ind() - prec - 6;  
        else
            digits = width - pch_ind() - 5;
        if(expsize > 3)
            --digits;
        while(digits > 0)
        {
            pch('0');
            --digits;
        }
    }


    // Number
    digit = val;
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
            digit = val;
            val -= (double) digit;
            digit += '0';
            pch(digit);
            val *= 10.0;
            --prec;
        }
    }

    for(i=0;exp10_str[i];++i)
        pch(exp10_str[i]);

    pch(0);
    return(strlen(str));
}
              
#endif



// =============================================
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
}   // _puts_pad()






/// @brief vsnprintf function
/// @param[out] fn: output character function pointer 
/// @param[in] fmt: printf forat string
/// @param[in] va: va_list arguments
/// @return size of string
MEMSPACE 
void _printf_fn(printf_t *fn, __memx const char *fmt, va_list va)
{
    int prec, width;
    int count;
    int spec;
    int size;
    int sign;
    short nums;
    int numi;
    long numl;
    long long numll;
    void * numv;
#ifdef __SIZEOF_INT128__
    __uint128_t num128;
#endif
    uint8_t *nump;

    f_t f;
#ifdef FLOATIO
    double dnum = 0;
#endif
    char chartmp[2];
    char *ptr;
    __memx const char *fmtptr;

    // buff has to be at least as big at the largest converted number
    // in this case base 2 long long with sign and end of string
#ifdef PRINTF_TEST
    char buff[307 * 2 + 5 + 1]; // double to full size
#else
    char buff[sizeof( long long ) * 8 + 2];
#endif

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

        prec = 0;   // minimum number of digits displayed 
        width = 0;  // padded width


        // we accept multiple flag combinations and duplicates as does GLIBC printf
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

/** Calling Variadic Functions 
  - exceprt from https://www.gnu.org/software/libc/manual/html_node/Calling-Variadics.html
Since the prototype doesn’t specify types for optional arguments, in a call to a variadic function the default argument promotions are performed on the optional argument values. This means the objects of type char or short int (whether signed or not) are promoted to either int or unsigned int, as appropriate; and that objects of type float are promoted to type double. So, if the caller passes a char as an optional argument, it is promoted to an int, and the function can access it with va_arg (ap, int).
*/

        size = sizeof(int); // int is default

        if( *fmt == 'I' ) 
        {
            fmt++;
            size = 0;
            while(isdigit(*fmt))
                size = size*10 + *fmt++ - '0';
            if(size == 0 || size & 7)
                size = 0;
            else
                size >>= 3;
        }
        else if(*fmt == 'h')
        {
            fmt++;
            size = sizeof(short);
        }
        else if(*fmt == 'l') 
        {
            fmt++;
            size = sizeof(long);
            if(*fmt == 'l')
            {
                fmt++;
                size = sizeof(long long);
            }
        }

        if(size)
            spec = *fmt;
        else
            spec = 0;

        sign = 0;
        if(spec == 'd' || spec == 'D')
            sign = 1;

    
        nump = (uint8_t *) &numi;

        // process integer arguments
        switch(spec) 
        {
            case 'p':
            case 'P':
                size = sizeof(void *);
            // Unsigned numbers
            case 'b':
            case 'B':
            case 'o':
            case 'O':
            case 'x':
            case 'X':
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
                f.b.space = 0;
                f.b.plus = 0;
                f.b.neg = 0;
            case 'D':
            case 'd':
                // make lint shut up
//FIXME vararg functions promote short - make this a conditional
                if(size == sizeof(short))
                {
                    nums = (short) va_arg(va, int);
                    if(sign && nums < 0)
                    {
                        f.b.neg = 1;
                        nums = -nums;
                    }
                    nump = (uint8_t *) &nums;
                }
                else if(size == sizeof(int))
                {
                    numi = (int) va_arg(va, int);
                    if(sign && numi < 0)
                    {
                        f.b.neg = 1;
                        numi = -numi;
                    }
                    nump = (uint8_t *) &numi;
                }
                else if(size == sizeof(long))
                {
                    numl = (long) va_arg(va, long);
                    if(sign && numl < 0)
                    {
                        f.b.neg = 1;
                        numl = -numl;
                    }
                    nump = (uint8_t *) &numl;
                }
                else if(size == sizeof(long long))
                {
                    numll = (long long) va_arg(va, long long);  
                    if(sign && numll < 0)
                    {
                        f.b.neg = 1;
                        numll = -numll;
                    }
                    nump = (uint8_t *) &numll;
                }
#ifdef __SIZEOF_INT128__
                else if(size == sizeof(__uint128_t))
                {
                    num128 = (__uint128_t) va_arg(va, __uint128_t); 
                    if(sign && numll < 0)
                    {
                        f.b.neg = 1;
                        num128 = -128;
                    }
                    nump = (uint8_t *) &num128;
                }
#endif
                else if(size == sizeof(void *))
                {
                    numv = (void *) va_arg(va, void *);  
                    nump = (uint8_t *) &numv;
                }
                else
                {
                    spec = 0;
                }
                // FIXME default;
                ++fmt;
                break;
#ifdef FLOATIO
            case 'f':
            case 'F':
            case 'e':
            case 'E':
                // K&R defines 'f' type as 6 - and matches GNU printf
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
            //count = p_itoa(nump, size, buff, sizeof(buff), width, prec, f);
            count = p_ntoa(nump, size, buff, sizeof(buff), 10, width, prec, f);
            _puts_pad(fn,buff, width, count, f.b.left);
            break;
            // FIXME sign vs FILL
        case 'd':
        case 'D':
            count = p_ntoa(nump, size, buff, sizeof(buff), 10, width, prec, f);
            _puts_pad(fn,buff, width, count, f.b.left);
            break;
        case 'b':
        case 'B':
            count = p_ntoa(nump, size, buff, sizeof(buff), 2, width, prec,f);
            _puts_pad(fn,buff, width, count, f.b.left);
            break;
        case 'o':
        case 'O':
            count = p_ntoa(nump, size, buff, sizeof(buff), 8, width, prec,f);
            _puts_pad(fn,buff, width, count, f.b.left);
            break;
        case 'p':
        case 'P':
                // size = sizeof(void *);
        case 'x':
        case 'X':
            count = p_ntoa(nump, size, buff, sizeof(buff), 16, width, prec,f);
            if(spec == 'X' || spec == 'P')
                strupper(buff);
            _puts_pad(fn,buff, width, count, f.b.left);
            break;
#ifdef FLOATIO
        case 'f':
        case 'F':
            count = p_ftoa(dnum, buff, sizeof(buff), width, prec, f);
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


// =============================================
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

#ifdef PRINTF_TEST
#ifdef DEFINE_PRINTF
#error DEFINE_PRINTF must not be defined when testing
#endif
#endif

#ifndef PRINTF_TEST
// =============================================
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

// =============================================
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

#ifdef AVR
/// @brief vsnprintf_P function
/// @param[out] str: string buffer for result
/// @param[in] size: maximum length of converted string
/// @param[in] format: printf forat string
/// @param[in] va: va_list list of arguments
/// @return string size
MEMSPACE 
int vsnprintf_P(char* str, size_t size, __memx const char *format, va_list va)
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

/// @brief snprintf_P function
/// @param[out] str: string buffer for result
/// @param[in] size: maximum length of converted string
/// @param[in] format: printf forat string
/// @param[in] ...: list of arguments
/// @return string size
MEMSPACE 
int snprintf_P(char* str, size_t size, __memx const char *format, ...)
{
    int len;
    va_list va;

    va_start(va, format);
    len = vsnprintf_P(str, size, format, va);
    va_end(va);

    return len;
}

/// @brief sprintf_P function
/// @param[out] str: string buffer for result
/// @param[in] format: printf forat string
/// @param[in] ...: list of arguments
/// @return string size
MEMSPACE 
int sprintf_P(char* str, __memx const char *format, ...)
{
    int len;
    va_list va;

    va_start(va, format);
    // FIXME max string size limit !!!!
    len = vsnprintf_P(str, 1024, format, va);
    va_end(va);

    return len;
}
#endif
// =============================================
/// @brief sprintf function is not recommended because it can overflow
// =============================================

#ifdef DEFINE_PRINTF
// =============================================
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
    printf_t fn;
    va_list va;

    fn.put = _putc_fn;
    fn.sent = 0;

    va_start(va, format);
    _printf_fn(&fn, format, va);
    va_end(va);

    return ((int)fn.sent);
}
#ifdef AVR
/// @brief printf_P function
///  Example user defined printf function using fputc for I/O
///  This method allows I/O to devices and strings without typical C++ overhead
/// @param[in] fmt: printf forat string
/// @param[in] va_list: vararg list or arguments
/// @return size of printed string
/// TODO create a devprintf using an array of function prointers ?
MEMSPACE 
int 
printf_P(__memx const char *format, ...)
{
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
#endif
