/**
 @file lib/timer_hal.h

 @brief timer hardware abstartion layer functions

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


#ifndef _TIMER_HAL_H_
#define _TIMER_HAL_H_

#include <user_config.h>
#include "timer.h"
#include "time.h"

#ifdef AVR
/*< We can read a high resolution hardware timer */
#define HAVE_HIRES_TIMER 

/// @brief AVR prescale divider.
#define TIMER1_PRESCALE     1L


///@brief Computer AVR counts per interrupt.
#define TIMER1_COUNTS_PER_TIC (F_CPU/TIMER1_PRESCALE/SYSTEM_TASK_HZ)

///@brief Computer AVR count time in Nanoseconds per counter increment.
#define TIMER1_COUNTER_RES (SYSTEM_TASK_TIC_NS/TIMER1_COUNTS_PER_TIC)

#if TIMER1_COUNTS_PER_TIC >= 65535L
#error TIMER1_COUNTS_PER_TIC too big -- increase TIMER1 Prescale
#endif

#define TIMER1_PRE_1 (1 << CS10)                        /*< 1 Prescale */
#define TIMER1_PRE_8 (1 << CS11)                        /*< 8 Prescale */
#define TIMER1_PRE_64 ((1 << CS11) | ( 1 << CS10))      /*< 64 Prescale */
#define TIMER1_PRE_256 (1 << CS12)                      /*< 256 Prescale */
#define TIMER1_PRE_1024 ((1 << CS12) | ( 1 << CS10))    /*< 1024 Prescape */
#endif

/* timer_hal.c */
MEMSPACE void clock_clear ( void );
MEMSPACE void disable_timers ( void );
MEMSPACE void enable_timers ( void );
void execute_timers ( void );
MEMSPACE void clock_init ( void );
void clock_task ( void );
MEMSPACE void init_timers ( void );
MEMSPACE int clock_getres ( clockid_t clk_id , struct timespec *res );
MEMSPACE void setup_timers_isr ( void );
MEMSPACE int clock_settime ( clockid_t clk_id , const struct timespec *ts );
MEMSPACE int clock_gettime ( clockid_t clk_id , struct timespec *ts );


#endif                                            // _TIMER_HAL_H_
