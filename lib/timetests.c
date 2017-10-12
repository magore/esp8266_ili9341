
/**
 @file lib/timetests.c

 @brief Common Linux/POSIX time testing functions

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


#ifdef ESP8266
#include "user_config.h"
#include "fatfs.h"
#include "printf/mathio.h"
#include "lib/time.h"
#include "lib/timer.h"
#ifdef RTC_SUPPORT
#include "lib/rtc.h"
#endif

#else
#include <stdlib.h>
#include <string.h>
#include <time.h>
#endif

static unsigned int __seed;

MEMSPACE
void mysrand(int seed)
{
    __seed = seed;
}

MEMSPACE
int myrand(int start, int end)
{
    long k,s;


    int val;
    double fval,begin,span;

    if(end > start)
    {
        begin = (double) start; 
        span = (double) (end - start);
    }
    else
    {
        span = (double) (start - end);
        begin = (double) end;
    }

    s = (long)__seed;

    if (s == 0)
        s = 0x31415926L;

    k = s / 127773;
    s = 16807 * (s - k * 127773) - 2836 * k;
    if (s < 0)
        s += 2147483647UL;

    __seed = (unsigned int)s;

    // force s into 0 .. 0x7fffffffUL positive 
    s &= 0x7fffffffUL;

    fval = (double) s / (double) 0x7fffffffL;

    fval *= (double) span;
    fval += (double) begin;
//printf("fval: %f, span:%f, begin:%f\n", (double)fval, (double)span, (double)begin);

    val = fval;

    return(val);
}


MEMSPACE
int timetests(char *str, int check)
{
    FILE *fp;
    time_t epoch;
    tm_t tm;
    tv_t tv;
    tz_t tz;
    int i;
    int val;
    char buf[64];

    str = skipspaces(str);
    if(!*str)
        fp = stdout;
    else if(check)
        fp = fopen(str,"rb");
    else
        fp = fopen(str,"wb");

    if(fp == NULL)
        perror("timetest file open failed");
    gettimeofday(&tv, &tz);

    mysrand(tv.tv_sec);

//    struct tm {
//     int tm_sec;    /* Seconds (0-60) */
//     int tm_min;    /* Minutes (0-59) */
//     int tm_hour;   /* Hours (0-23) */
//     int tm_mday;   /* Day of the month (1-31) */
//     int tm_mon;    /* Month (0-11) */
//     int tm_year;   /* Year - 1900 */
//     int tm_wday;   /* Day of the week (0-6, Sunday = 0) */
//     int tm_yday;   /* Day in the year (0-365, 1 Jan = 0) */
//     int tm_isdst;  /* Daylight saving time */
//    };

    for(i=0;i<20;++i)
    {
#ifdef ESP8266
        optimistic_yield(1000);
#endif

        if(!check)
        {
            tm.tm_year = myrand(EPOCH_YEAR+2,2099) - 1900;
            tm.tm_mon = myrand(-18,18);
            tm.tm_mday = myrand(-45,45);
            tm.tm_hour = myrand(-36,36);
            tm.tm_min = myrand(-90,90);
            tm.tm_sec = myrand(-90,90);

            fprintf(fp,"test: %d\n", i);
            fprintf(fp,"%4d,%2d,%2d, %02d:%02d:%2d\n", 
                (int)tm.tm_year+1900, (int)tm.tm_mon, (int)tm.tm_mday, 
                (int)tm.tm_hour, (int)tm.tm_min, (int)tm.tm_sec);

            // express val as local time
            epoch = mktime(&tm);

            fprintf(fp,"%10lu, %s\n", (long) epoch, asctime_r(&tm, buf));
            fprintf(fp,"%4d,%2d,%2d, %02d:%02d:%2d\n", 
                (int)tm.tm_year+1900, (int)tm.tm_mon, (int)tm.tm_mday, 
                (int)tm.tm_hour, (int)tm.tm_min, (int)tm.tm_sec);

            localtime_r(&epoch, &tm);

            fprintf(fp,"%10lu, %s\n", (long) epoch, asctime_r(&tm, buf));
            fprintf(fp,"%4d,%2d,%2d, %02d:%02d:%2d\n", 
                (int)tm.tm_year+1900, (int)tm.tm_mon, (int)tm.tm_mday, 
                (int)tm.tm_hour, (int)tm.tm_min, (int)tm.tm_sec);
            fprintf(fp,"\n");

        }
    }
    printf("-1 = %d\n", -1);

    if (fp != stdout )
        if( fclose(fp) )
            perror("timetest fclose failed");
    return(1);
}
