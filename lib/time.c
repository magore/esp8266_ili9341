/**
 @file lib/time.c

 @brief Common Linux/POSIX time functions

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


#include <user_config.h>

#include "timer_hal.h"
#include "time.h"

/// @brief  System Clock Time
extern volatile ts_t __clock;

/// @brief  System Time Zone
tz_t __tzone;


///@brief accumulated days to the start of a month in a year.
///
/// - without Leap days.
/// - Index: Month 00 .. 11  (12 is a year).
/// @see timegm().
static const uint16_t __days_sum[] =
{
    0, 31, 59, 90, 120, 151, 181, 212, 243, 273, 304, 334, 365
};

///@brief days in each month.
///  - without leap days.
///  - Index: Month 00 .. 11.
/// @see timegm().
static const uint16_t __days[] =
{
    31, 28, 31, 30, 31, 30, 31, 31, 30, 31, 30, 31
};

///@brief Short Name of each Day in a week.
///
/// - Day 0 .. 6 to string.
///
///@see asctime_r()
const char *__WDay[] = { "Sun","Mon","Tue","Wed","Thu","Fri","Sat","BAD"};

/// @brief Get string Short name of day from day number.
///
///@param[in] i: Day 0 .. 6.
///
/// @return string pointer to day.
/// @return "BAD" on error.
MEMSPACE
char *tm_wday_to_ascii(int i)
{
    if(i >= 0 && i <= 6)
        return((char *)__WDay[i]);
    else
        return((char *)__WDay[7]);
}


/// @brief Short Name or each Month in a year.
///
/// - Month 0 .. 11 to string.
///
const char *__Month[]= \
{ \
    "Jan","Feb","Mar","Apr","May","Jun","Jul", \
        "Aug","Sep","Oct","Nov","Dec","BAD"
};

/// @brief Get string Short name of Month from month number.
///
/// @param[in] i: Month 0 .. 11 to string.
///
/// @return string pointer to month.
/// @return "BAD" on error.
/// @see asctime_r()
MEMSPACE
char *tm_mon_to_ascii(int i)
{
    if(i >= 0 && i <= 11)
        return((char *)__Month[i]);
    else
        return((char *)__Month[12]);
}


/// @brief Check if a year is a leap year.
///
/// @param[in] year: valid over 1900..2199.
///
/// @warning:	No limit checking.
///
/// @return  	1 (leap year), 0 (not a leap year(
/// @see time_to_tm() 
/// @see ctime_gm() 
/// @see gmtime_r() 
/// @see localtime_r()
MEMSPACE
static int IS_Leap(int year)
{
/// @return return

    if(year & 3)
        return(0);

    if(year == 1900 || year == 2100)
        return(0);

    return(1);
}


/// @brief  Number of leap days since 1900 to the BEGINNING of the year.
///
/// @param[in] year: valid over 1900..2199.
///
/// @warning No limit checking
///
/// @return days.
///
/// @see time_to_tm() 
/// @see ctime_gm() 
/// @see gmtime_r() 
/// @see localtime_r()
MEMSPACE
static int Leap_Days_Since_1900(int year)
{
    int sum;

    year -= 1900;

    if(year>0)
        --year;

    sum = (year >> 2);


    if(year >= 200 )
        --sum;

    return(sum);
}


/// @brief  Find number of days in a given year.
///
/// @param[in] year: valid over 1900..2199.
///
/// @warning No limit checking
///
/// @return days in a given year.
///
/// @see time_to_tm() 
/// @see ctime_gm() 
/// @see gmtime_r() 
/// @see localtime_r()
MEMSPACE
static int Days_Per_Year(int year)
{
    return(IS_Leap(year) ? 366 : 365);
}


/// @brief  Converts epoch ( seconds from 1 Jan EPOCH_YEAR UTC), offset seconds, to UNIX tm *t.
///  @param[in] epoch:	Seconds elapsed since January 1, EPOCH_YEAR.
///	 - unsigned long,	range limited to: 0 .. 0xFFFD5D00>
///  - The range 0xFFFEAE80 .. 0xFFFFFFFF is reserverd for Dec 31, 1969.
///  - The range 0xFFFD5D00 .. 0xFFFEAE7F is reserverd of offset overflow.
///  @param[in] offset: 	Offset in seconds to localtime.
///  - long int, range limited to +/- 86400.
///  - (Number of seconds that we add to UTC to get local time).
///  @param[out] t: Unix tm struct pointer output.

/// @return 0: *t has result.
/// @return -1: error.
///
/// @see ctime_gm() 
/// @see gmtime_r() 
/// @see localtime_r()

MEMSPACE
int time_to_tm(time_t epoch, int32_t offset, tm_t *t)
{
    int year,month,tmp;
    int flag = 0;
    int32_t days;

    memset(t,0,sizeof(tm_t));

    if(epoch >= 0xFFFD5D00UL)
        return(-1);

    t->tm_gmtoff = offset;
    epoch -= offset;

    if(epoch >= 0xFFFEAE80UL)
    {
        epoch -= 0xFFFEAE80UL;
        flag = 1;
    }

    t->tm_sec = epoch % 60;
    epoch /= 60;

    t->tm_min = epoch % 60;
    epoch /= 60;

    t->tm_hour = epoch % 24;
    epoch /= 24;

    days = epoch;

    if(flag)
    {
        t->tm_year = 69;
        t->tm_mon = 11;
        t->tm_mday = 31;
        t->tm_wday = (EPOCH_DAY - 1) % 7;
    }
    else
    {
        t->tm_wday = (EPOCH_DAY + days) % 7;

        year=EPOCH_YEAR;
        while (days >= (tmp=Days_Per_Year(year)) )
        {
            ++year;
            days -= tmp;
        }

        t->tm_year = year - 1900;
        t->tm_yday = days;

        month = 0;

        while(days > 0 && month < 12)
        {
            tmp=__days[month];
            if( month ==  1 && IS_Leap(year))
                ++tmp;
            if(days < tmp)
                break;
            days -= tmp;
            ++month;
        }

        t->tm_mon = month;
        t->tm_mday = days + 1;
    }
    return(0);
}


/// @brief Convert tm_t structure as GMT time into seconds since 1900.
///
/// - Standards: GNU and BSD.
/// - Limits: 	year(1900..2199).
/// - Assume:  epoch size is uint32_t time_t;
///
/// @see mktime() POSIX function.
/// @see timegm() POSIX function.
/// @return Seconds since EPOCH_YEAR Jan 1st.
/// @return -1 on error.
MEMSPACE
time_t timegm( tm_t *t )
{
    time_t days,seconds;

    int year = t->tm_year + 1900;
    int mon = t->tm_mon;                          // 0..11
    int mday = t->tm_mday;                        // 1..28,29,30,31
    int hour = t->tm_hour;                        // 0..23
    int min = t->tm_min;                          // 0..59
    int sec = t->tm_sec;                          // 0..59

    if (year < EPOCH_YEAR || year > 2106)
        return(-1);

    if(mon > 12 || mday > 31 || hour > 23 || min > 59 || sec > 59)
        return(-1);

    if(mon < 0 || mday < 0 || hour < 0 || min < 0 || sec < 0)
        return(-1);

    --mday;                                       // remove offset

///  Note: To simplify we caculate Leap Day contributions in stages


    days = ( year  - EPOCH_YEAR );

    days *= (time_t) 365L;

    days += (time_t) __days_sum[mon];

    days += (time_t) mday;


    days += (time_t) Leap_Days_Since_1900(year);

    days -= (time_t) 17;

    if(mon > 1 && IS_Leap(year))
        ++days;

    seconds = days;

    seconds *= (time_t) 24L;                      // hours
    seconds += (time_t) hour;
    seconds *= (time_t) 60L;                      // Minutes
    seconds += (time_t) min;
    seconds *= (time_t) 60L;                      // Seconds
    seconds += (time_t) sec;
    return (seconds);
}


/// @todo  implement strftime() and strptime()

/// @brief Convert tm_t *t structure into POSIX asctime() ASCII string *buf.
///
/// @param[in] t: tm_t structure pointer.
/// @param[out] buf: user buffer for POSIX asctime() string result.
/// - Example output: "Thu Dec  8 21:45:05 EST 2011".
///
/// @return buf string pointer.
MEMSPACE
char *asctime_r(tm_t *t, char *buf)
{
    snprintf(buf,32,"%s %s %2d %02d:%02d:%02d %4d",
        __WDay[t->tm_wday],
        __Month[t->tm_mon],
        t->tm_mday,
        t->tm_hour,
        t->tm_min,
        t->tm_sec,
        t->tm_year + 1900);
    return(buf);
}



/// @brief String storage for asctime().
static char __ctime_buf[32];

/// @brief Convert tm_t *t structure into POSIX asctime() ASCII string.
///
/// @param[in] t: struct tm * time input.
///
/// @return __ctime_buf[] string pointer in POSIX asctime() format.
/// - Example output: "Thu Dec  8 21:45:05 EST 2011".
/// @warning result is overwritten on each call.
MEMSPACE
char *asctime(tm_t *t)
{
    return( asctime_r(t,__ctime_buf) );
}


/// @brief Convert local time_t *t epoch time into POSIX asctime() ASCII string *buf.
///
/// @param[in] t: time_t * time input.
/// @param[out] buf: string output.
///  - Example output: "Thu Dec  8 21:45:05 EST 2011"
///
/// @return  buf string pointer.
MEMSPACE
char *ctime_r(time_t *t, char *buf)
{
    return( asctime_r(localtime(t),buf) );
}


/// @brief Convert local time_t *t epoch time into POSIX asctime() string __ctime_buf[]
///
/// @param[in] tp: time_t * time input.
///
/// @return  __ctime_buf[].
///  - Example: "Thu Dec  8 21:45:05 EST 2011".
MEMSPACE
char *ctime(time_t *tp)
{
    return( asctime_r( localtime(tp),__ctime_buf ) );
}


/// @brief GMT version of POSIX ctime().
///
/// @param[in] tp: time_t * time input.
///
/// @return  __ctime_buf[].
///  - Example: "Thu Dec  8 21:45:05 EST 2011".
/// @see ctime()
/// @warning result is overwritten on each call.
MEMSPACE
char *ctime_gm(time_t *tp)
{
    time_t epoch = *tp;
    tm_t tm;
    time_to_tm(epoch, 0L, &tm);
    return( asctime_r( &tm, __ctime_buf ) );
}


/// @brief Convert epoch GMT time_t *tp into POSIX tm_t *result.
///
/// @param[in] tp: time_t * time input.
/// @param[out] result: tm_t *result.
///
/// @return tm_t *result.
MEMSPACE
tm_t *gmtime_r(time_t *tp, tm_t *result)
{
    time_t epoch = *tp;
    time_to_tm(epoch , 0L, result);
    return(result);
}


/// @brief Convert epoch GMT time_t *tp into POSIX static tm_t *t.
///
/// @param[in] tp: time_t * time input.
///
/// @return tm_t t.
/// @warning result is overwritten on each call.
MEMSPACE
tm_t *gmtime(time_t *tp)
{
    static tm_t t;
    gmtime_r(tp, &t);
    return(&t);
}


/// @brief Convert POSIX epoch time_t *tp into POSIX tm_t *result.
///
/// @param[in] t: time_t * epoch time input.
/// @param[out] result: tm_t *result.
///
/// @return result.
MEMSPACE
tm_t *localtime_r(time_t *t, tm_t *result)
{
    tz_t tz;
    long int offset;
    time_t epoch = *t;

    gettimezone(&tz);
    offset = 60L * tz.tz_minuteswest;

    time_to_tm(epoch, offset, result);

    return(result);
}


/// @brief Convert POSIX epoch time_t *tp into POSIX tm_t *result.
///
/// @param[in] tp: time_t * epoch time input.
///
/// @return struct tm result.
/// @warning result is overwritten on each call.
MEMSPACE
tm_t *localtime(time_t *tp)
{
    static struct tm t;
    return( localtime_r(tp, &t) );
}


/// @brief Convert POSIX tm_t *t struct into POSIX epoch time in seconds with UTC offset adjustment.
///
/// @param[in] t: time_t * epoch time input.
///
// @return time_t epoch time.
MEMSPACE
time_t mktime(tm_t *t)
{

    time_t val;
    long int offset;
    tz_t tz;

    gettimezone(&tz);
    offset = 60L * tz.tz_minuteswest;

    val = timegm( t );
    val = val - offset;

    return( val );
}


/// @brief Get current timezone in struct timezone *tz - POSIX function.
///
/// @param[out] tz: timezone result.
///
/// @return  0
MEMSPACE
int gettimezone(tz_t *tz)
{
    tz->tz_minuteswest = __tzone.tz_minuteswest;
    tz->tz_dsttime = __tzone.tz_dsttime;
    return(0);
}


/// @brief Set current timezone with struct timezone *tz - POSIX function.
///
/// @param[in] tz: timezone result.
///
/// @return 0.
MEMSPACE
int settimezone(tz_t *tz)
{
    __tzone.tz_minuteswest = tz->tz_minuteswest;
    __tzone.tz_dsttime = tz->tz_dsttime;
    return(0);
}


/// @brief Get current time struct timeval *tv and struct timezone *tz - POSIX function.
///
/// @param[in] tv: time.
/// @param[in] tz: timezone.
///
/// @return  0
MEMSPACE
int gettimeofday(tv_t *tv, tz_t *tz)
{

    ts_t ts;

    clock_gettime(0, (ts_t *) &ts);

    tv->tv_sec = ts.tv_sec;
    tv->tv_usec = ts.tv_nsec / 1000L;

    gettimezone(tz);
    return(0);
}


/// @brief Return second from epoch - POSIX function.
///
///  @param[in,out] t: pointer to store time in.
///   - Notes:  If t is non-NULL, store the return value there also.
/// @return time_t seconds from epoch.
/// @see clock_gettime().
MEMSPACE
time_t time(time_t *t)
{
    ts_t ts;
    clock_gettime(0, (ts_t *) &ts);
    if(t != NULL)
        *t = ts.tv_sec;
    return(ts.tv_sec);
}


/// @brief Set current time struct timeval *tv and struct timezone *tz - POSIX function.
///
/// @param[in] tv: time.
/// @param[in] tz: timezone.
///
/// @return  0
MEMSPACE
int settimeofday(tv_t *tv, tz_t *tz)
{
    ts_t ts;

    ts.tv_sec = tv->tv_sec;
    ts.tv_nsec = tv->tv_usec * 1000L;

    clock_settime(0, (ts_t *) &ts);

    settimezone(tz);

    return(0);
}


/// @brief Set system clock with seconds and microseconds.
///
/// - Note: Alternate clock functions.
/// @return void.
/// @see clock_settime().
MEMSPACE
void clock_set(uint32_t seconds, uint32_t us)
{
    ts_t ts;
    ts.tv_sec = seconds;
    ts.tv_nsec = us * 1000L;

    clock_settime(0, (ts_t *) &ts);
}


///@brief Set date and time by prompting user.
///
/// - Prompt use for Date Time input with "YYYY MM DD HH:MM:SS>"
/// - Input format is: "YYYY MM DD HH:MM:SS"
///
///@return 0 on success.
///@return -1 on error>
MEMSPACE
int setdate (void)
{
    char buf[40];
	extern int get_line (char *buff, int len);

    printf("Enter date YYYY MM DD HH:MM:SS >");
    get_line(buf,40);

	return(setdate_r(buf));
}

///@brief Set date and time from string in this format "YYYY MM DD HH:MM:SS".
///
///@param[in] buf: Date string in this format "YYYY MM DD HH:MM:SS".
///
///@return 0 on success.
///@return(-1) on error.
MEMSPACE
int setdate_r (char *buf)
{
    tm_t tm;
    ts_t ts;
    time_t seconds;

    tm.tm_year=tm.tm_mon=tm.tm_mday=tm.tm_hour=tm.tm_min=tm.tm_sec=0;


#ifdef NO_SCANF
    while(*buf && *buf < '0' && *buf > '9')
        ++buf;
    tm.tm_year = strtol(buf,&buf,10);
    while(*buf && *buf < '0' && *buf > '9')
        ++buf;
    tm.tm_mon = strtol(buf,&buf,10);
    while(*buf && *buf < '0' && *buf > '9')
        ++buf;
    tm.tm_mday = strtol(buf,&buf,10);
    while(*buf && *buf < '0' && *buf > '9')
        ++buf;
    tm.tm_hour = strtol(buf,&buf,10);
    while(*buf && *buf < '0' && *buf > '9')
        ++buf;
    tm.tm_min = strtol(buf,&buf,10);
    while(*buf && *buf < '0' && *buf > '9')
        ++buf;
    tm.tm_sec = strtol(buf,&buf,10);
    while(*buf && *buf < '0' && *buf > '9')
        ++buf;
#else
    sscanf(buf,"%d %d %d %d:%d:%d",
        &tm.tm_year,
        &tm.tm_mon,
        &tm.tm_mday,
        &tm.tm_hour,
        &tm.tm_min,
        &tm.tm_sec);
#endif

    tm.tm_mon--;

    if(tm.tm_year < 1970 || tm.tm_year > 2038)
    {
        printf("invalid year: %d\n",tm.tm_year);
        return(-1);
    }
    if(tm.tm_year >= 1900)
        tm.tm_year -= 1900;
    if(tm.tm_mon < 0 || tm.tm_mon > 11)
    {
        printf("invalid mon: %d\n",tm.tm_year);
        return(-1);
    }
    if(tm.tm_mday < 1 || tm.tm_mday > 31)
    {
        printf("invalid day: %d\n",tm.tm_mday);
        return(-1);
    }
    if(tm.tm_hour < 0 || tm.tm_hour > 23)
    {
        printf("invalid hour: %d\n",tm.tm_hour);
        return(-1);
    }
    if(tm.tm_min < 0 || tm.tm_min > 59)
    {
        printf("invalid min: %d\n",tm.tm_min);
        return(-1);
    }

    seconds = timegm(&tm);

    ts.tv_sec = seconds;
    ts.tv_nsec = 0L;
    clock_settime(0, (ts_t *) &ts);

#ifdef RTC
    if( !rtc_init(1, (time_t) seconds ) )
    {
        printf("rtc force init failed\n");
        return(-1);
    }
#endif
    return(0);
}
