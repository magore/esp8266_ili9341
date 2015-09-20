/**
 @file lib/timer.c

 @brief timer functions

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
#include <time.h>

///@brief system interrupt rate in HZ
#define SYSTEM_HZ 1000L
#include "timer.h"

/// @brief  array or user timers
extern TIMERS timer_irq[];

/// @brief  Install a user timer task.
///
/// - timer argument is unsused, reserved for timer source.
///
/// @param[in] handler):  function pointer to user task.
/// @param[in] timer:  reserved for systems with additional low level hardware timers.
///
/// @return timer on success.
/// @return -1 on error.
MEMSPACE
int set_timers(void (*handler)(void), int timer)
{
    int i;
    int ret = -1;

    if(!handler)
        return -1;

    for(i=0;i<MAX_TIMER_CNT;++i)
    {

        // already assigned
        if(timer_irq[i].user_timer_handler == handler)
            ret = i;

        if(!timer_irq[i].user_timer_handler)
        {
            timer_irq[i].timer = 0;   // Set to disable
            timer_irq[i].user_timer_handler = handler;
            timer_irq[i].timer = 1;      // Set if enabled, 0 if not
            ret = i;
            break;
        }
    }
    if(ret == -1)
        printf("set_timers: No more timers!\n");

    return(ret);
}


/// @brief  Delete "Kill" one user timer task.
///
/// - "kill" is a common Linux term for ending a process.
///
/// @param[in] timer: user timer task index.
///
/// @return timer on success.
/// @return -1 on error.
MEMSPACE
int kill_timer( int timer )
{
    int ret = -1;
    if(timer >= 0 && timer <= MAX_TIMER_CNT)
    {
        timer_irq[timer].timer = 0;               // Disable
        timer_irq[timer].user_timer_handler = 0;
        ret = timer;
    }
    return(ret);
}


/// @brief  Clear ALL user timer tasks.
///
/// @return  void
MEMSPACE
void delete_all_timers()
{
    int i;
    for(i=0; i < MAX_TIMER_CNT; i++)
    {
        timer_irq[i].timer = 0;                   // Disable
        timer_irq[i].user_timer_handler = 0;
    }
}


/// @brief  subtract a-= b timespec * structures.
///
/// @param[in] a: timespec struct.
/// @param[in] b: timespec struct.
///
/// @return  void.
MEMSPACE
void subtract_timespec(ts_t *a, ts_t *b)
{
    a->tv_nsec = a->tv_nsec - b->tv_nsec;
    if (a->tv_nsec < 0L)
    {
        a->tv_nsec += 1000000000L;
        a->tv_sec --;
    }
    a->tv_sec = a->tv_sec - b->tv_sec;
}


/// @brief  timespec structure in seconds.nanoseconds in string.
///
/// @param[in] : timespec struct we wish to display.
/// @param[in] : output string.
///
/// @return  void.
static char _ts_to_str[32];
MEMSPACE
char * ts_to_str(ts_t *val)
{
    snprintf(_ts_to_str,31,"%ld.%09ld", val->tv_sec, val->tv_nsec);
    return( _ts_to_str );
}

/// @brief  timespec structure in seconds.nanoseconds.
///
/// @param[in] val: timespec struct we want to display.
///
/// @return  void.
MEMSPACE
void display_ts(ts_t *val)
{
    printf("[Seconds: %s]\n", ts_to_str(val) );
}


/// @brief Used for elapsed time calculations.
/// @see clock_elapsed_begin().
/// @see clock_elapsed_end().
static ts_t __clock_elapsed;

/// @brief Store current struct timespec in __clock_elapsed.
///
/// @see clock_gettime() POSIX function.
///
/// @return  void
MEMSPACE
void clock_elapsed_begin()
{
    clock_gettime(0, (ts_t *) &__clock_elapsed);
}


/// @brief Subtract and display time difference from clock_elapesed_begin().
///
/// - Notes: Accuracy is a function of timer resolution, CPU overhead and background tasks.
///
/// @param[in] msg: User message to proceed Time display.
///
/// @return  void
/// @see clock_gettime().
/// @see clock_elapesed_begin().
MEMSPACE
void clock_elapsed_end(char *msg)
{
    ts_t current;

    clock_gettime(0, (ts_t *) &current);

    subtract_timespec((ts_t *) &current, (ts_t *) &__clock_elapsed);

    if(msg && *msg)
        printf("[%s Time:%s]\n", msg, ts_to_str((ts_t *) &current) );
    else
        printf("[Time:%s]\n", ts_to_str((ts_t *) &current) );
}
