/**
 @file lib/time.h

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


#ifndef TIME_H
#define TIME_H

#define EPOCH_YEAR    1970    /*< Thursday Jan 1 1970 */
#define EPOCH_DAY        4    /*< Sunday = 0 ... Saturday = 6 */
#define EPOCH            0    /*< Zero seconds */
#define EPOCH_2000 946684800  /*< Sat, 01 Jan 2000 00:00:00 GMT */

///@brief type of EPOCH result.
typedef uint32_t time_t;

///@brief type of clockid_t.
typedef uint16_t clockid_t;

///@brief POSIX struct tm.
struct tm 
{
    int tm_sec;    /*<  Seconds.     [0-60] (1 leap second) */
    int tm_min;    /*<  Minutes.     [0-59] */ 
    int tm_hour;   /*<  Hours.       [0-23] */
    int tm_mday;   /*<  Day.         [1-31] */
    int tm_mon;    /*<  Month.       [0-11] */
    int tm_year;   /*<  Year - 1900. */
    int tm_wday;   /*<  Day of week. [0-6] */
    int tm_yday;   /*<  Days in year.[0-365] */
    int tm_isdst;  /*<  DST.         [-1/0/1] */
    int32_t tm_gmtoff; /*<  GMT offset in seconds */
};  

/**
 tm_isdst value notes
1 DST in effect - add an hour
0 DST not in effect
-1 DST Unknown status
*/


///@brief POSIX struct tm typedef.
typedef struct tm tm_t;


///@brief POSIX timeval.
struct timeval
{
    time_t       tv_sec;   /*< seconds */
    uint32_t     tv_usec;  /*< microseconds */
};

///@brief POSIX timeval typedef.
typedef struct timeval tv_t;

///@brief POSIX timezone.
struct timezone
{
    int tz_minuteswest;  /*< minutes west of Greenwich */
    int tz_dsttime;      /*< type of DST correction */
};

///@brief POSIX timezone typedef.
typedef struct timezone tz_t;

///@brief POSIX timespec.
struct timespec
{
    time_t   tv_sec;   /*< seconds */
    long     tv_nsec;  /*< nanoseconds */
};

///@brief POSIX timespec typedef.
typedef struct timespec ts_t;

/// @brief  System Clock Time
extern volatile ts_t __clock;

/// @brief  System Time Zone
extern tz_t __tzone;


///@brief  DST structure
typedef struct {
    time_t start;	///@brief Start of local DST in GMT
    time_t end;		///@brief End of local DST in GMT
    time_t epoch;	///@brief epoch used to determine the year of DST start and end caluclulation - for caching
} dst_t;



/* lib/time.c */
MEMSPACE char *tm_wday_to_ascii ( int i );
MEMSPACE char *tm_mon_to_ascii ( int i );
MEMSPACE int finddayofweek ( int year , int month , int day );
MEMSPACE int Days_Per_Month ( int month , int year );
MEMSPACE time_t time_to_tm ( time_t epoch , int32_t offset , tm_t *t );
MEMSPACE time_t timegm ( tm_t *t );
MEMSPACE char *asctime_r ( tm_t *t , char *buf );
MEMSPACE char *asctime ( tm_t *t );
MEMSPACE char *ctime_r ( time_t *t , char *buf );
MEMSPACE char *ctime ( time_t *tp );
MEMSPACE char *ctime_gm ( time_t *tp );
MEMSPACE tm_t *gmtime_r ( time_t *t , tm_t *result );
MEMSPACE tm_t *gmtime ( time_t *tp );
MEMSPACE tm_t *localtime_r ( time_t *t , tm_t *result );
MEMSPACE tm_t *localtime ( time_t *tp );
MEMSPACE time_t mktime ( tm_t *t );
MEMSPACE time_t normalize ( tm_t *t , int normalize_to_timezone );
MEMSPACE int gettimezone ( tz_t *tz );
MEMSPACE int settimezone ( tz_t *tz );
MEMSPACE int gettimeofday ( tv_t *tv , tz_t *tz );
MEMSPACE time_t time ( time_t *t );
MEMSPACE int settimeofday ( tv_t *tv , tz_t *tz );
MEMSPACE void clock_set ( uint32_t seconds , uint32_t us );
MEMSPACE int setdate ( void );
MEMSPACE int setdate_r ( char *buf );
MEMSPACE time_t find_dst ( int dst , time_t epoch , int year , int month , int weekno , int dayno , int hour );
MEMSPACE void set_dst ( time_t epoch );
MEMSPACE int is_dst ( time_t epoch );
MEMSPACE void print_dst ( void );
MEMSPACE void print_dst_gmt ( void );


#endif
