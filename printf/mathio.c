
/**
 @file mathio.c 

 @brief String to number conversions with floating point support

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

#ifdef USER_CODFIG
#include "user_config.h"
#endif

#ifdef PRINTF_TEST
#include <stdio.h>
#include <stdlib.h>
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "mathio.h"

#include <math.h>
#undef atof

// =============================================
// MATH/IO
// =============================================

// =============================================
/// @brief Convert ASCII character to radix based digit , or -1
/// @param[in] c: character
/// @param[in] radix: radix
/// @return radix based digit , or -1
/// @see strtol
MEMSPACE 
int
atodigit(int c,int radix)
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

// =============================================
///@brief Convert ASCII hex string to long integer.
///
///@param[in] p: String to convert.
///
///@return long integer result.
///@see strtol() for return overflow and underflow results.
MEMSPACE
long atoh(const char *p)
{
    return strtol(p, (char **) NULL, 16);
}


// =============================================
/// @brief Convert ASCII string to number in a given base
/// @param[in] str: string
/// @param[in] base: radix
/// @return long value
/// @see strtol
MEMSPACE
long
aton(char *str, int base)
{
    unsigned long num;
    char *endptr;

    num = strtol(str, &endptr, base);
    return(num);
}

// =============================================
/// @brief Fast multiply number of any size by 10 
/// @param[in] str: string
/// @param[in] size: string size
/// @return non-zero on overflow , 0 if ok
MEMSPACE
int mul10str(uint8_t *str, int size)
{

    uint16_t d;
    uint16_t carry = 0;
    while(size--)
    {
        d = *str;
        d <<= 1;
        d <<= 1;
        d += *str;
        d <<= 1;
        d += carry;
        *str = d & 0xff;
        carry = d >> 8;
        ++str;
    }
    return(carry);
}

// =============================================
/// @brief Convert ASCII string to number in a given base
/// @param[in] nptr: string
/// @param[in] endptr: pointer to string pointer return value
/// @param[in] base: radix
/// @return long value
MEMSPACE 
long
strtol(const char *nptr, char **endptr, int base)
{
    unsigned long num;
    int sign;
    int d;

    while(*nptr == ' ' || *nptr == '\t')
        ++nptr;
    sign = 0;
    if(*nptr == '-' ) 
    {
        sign = 1;
        ++nptr;
    } 
    else if(*nptr == '+' )
        {
            ++nptr;
        }
    // skip leading zeros
    while(*nptr == '0')
        ++nptr;
    num = 0;
    while(*nptr)
    {
        d = atodigit(*nptr,base);
        if(d < 0)
            break;
        num = num*base;
        num += d;
        ++nptr;
    }

    if(sign)
        num = -num;
    if(endptr)
        *endptr = (char *) nptr;
    return(num);
}

// =============================================
/// @brief Convert ASCII string to number in a given base
/// @param[in] nptr: string
/// @param[in] endptr: pointer to string pointer return value
/// @param[in] base: radix
/// @return long long
MEMSPACE 
long long
strtoll(const char *nptr, char **endptr, int base)
{
    unsigned long long num;
    int sign;
    int d;

    while(*nptr == ' ' || *nptr == '\t')
        ++nptr;
    sign = 0;
    if(*nptr == '-' )
    {
        sign = 1;
        ++nptr;
    }
    else if(*nptr == '+' )
        {
            ++nptr;
        }
    // skip leading zeros
    while(*nptr == '0')
        ++nptr;

    num = 0;
    while(*nptr)
    {
        d = atodigit(*nptr,base);
        if(d < 0)
            break;
        num = num*base;
        num += d;
        ++nptr;
    }
    if(sign)
        num = -num;
    if(endptr)
        *endptr = (char *) nptr;
    return(num);
}


#ifdef __SIZEOF_INT128__
// =============================================
/// @brief Convert ASCII string to number in a given base
/// @param[in] nptr: string
/// @param[in] endptr: pointer to string pointer return value
/// @param[in] base: radix
/// @return __uint128_t
MEMSPACE
__uint128_t
strto128(const char *nptr, char **endptr, int base)
{
    __uint128_t num;
    int sign;
    int d;

    while(*nptr == ' ' || *nptr == '\t')
        ++nptr;
    sign = 0;
    if(*nptr == '-' )
    {
        sign = 1;
        ++nptr;
    }
    else if(*nptr == '+' )
        {
            ++nptr;
        }

    // skip leading zeros
    while(*nptr == '0')
        ++nptr;

    num = 0;
    while(*nptr)
    {
        d = atodigit(*nptr,base);
        if(d < 0)
            break;
        num = num*base;
        num += d;
        ++nptr;
    }
    if(sign)
        num = -num;
    if(endptr)
        *endptr = (char *) nptr;
    return(num);
}
  
#endif 

// =============================================
/// @brief Convert ASCII string to number in base 10
/// @param[in] str: string
/// @return int value
/// @see strtol
MEMSPACE 
int
atoi(const char *str)
{
    unsigned long num;
    num = strtol(str, NULL, 10);
    return((int)num);
}

// =============================================
/// @brief Convert ASCII string to number in base 10
/// @param[in] str: string
/// @return long value
/// @see strtol
MEMSPACE 
long
atol(const char *str)
{
    unsigned long num;
    num = strtol(str, NULL, 10);
    return(num);
}

// =============================================
// Floating point I/O helper functions
// =============================================
#ifdef FLOATIO
/// @brief Raise number to integer exponent power 
/// The process is much like a bitwise multiply - and reduces operatiosn required
/// @param[in] num: number
/// @param[in] exp:  interger exponent
/// @return num ** exp
MEMSPACE
double
iexp(double num, int exp)
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

// =============================================
/// @brief Scale a number to range 1.0 .. 9.99999... and return exponent
/// @param[in] num: number
/// @param[out] *exp: interger power of 10 for scale factor
/// @return scaled number
MEMSPACE 
double
scale10(double num, int *exp)
{
    int exp10,exp2;
    int sign;

    double scale;

    if(!num)
    {
        *exp = 0;
        return(0.0);
    }

    sign = 0;
    if(num < 0)
    {
        num = -num;
        sign = 1;
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
    if(sign)
        return(-num);
    return(num);
}

// =============================================
/// @brief Convert ASCII string to a double 
/// @param[in] nptr: string
/// @param[in] endptr: pointer to string pointer return value
/// @return double
MEMSPACE
double
strtod(const char *nptr, char **endptr)
{
    double num;
    double frac;
    double tmp;

    int digit, power,sign;

    while(*nptr == ' ' || *nptr == '\t' || *nptr == '\n')
        ++nptr;
    sign = 1;
    if(*nptr == '-')
    {
        ++nptr;
        sign = -1;
    }
    else if(*nptr == '+')
    {
        ++nptr;
    }
    // skip leading zeros
    while(*nptr == '0')
        ++nptr;
    num = 0.0; 
    while(*nptr && isdigit(*nptr)) 
    {
        num *= 10.0;    // make room for new digit
        digit = (*nptr - '0');
        num += (double) digit;
        nptr++;
    }

    if(*nptr == '.') 
    {
        ++nptr;
        frac = 0.1;
        while(*nptr && isdigit(*nptr)) 
        {
            digit = (*nptr - '0');
            tmp = (double) digit;
            num += tmp * frac;
            frac *= 0.1;
            nptr++;
        }
    }
    if(sign == -1)
        num = -num;

    if(*nptr == 'E' || *nptr == 'e') 
    {
        nptr++;
        sign = (*nptr == '-') ? -1 : 1;
        if(sign == -1 || *nptr == '+') 
            nptr++;
        power=0;
        while(isdigit(*nptr)) 
        {
            power *= 10.0;
            digit = (*nptr - '0');
            power += (double) digit; 
            nptr++;
        }
        if(num)
        {
            if(sign<0)
                power = -power;
            // iexp - number to integer power
            num *= iexp(10.0, power);
        }
    }
    if(endptr)
        *endptr = (char *) nptr;
    return(num);
}

// =============================================
/// @brief atof ASCII to float
/// @param[in] str: string
/// @return number
MEMSPACE 
double
atof(const char *str)
{
    double num;
    num = strtod(str, NULL);
    return(num);
}
  
#endif // ifdef FLOATIO

