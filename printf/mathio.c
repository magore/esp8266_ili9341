#ifdef USER_CODFIG
#include "user_config.h"
#endif

#include <stdint.h>
#include <stdarg.h>
#include <string.h>

#include "mathio.h"

#include <math.h>
#undef atof

// ==================================================
// MATH/IO
// ==================================================

// ==================================================
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

// ==================================================
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


// ==================================================
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

// ==================================================
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

// ==================================================
/// @brief Convert ASCII string to number in base 10
/// @param[in] str: string
/// @return int value
/// @see strtol
MEMSPACE 
int
atoi(const char *str)
{
	unsigned long num;
	char *endptr;
	num = strtol(str, &endptr, 10);
	return((int)num);
}

// ==================================================
/// @brief Convert ASCII string to number in base 10
/// @param[in] str: string
/// @return long value
/// @see strtol
MEMSPACE 
long
atol(const char *str)
{
	unsigned long num;
	char *endptr;
	num = strtol(str, &endptr, 10);
	return(num);
}

// ==================================================
// Floating point I/O helper functions
// ==================================================
#ifdef FLOATIO
/// @brief Raise number to exponent power (exponent is integer)
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


// ==================================================
/// @brief Scale a number to 1.0 .. 9.99999...
/// @param[in] num: number
/// @param[out] *exp: interger power of 10 for scale factor
/// @return scaled number
MEMSPACE 
double
scale10(double num, int *exp)
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

// ==================================================
/// @brief atof ASCII to float
/// @param[in] str: string
/// @return number
MEMSPACE 
double
atof(const char *str)
{
	double num;
	double frac;

	int power,sign;

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
#endif // ifdef FLOATIO

