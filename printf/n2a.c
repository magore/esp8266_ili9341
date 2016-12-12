#ifdef PRINTF_TEST

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include "mathio.h"
#include <math.h>


/// @brief Test bin2num - see printf 
/// Convert an unsigned number (count bits in size) to ASCII 
///   in a given base
///   Note: There is NO limit on the number of bytes in the binary number!
///   - How it works: Say you have a table of powers of 2 in a given base base
///   - To get the result add each table entry, in the base, that corresponds 
///   to each 1 bit in the binary number.
///   - Now instead of a table we can start with 1 and multiple by 2 in the
///   base we want to get each table entry.
///   - So we convert 1 bit at a time to the base and multiply it by the
///   position in the binary number (this would corresponds to a table 
///   of powers in that base exactly).
/// @param[out] numstr: number string
/// @param[in] nummax: maximum size of numstr in bytes
/// @param[in] nummin: minimum number of digits to display
/// @param[in] bin: Binary input number to convert assumes LSB to MSB order
/// @param[in] size: size of binary number in bytes
/// @param[in] sign_ch: sign of binary number 
/// return converted number string size in bytes
int bin2num(uint8_t *numstr, int nummax, int nummin, int base, uint8_t *bin, int size, int sign_ch)
{
	int i,j,carry;
	uint8_t data;

	int sizebits = size * 8;

	for(i=0;i<=nummin;++i)
		numstr[i] = 0;	// initial string starts out empty

	// Loop for all bits (size in total) in the binary number (bin)
	// Examin each bit MSB to LSB order, FIXME Little/Big-endian
	for(i = sizebits - 1; i>= 0; --i)
	{
		// We extract 1 bit at a time from the binary number (MSB to LSB order) 
		// 	FIXME Little/Big-endian
		data = bin[i>>3];
		// If extracted bit was set carry = 1, else 0
		carry = ( data & (1 << (i & 7)) ) ? 1 : 0;

		// Multiply base number by two and add the previously extracted bit
		// Next do base digit to digit carries as needed
		// Note: nummin is the current string size that can grow as needed
		// Carry test in the loop can optionally extend base strings size 
		for(j=0;(j<nummin || carry) ;++j)
		{
			if(j >= (nummax - 2))
				break;

			data = numstr[j];
			data = (data<<1) | carry;
			// process base carry
			carry = 0;
			if(data >= base)
			{
				data -= base;
				carry = 1;
			}
			numstr[j] = data;
		}
		numstr[j] = 0; 	// zero next digit if carry extends size
		nummin = j;		// update nummin if carry extends size
	}

	// Add ASCII '0' or 'a' offsets to base string
	for(i=0;i<nummin;++i)
	{
		if(numstr[i] >= 0 && numstr[i] < 10)
			numstr[i] += '0';
		else numstr[i] += 'a'-10;
	}

	// Add optional sign character
	if(sign_ch && i <= (nummax - 2))
	{
		numstr[i++] = sign_ch;
		++nummin;
	}
	numstr[i] = 0;		// Terminate string eith EOS
		
	// Reverse string into the correct order 
	//    We only exchange up to half way
	for (i = 0; i < (nummin >> 1); i++)
	{
		data = numstr[nummin - i - 1];
		numstr[nummin - i - 1] = numstr[i];
		numstr[i] = data;
	}

	return(nummin);	// Return string size
}


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



MEMSPACE
__int128_t 
strto128(const char *nptr, char **endptr, int base)
{
	__int128_t num;
    int sign = 0;
    int d;

    while(*nptr == ' ' || *nptr == '\t')
        ++nptr;
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
    if(sign)
        num = -num;
    if(endptr)
        *endptr = (char *) nptr;
    return(num);
}

     
int main(int argc, char *argv[])
{

	int size;
	int sign_ch;
	char line[1024];
	char numresult[1024];
	long long val;
	__int128_t val2;

	printf("sizeof(long) = %d\n", (int) sizeof(long));
	printf("sizeof(long long) = %d\n", (int) sizeof(long long));
	printf("sizeof(val) = %d\n", (int) sizeof(val));
	printf("sizeof(__uint128_t) = %d\n", (int) sizeof(__uint128_t));

	while(1) {
		fgets(line, 1020,stdin);
		if(!*line)
			break;
		//sscanf(line,"%lld", &val);

		sign_ch = 0;
		val2 = strto128(line, NULL, 10);
		if(val2 < (__int128_t) 0)
		{
			sign_ch = '-'; 
			val2 = -val2;
		}

		size = bin2num(numresult, 1022, 0, 10, (uint8_t *) &val2, sizeof(val2), sign_ch);
		//dump(numresult,size);
		printf("numresult[%s]\n",numresult);
	}
}
#endif
