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

#include "user_config.h"

#ifdef AVR
#include <stdlib.h>
#define HAVE_HIRES_TIMER /*< We can read a high resolution hardware timer */
#endif

#include "printf/mathio.h"

#include "lib/time.h"
#include "lib/timer.h"

/// @brief  System Clock Time
volatile ts_t __clock;

/// @brief  System Time Zone
tz_t __tzone;

static int timers_enabled = 0;
static int timers_configured = 0;

/// @brief  array or user timers
TIMERS timer_irq[MAX_TIMER_CNT];


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

/// @brief  clear time and timezone to 0.
///
/// @see clock_settime().
/// @see settimezone().
///
/// @return  void.
MEMSPACE
void clock_clear()
{
    struct timespec ts;
    ts.tv_nsec = 0;
    ts.tv_sec = 0;
    clock_settime(0, &ts);
    __tzone.tz_minuteswest = 0;
    __tzone.tz_dsttime = 0;
}

/// @brief  Disable all timer tasks
///
/// @return  void
MEMSPACE
void disable_timers()
{
    if(timers_configured && timers_enabled)
    {
		disable_system_task();
        timers_enabled = 0;
    }
}

/// @brief  Enable timer tasks
///
/// @return  void
MEMSPACE
void enable_timers()
{
    if(timers_configured && !timers_enabled)
    {
		enable_system_task();
        timers_enabled = 1;
    }
}

/// @brief  Execute all user timers at SYSTEM_HZ rate.
///  Called by system task
///
/// @return  void
void execute_timers()
{
    int i;

    for(i=0; i < MAX_TIMER_CNT; i++)
    {
        if(timer_irq[i].timer && timer_irq[i].user_timer_handler != NULL)
            (*timer_irq[i].user_timer_handler)();
    }
}

/**
 @brief 1000HZ timer task
 @return void
*/
void clock_task(void)
{
    __clock.tv_nsec += 1000000;
    if(__clock.tv_nsec >= 1000000000L)
    {
        __clock.tv_sec++;
        __clock.tv_nsec = 0;
    }
}

/// @brief  Setup all timers tasksi and enable interrupts
///
/// @see clock_task()
/// @see: timer_hal.c for hardware dependent interface
//
/// @return  void
MEMSPACE
void init_timers()
{
    printf("Timers init called\n");

    if(!timers_configured)
    {
		install_timers_isr();
        timers_configured = 1;
        timers_enabled = 0;
        printf("Timers configured\n");
    }

    delete_all_timers();

    clock_clear();
    printf("Clock Init\n");

///  See time.c
    if(set_timers(clock_task,1) == -1)
        printf("Clock task init failed\n");
    printf("Clock Installed\n");

    enable_timers();

    printf("Timers enabled\n");
}


/// @brief Read clock time resolution into struct timepec *ts - POSIX function.
///  - Note: We ignore clk_id
/// @param[in] clk_id:  unused hardware clock index.
/// @param[out] res:        timespec resolution result.
///
/// @return 0 on success.
/// @return -1 on error.
MEMSPACE
int clock_getres(clockid_t clk_id, struct timespec *res)
{
    res->tv_sec = 0;
    res->tv_nsec = SYSTEM_TASK_TIC_NS;
    return(0);
}


/// @brief Set system clock using seconds and nonoseconds - POSIX function.
///
/// @param[in] clk_id: Hardware timer index used when there are more then one.
///  - Note: We ignore clk_id for now.
/// @param[in] ts: struct timespec input.
///
/// @return 0.
MEMSPACE
int clock_settime(clockid_t clk_id, const struct timespec *ts)
{
    if(clk_id)
        return(-1);

    while(1)
    {
        __clock.tv_nsec = ts->tv_nsec;
        __clock.tv_sec  = ts->tv_sec;

        if(ts->tv_nsec != __clock.tv_nsec || ts->tv_sec != __clock.tv_sec)
            continue;
        break;
    }

    return(0);
}

#if ! defined(HAVE_HIRES_TIMER) 
/// Generic clock_gettime() function WITHOUT high resolution 

/// @brief Read clock time into struct timepec *ts - POSIX function.
///
///  - Note: We ignore clk_id, and include low level counter offsets when available.
///
/// @param[in] clk_id:  unused hardware clock index.
/// @param[out] ts:     timespec result.
///
/// @return 0 on success.
/// @return -1 on error.
//TODO try to use system_get_time() to get microsecond offsets
MEMSPACE
int clock_gettime(clockid_t clk_id, struct timespec *ts)
{
    ts->tv_nsec = 0;
    ts->tv_sec = 0;

    while(1)
    {
        ts->tv_nsec = __clock.tv_nsec;
        ts->tv_sec = __clock.tv_sec;
        if(ts->tv_nsec != __clock.tv_nsec || ts->tv_sec != __clock.tv_sec)
            continue;
        break;
    }
    return(0);
}

#endif
