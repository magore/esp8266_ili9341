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

#include "user_config.h"
#include "time.h"
#include "timer.h"

/// @brief  array or user timers
extern TIMERS timer_irq[];

/* lib/timer_hal.c */
MEMSPACE void clock_clear ( void );
MEMSPACE void disable_timers ( void );
MEMSPACE void enable_timers ( void );
void execute_timers ( void *arg );
MEMSPACE void clock_init ( void );
LOCAL void clock_task ( void );
MEMSPACE void init_timers ( void );
MEMSPACE int clock_getres ( clockid_t clk_id , struct timespec *res );
MEMSPACE int clock_gettime ( clockid_t clk_id , struct timespec *ts );
MEMSPACE int clock_settime ( clockid_t clk_id , const struct timespec *ts );



#endif                                            // _TIMER_HAL_H_
