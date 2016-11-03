/**
 @file lib/timer.h

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

#ifndef __TIMER_H__
#define __TIMER_H__

///@brief Number of user timer tasks
#define MAX_TIMER_CNT 8

///@brief type of clockid_t.
typedef uint16_t clockid_t;

///@brief user timer struct
typedef struct
{
    void (*user_timer_handler)(void);             // user task
    uint8_t timer;                                // user task enabled ?
} TIMERS;

///@brief System task in HZ.
#ifndef SYSTEM_TASK_HZ
#error #define SYSTEM_TASK_HZ 1000L
#endif

///@brief System task in Nanoseconds.
#define SYSTEM_TASK_TIC_NS ( 1000000000L / SYSTEM_TASK_HZ )
///@brief System task in Microseconds.
#define SYSTEM_TASK_TIC_US ( 1000000L / SYSTEM_TASK_HZ )

///@brief Clock task in HZ defined as System task.
#define CLOCK_HZ SYSTEM_TASK_HZ
///@brief Clock task in Nanoseconds defined as System Task.
#define CLOCK_TIC_NS SYSTEM_TASK_TIC_NS
///@brief CLock task in Microseconds defined as System Task.
#define CLOCK_TIC_US SYSTEM_TASK_TIC_US

/* timer.c */
MEMSPACE int set_timers ( void (*handler )(void ), int timer );
MEMSPACE int kill_timer ( int timer );
MEMSPACE void delete_all_timers ( void );
MEMSPACE void subtract_timespec ( ts_t *a , ts_t *b );
MEMSPACE char *ts_to_str ( ts_t *val );
MEMSPACE void display_ts ( ts_t *val );
MEMSPACE void clock_elapsed_begin ( void );
MEMSPACE void clock_elapsed_end ( char *msg );
MEMSPACE void clock_clear ( void );
MEMSPACE void disable_timers ( void );
MEMSPACE void enable_timers ( void );
void execute_timers ( void );
MEMSPACE void clock_init ( void );
void clock_task ( void );
MEMSPACE void init_timers ( void );
MEMSPACE int clock_getres ( clockid_t clk_id , struct timespec *res );
MEMSPACE int clock_settime ( clockid_t clk_id , const struct timespec *ts );

extern MEMSPACE int clock_gettime ( clockid_t clk_id , struct timespec *ts );
/* timer_hal.c */
MEMSPACE void disable_system_task ( void );
MEMSPACE void enable_system_task ( void );
MEMSPACE void install_timers_isr ( void );

#endif   // _TIMER_H_
