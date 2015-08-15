/**
 @file sup.c 

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
	#include "user_config.h"
#endif
#include "sup.h"


/// @brief Reverse a string
///  Example: abcdef -> fedcba
/// @param[in] str: string 
/// @return string length
MEMSPACE 
void reverse(char *str)
{
        uint8_t temp;
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

/// @brief Upercase a string
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

/// @brief Convert ASCII character to radix based digit , or -1
/// @param[in] c: character
/// @param[in] radix: radix
/// @return radix based digit , or -1
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
MEMSPACE
long aton(uint8_t *str,int base)
{
    unsigned long num;
    char *endptr;

    num = strtol(str, &endptr, base);
    return(num);
}

#ifndef PRINTF_TEST

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
/// @param[in] base: radix
/// @return long value
MEMSPACE 
long atoi(uint8_t *str)
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
double atof(str)
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
		frac = 1.0; 
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

#endif
