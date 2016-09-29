/**
 @file stdlib.c

 @brief Part of Small printf, and verious conversion code with floating point support

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

#include "std.h"

#ifdef FLOAT

/// @brief Raise number to exponent power (exponent is integer)
/// @param[in] num: number
/// @param[in] exp:  interger exponent
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
#endif // ifdef FLOAT

/// @brief Convert ASCII character to radix based digit , or -1
/// @param[in] c: character
/// @param[in] radix: radix
/// @return radix based digit , or -1
/// @see strtol
MEMSPACE 
int atodigit(int c,int radix)
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

/// @brief Convert ASCII string to number in a given base
/// @param[in] str: string
/// @param[in] base: radix
/// @return long value
/// @see strtol
MEMSPACE
long aton(char *str, int base)
{
    unsigned long num;
    char *endptr;

    num = strtol(str, &endptr, base);
    return(num);
}

// Skip if we have the linux stdlib.h
#ifndef _STDLIB_H

/// @brief Convert ASCII string to number in a given base
/// @param[in] nptr: string
/// @param[in] endptr: pointer to string pointer return value
/// @param[in] base: radix
/// @return long value
MEMSPACE 
long strtol(const char *nptr, char **endptr, int base)
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

	if(endptr)
	{
		*endptr = (char *) nptr;
	}
	if(sign)
		return(-num);
	else
		return(num);
}

/// @brief Convert ASCII string to number in base 10
/// @param[in] str: string
/// @return int value
/// @see strtol
MEMSPACE 
int atoi(const char *str)
{
	unsigned long num;
	char *endptr;
	num = strtol(str, &endptr, 10);
	return((int)num);
}

/// @brief Convert ASCII string to number in base 10
/// @param[in] str: string
/// @return long value
/// @see strtol
MEMSPACE 
long atol(const char *str)
{
	unsigned long num;
	char *endptr;
	num = strtol(str, &endptr, 10);
	return(num);
}



#ifdef FLOAT
/// @brief atof ASCII to float
/// @param[in] str: string
/// @return number
MEMSPACE 
double atof(const char *str)
{
	double num;
	double frac;

	int i,j,power,sign;

	while(*str == ' ' || *str == '\t' || *str == '\n')
		++str;
	sign = 1;
	if(*str == '-')
	{
		++str;
		sign = -1;
	}
	else if(*str == '+')
	{
		++str;
	}
	num=0.0; 
	while(isdigit(*str)) 
	{
		num = (num * 10.0) + (double) (*str - '0');
		str++;
	}
	if(*str == '.') 
	{
		++str;
		frac = 1.0; 
		while(isdigit(*str)) 
		{
			num = (num * 10.0) + (double) (*str - '0');
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
			power = (power * 10) + (double)(*str - '0');
			str++;
		}
		if(num == 0.0)
			return(num);
		if(sign<0)
			power = -power;
		// iexp - number to integer power
		return(num * iexp(10.0, power));
	}
	return(num);
}
#endif // ifdef FLOAT
#endif // ifndef _STDLIB_H

