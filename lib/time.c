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

#include "user_config.h"

#ifdef AVR
#include <stdlib.h>
#include <string.h>
#endif

#include "fatfs.h"

#include "printf/mathio.h"

#include "lib/time.h"
#include "lib/timer.h"

#ifdef RTC
#include "lib/rtc.h"
#endif

/// @brief  System Clock Time
extern volatile ts_t __clock;

/// @brief  System Time Zone
tz_t __tzone;

///@brief DST start and stop in GMT epoch
dst_t dst;

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
    if((year & 3) || year == 1900 || year == 2100)
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

///@brief return day of week for givenn day, month, year
/// @param[in] year: year  such as 2016
/// @param[in] month: month 0 .. 11
/// @param[in] day: day 1 .. 28|29|30|31
/// result is day of week 0 .. 6, 0 = Sunday
MEMSPACE
int finddayofweek(int year, int month, int day)
{
	int value;

	if ( month < 3 )
	{ 
		month += 12;
		year -= 1;
	}
	value = ( year + year/4 - year/100 + year/400 );
	value += ( 6 * (month+1) / 10 + (month * 2));
	value += day;
	value += 1;
	return( value % 7 );
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

/// @brief days in a month
///
/// @param[in] month; month of year
/// @param[in] year; year
///
/// @return days in month including yeapyears
MEMSPACE
int Days_Per_Month(int month, int year)
{
	int days;

	// Normalize month
	while(month >= 12)
	{
		++year;
		month -= 12;
	}
	while(month < 0)
	{
		--year;
		month += 12;
	}
	days = __days[month];
	if( month ==  1 && IS_Leap(year))
		++days;
	return( days );
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

/// @return epoch: time in seconds, *t has result.
/// @return -1: error.
///
/// @see ctime_gm() 
/// @see gmtime_r() 
/// @see localtime_r()

MEMSPACE
time_t time_to_tm(time_t epoch, int32_t offset, tm_t *t)
{
    int year,month,tmp;
    int flag = 0;
    int32_t days;
	time_t save = epoch;

    memset(t,0,sizeof(tm_t));

    if(epoch >= 0xFFFD5D00UL)
        return(-1);

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
    return(save - offset);
}


/// @brief Convert tm_t structure as GMT time into GMT seconds since 1900.
/// All calculactions are in GMT regardless of timezoe settings
///
/// - Standards: GNU and BSD.
/// - Limits: 	year(1900..2199).
/// - Assume:  epoch size is time_t;
///
/// @see mktime() POSIX function.
/// @see timegm() POSIX function.
/// @see normalize() and tm2epoch non POSIX functions
/// @return Seconds since EPOCH_YEAR Jan 1st.
/// @return -1 on error.
MEMSPACE
time_t timegm( tm_t *t )
{
	time_t seconds;
	seconds = normalize(t,0);
    return (seconds);
}

// =============================================
/// @todo  implement strftime() and strptime()
// =============================================


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
	// normaize tm_t before output
	(void) normalize(t,0);

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



/// @brief Convert tm_t *t structure into POSIX asctime() ASCII string.
///
/// @param[in] t: struct tm * time input.
///
/// @return buf[] string pointer in POSIX asctime() format.
/// - Example output: "Thu Dec  8 21:45:05 EST 2011".
/// @warning result is overwritten on each call.
MEMSPACE
char *asctime(tm_t *t)
{
	static char buf[32];
	// acstime_r does tm_t normalization
    return( asctime_r(t,buf) );
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
	// acstime_r does tm_t normalization
    return( asctime_r(localtime(t),buf) );
}


/// @brief Convert local time_t *t epoch time into POSIX asctime() string buf[]
///
/// @param[in] tp: time_t * time input.
///
/// @return  buf[].
///  - Example: "Thu Dec  8 21:45:05 EST 2011".
MEMSPACE
char *ctime(time_t *tp)
{
	static char buf[32];
	// acstime_r does tm_t normalization
    return( asctime_r( localtime(tp), buf) );
}


/// @brief GMT version of POSIX ctime().
///
/// @param[in] tp: time_t * time input.
///
/// @return  buf[].
///  - Example: "Thu Dec  8 21:45:05 EST 2011".
/// @see ctime()
/// @warning result is overwritten on each call.
MEMSPACE
char *ctime_gm(time_t *tp)
{
	static char buf[32];
    tm_t tm;
    return( asctime_r( gmtime_r(tp,&tm), buf) );
}


/// @brief Convert epoch GMT time_t *tp into POSIX tm_t *result.
///
/// @param[in] tp: time_t * time input.
/// @param[out] result: tm_t *result.
///
/// @return tm_t *result.
MEMSPACE
tm_t *gmtime_r(time_t *t, tm_t *result)
{
    time_t epoch = *t;
    (void)time_to_tm(epoch, 0, result);
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
    static tm_t t, *p;
	p = &t;
    gmtime_r(tp, p);
    return(p);
}


// FIXME localtime
/* 
 The localtime function converts the calendar time timep to broken-down time representation, expressed relative to the users specified  timezone.   
 The function  acts  as if it called tzset(3) and sets the external variables tzname with information about the current timezone, timezone with the difference
       between Coordinated Universal Time (UTC) and local standard time in seconds, and daylight to a nonzero value if daylight savings time rules apply  during
       some  part  of  the year.  The return value points to a statically allocated struct which might be overwritten by subsequent calls to any of the date and
       time functions.  The localtime_r() function does the same, but stores the data in a user-supplied struct.  It need not set  tzname,  timezone,  and  day‐
       light.
*/


/// @brief Convert POSIX epoch time_t *tp into POSIX tm_t *result 
///   expressed as local time using timezone and DST corrections.
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

	if(is_dst(epoch - offset))
		offset -= 3600L;
    (void) time_to_tm(epoch, offset, result);

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

/// @brief convert tm_t structure to time_t local time epoch 
///
/// @param[in] t: tm_t time input.
///
/// @return local epoch or -1 on failure out of range
MEMSPACE
time_t mktime(tm_t *t)
{
	return( normalize(t, 1) );
}


/// @brief Converts tm_t structure as GMT time into GMT epoch since 1900.
/// - used internally by normalize() function after normalization
/// - DO NOT use in user code , use normaize or any function that uses it
///
/// - Standards: none
/// - Limits: 	year(1900..2199).
/// - Assume:  epoch size is time_t;
///
/// @return Seconds since GMT EPOCH_YEAR Jan 1st.
/// @return -1 on error.
MEMSPACE
static
time_t tm2epoch( tm_t *t )
{
    time_t days,seconds;

    int year = t->tm_year + 1900;
    int mon = t->tm_mon;                          // 0..11
    int mday = t->tm_mday - 1;                        // 1..28,29,30,31
    int hour = t->tm_hour;                        // 0..23
    int min = t->tm_min;                          // 0..59
    int sec = t->tm_sec;                          // 0..59

#ifdef TIME_DEBUG
	printf("tm2epoch %4d,%2d,%2d, %02d:%02d:%02d\n",
		(int)t->tm_year+1900, (int)t->tm_mon, (int)t->tm_mday,
		(int)t->tm_hour, (int)t->tm_min, (int)t->tm_sec);
#endif

    if (year < EPOCH_YEAR || year > 2106)
	{
#ifdef TIME_DEBUG
		printf("tm2epoch year out of range: %4d\n", year);
#endif
        return(-1);
	}

    if(mon >= 12 || mon < 0)
	{
#ifdef TIME_DEBUG
		printf("tm2epoch mon out of range: %4d\n", mon);
#endif
        return(-1);
	}

    if(mday >= Days_Per_Month(mon,year)  || mday < 0)
	{
#ifdef TIME_DEBUG
		printf("tm2epoch mday out of range: %4d\n", mday);
#endif
        return(-1);
	}

    if(hour >= 24 || hour < 0)
	{
#ifdef TIME_DEBUG
		printf("tm2epoch hour out of range: %4d\n", hour);
#endif
        return(-1);
	}

    if(min >= 60|| min < 0)
	{
#ifdef TIME_DEBUG
		printf("tm2epoch min out of range: %4d\n", min);
#endif
        return(-1);
	}

    if(sec >= 60 || sec < 0)
	{
#ifdef TIME_DEBUG
		printf("tm2epoch sec out of range: %4d\n", sec);
#endif
        return(-1);
	}

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


/// @brief Normalize POSIX tm_t *t struct and convert to epoch time
/// Note: does not deal with DST - by design
///
/// @param[in] t: tm_t time input.
/// @param[in] normalize_to_timezone: nonzero = adjust to local timezone and DST
///
/// @return epoch since 1900 on sucess, -1 on failure out of range
MEMSPACE
time_t normalize(tm_t *t, int normalize_to_timezone)
{
	time_t epoch;
	int32_t offset;
	int isdst;

// 	struct tm
// 	{
// 		int tm_sec;    /*<  Seconds.     [0-60] (1 leap second) */
// 		int tm_min;    /*<  Minutes.     [0-59] */
// 		int tm_hour;   /*<  Hours.       [0-23] */
// 		int tm_mday;   /*<  Day.         [1-31] */
// 		int tm_mon;    /*<  Month.       [0-11] */
// 		int tm_year;   /*<  Year - 1900. */
// 		int tm_wday;   /*<  Day of week. [0-6] */
// 		int tm_yday;   /*<  Days in year.[0-365] */
// 		int tm_isdst;  /*<  DST.         [-1/0/1] */
// 	};

	// Normalize t->tm_seconds
	while(t->tm_sec >= 60)
	{
		++t->tm_min;
		t->tm_sec -= 60;
	}
	while(t->tm_sec < 0)
	{
		--t->tm_min;
		t->tm_sec += 60;
	}

	// Normalize t->tm_miniutes
	while(t->tm_min >= 60)
	{
		++t->tm_hour;
		t->tm_min -= 60;
	}
	while(t->tm_min < 0)
	{
		--t->tm_hour;
		t->tm_min += 60;
	}

	// Normalize t->tm_hours
	while(t->tm_hour >= 24)
	{
		++t->tm_mday;
		t->tm_hour -= 24;
	}
	while(t->tm_hour < 0)
	{
		--t->tm_mday;
		t->tm_hour += 24;
	}

	// Normalize t->tm_months
	while(t->tm_mon >= 12)
	{
		++t->tm_year;
		t->tm_mon -= 12;
	}
	while(t->tm_mon < 0)
	{
		--t->tm_year;
		t->tm_mon += 12;
	}

	// Normalize t->tm_mday
	// t->tm_mday is 1 based
	while(t->tm_mday > Days_Per_Month(t->tm_mon,t->tm_year) )
	{
		// subtract days in current t->tm_month
		t->tm_mday -= Days_Per_Month(t->tm_mon,t->tm_year);
		// Keep month normalized
		if(++t->tm_mon >= 12)
		{
			t->tm_mon -= 12;
			++t->tm_year;
		}
	}

	// t->tm_mday is 1 based
	while(t->tm_mday < 1)
	{
		// Keep month normalized
		if(--t->tm_mon < 0)
		{
			t->tm_mon += 12;
			--t->tm_year;
		}
		// add days in previous mount
		t->tm_mday += Days_Per_Month(t->tm_mon,t->tm_year);
	}


	// We can now set the remain values by converting to EPOCH and back again
	// convert to EPOCH based seconds
	epoch = tm2epoch( t );
#ifdef TIME_DEBUG
	printf("%10lu - epoch-1\n",epoch);
#endif
	// Normaize to local timezone with DST
	offset = 0L;
	isdst = 0;
	if(normalize_to_timezone)
	{
		tz_t tz;

		gettimezone(&tz);
		offset = tz.tz_minuteswest * 60L;
		if(is_dst(epoch))
		{
			offset -= 3600L;
			isdst = 1;
		}
	}
#ifdef TIME_DEBUG
	printf("%10lu - epoch-2\n",epoch);
#endif

	// convert back to TM structure 
	epoch = time_to_tm(epoch, offset, t);
	if(isdst)
		t->tm_isdst = 1;

#ifdef TIME_DEBUG
	printf("%10lu - epoch-3\n",epoch);
#endif
	return( epoch );
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
/// We assume a GMT hardware clock
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
    static ts_t ts;
    clock_gettime(0, (ts_t *) &ts);
    if(t != NULL)
        *t = ts.tv_sec;
    return(ts.tv_sec);
}


/// @brief Set current time struct timeval *tv and struct timezone *tz - POSIX function.
/// We assume a GMT hardware clock
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

//FIXME we may want to add hooks to the OS time functions if they exist
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
	extern MEMSPACE char *fgets ( char *str , int size , FILE *stream );

    printf("Enter date YYYY MM DD HH:MM:SS >");
    fgets(buf,39,stdin);

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


#ifdef SMALL_SSCANF
    sscanf(buf,"%d %d %d %d:%d:%d",
        &tm.tm_year,
        &tm.tm_mon,
        &tm.tm_mday,
        &tm.tm_hour,
        &tm.tm_min,
        &tm.tm_sec);
#else
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

/// @brief Calculate GMT seconds of DST transition given LOCAL time start / end time and DST flag
///
/// @param[in] dst:    0 .. 1    DST needs to be applied to the arguments for DST caluclulations
/// @param[in] epoch:  0 | epoch if non-zero - UTC epoch time used to obtain year of DST calculations 
/// @param[in] year:   0 | year  if non-zero - UTC year of DST calcululation, if year and epoch are used - ignore epoch
/// @param[in] month:  0 .. 11,  local time month DST transition
/// @param[in] weekno: 1 .. 4    localtime dayno count in this month
/// @param[in] dayno:  0 .. 6,   localtime day of DST transition, 0 = Sunday
/// @param[in] hour:   0 .. 23   local time hour of DST transition
/// result is in epoch GMT seconds
/// Example: Eastern Time and Daylight Savings time
///     time_t epoch,start,end;
///     tv_t tv;
///     tv_t tz;
/// 	gettimeofday(&tv, &tz);
///     epoch = tv.tv_sec;
///     //    DST start for this year,   2nd Sunday of Mar at 2:00am EST
/// 	start = find_dst(0, epoch, 0,  3, 2, 0, 2);
///     //    DST start on 2016,         2nd Sunday of Mar at 2:00am EST
/// 	start = find_dst(0, 0, 2016,  3, 2, 0, 2);
///     //    DST ends on for this year, 1st Sunday of Nov at 2:00am DST
/// 	end   = find_dst(1, epoch, 0, 11, 1, 0, 2);
///     //    DST ends on 2016,          1st Sunday of Nov at 2:00am DST
/// 	end   = find_dst(1, 0, 2016, 11, 1, 0, 2);
MEMSPACE
time_t find_dst(int dst, time_t epoch, int year, int month, int weekno, int dayno, int hour)
{
	tm_t t;
	tz_t tz;
	tv_t tv;
	time_t dst_epoch;
	int32_t offset;
	int days;

	// Get timezone and clock time
	gettimeofday(&tv, &tz);

	// Local time offset in seconds
	// Get local timezone offset in seconds without DST offset
	offset = tz.tz_minuteswest * 60UL;
	// Add DST offset if DST end time includes DST offset
	if(dst)
		offset -= (60UL * 60UL);

	if(year)
	{
		// Year is specified - use it
		t.tm_year = year - 1900;   // 0 = 1900
	}
	else
	{
		// Otherwise, Calculate year from epoch or GMT time
		if(!epoch)
			epoch = tv.tv_sec;
		(void) time_to_tm(epoch, offset, &t);	
	}

	// Local time of DST
	// We compute seconds for start of month in which DST happens
	// Result from timegm() is in GMT
    t.tm_mon = month - 1;   // 0..11
    t.tm_mday = 1;          // 1..28,29,30,31
    t.tm_hour = hour;		// 0 .. 23
    t.tm_min =  0; 			// 0..59
    t.tm_sec =  0;			// 0..59


	// Adjust tm_t to localtime - we can not use normalize as it calls us

	// local seconds as timegm does not add offset
	dst_epoch = timegm(&t);
	
	// Add offset for localtime
	dst_epoch += offset;

	// Convert to back to tm_t structure with localtime offsets applied
	(void) time_to_tm(dst_epoch, 0L, &t);	

	dayno = dayno;
	weekno= weekno;
	days= 0;
	while( 1 )
	{
		if( ((t.tm_wday + days) % 7) == dayno)
		{
			if( (--weekno) == 0)
				break;
		}
		++days;
		dst_epoch += 86400L;
	}

	// Return GMT
	return(dst_epoch);
}

/// @brief Set DST start and end time for the given epoch year
/// @param[in] 0 - or epoch seconds in GMT used to determin the year to aply DST in
///            If 0 we get local GMT epoch time in seconds
MEMSPACE
void set_dst(time_t epoch)
{
	if(epoch == 0)
	{
		tv_t tv;
		tz_t tz;
		// Get timezone and clock time
		gettimeofday(&tv, &tz);
		epoch = tv.tv_sec;
	}

	// Cache caclulations so we do not do them more often then once a day
	if(dst.epoch >= epoch)
	{
		if((dst.epoch - epoch) < 86400L)
			return;
	}
	else
	{
		if((epoch - dst.epoch) < 86400L)
			return;
	}

	dst.epoch = epoch;
	// FIXME think of ways to cache this computation
    dst.start = find_dst(0, epoch, 0,  3, 2, 0, 2);
    dst.end   = find_dst(1, epoch, 0, 11, 1, 0, 2);
}

/// @brief Test GMT epoch time to see if DST applies in a local timezone
/// @param[in] epoch seconds in GMT
/// @See normalize()
/// return 1 if yes, 0 if not
MEMSPACE
int is_dst(time_t epoch)
{
    set_dst(epoch);

    if( epoch >= dst.start && epoch <= dst.end)
		return(1);
	return(0);
}

/// @brief print start/stop for DST as localtime for this year
MEMSPACE
void print_dst()
{
    printf("DST START localtime: %s\n", ctime(&dst.start));
    printf("DST END   localtime: %s\n", ctime(&dst.end));
}

/// @brief print start/stop for DST as GMT for this year
MEMSPACE
void print_dst_gmt()
{
    printf("DST START       GMT: %s\n", ctime_gm(&dst.start));
    printf("DST END         GMT: %s\n", ctime_gm(&dst.end));

}
