/**
 @file n2a.c

 @brief Test 128bit String to number conversion

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


#ifdef PRINTF_TEST

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "mathio.h"


void test10(long long max)
{
    long long val;
    long long s;
    printf("testing mul10str() function: (%lld) values\n", max);
    for(val=0;val<max;++val)
    {
        s = val;
        mul10str((uint8_t *) &s, sizeof(s));
        if(s != (val * 10))
            printf("%lld * 10 = %lld, != %lld\n", val, (val * 10), s);
    }
    printf("Done %lld tests\n\n", val);
}



int main(int argc, char *argv[])
{

    int len;
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
    printf("sizeof( intmax_t ) == %d\n", (int) sizeof( intmax_t ));

    test10(1000000LL);

    while(1) {
        printf("Enter a number or ENTER to quit:");
        fgets(line, 1020,stdin);
        len = strlen(line);
        while(len)
        {
            --len;
            if(line[len] < ' ')
                line[len] = 0;
        }
        if(!*line)
            break;
        sign_ch = 0;
        val2 = strto128(line, NULL, 10);
        if(val2 < (__int128_t) 0)
        {
            sign_ch = '-'; 
            val2 = -val2;
        }
        size = bin2num(numresult, 1022, 1, 10, (uint8_t *) &val2, sizeof(val2), sign_ch);
        //dump(numresult,size);
        printf("numresult[%s]\n",numresult);
    }
}
#endif
