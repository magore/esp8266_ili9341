/**
 @file lib/timer_hal.c

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

#include "user_config.h"

#include "lib/time.h"
#include "lib/timer.h"

// =============================================
/// @brief ESP8266 code
#ifdef ESP8266
static ETSTimer task_1ms;

/// @brief Disable interrupts
/// @return void.
MEMSPACE
void disable_system_task()
{
	os_timer_disarm(&task_1ms);
}

/// @brief Enable interrupts
/// @return void.
MEMSPACE
void enable_system_task()
{
	os_timer_arm(&task_1ms, 1, 1);
}

/// @brief Setup main timers ISR - this ISR calls execute_timers() task.
/// @return void.
MEMSPACE void install_timers_isr()
{
	os_timer_disarm(&task_1ms);
	os_timer_setfn(&task_1ms, ( os_timer_func_t *) execute_timers, NULL );
}
#endif	// ifdef ESP8266
// =============================================

// =============================================
/// @brief AVR code
#ifdef AVR
/// @brief AVR specific code
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

/// @brief Disable interrupts
/// @return void.
/// FIXME modify to just disable the timer IRQ
MEMSPACE
void disable_system_task()
{
	cli();
}

/// @brief Enable interrupts
/// @return void.
MEMSPACE
void enable_system_task()
{
	sei();
}

/// @brief Setup main timers ISR 
///       This ISR calls execute_timers() task.
///
/// - AVR Notes:
///  - We attempt to use the largest reload count for a given SYSTEM_HZ interrupt rate.
///    This permits using hardware counter offset to increase resolution to the best 
///    possible amount.
/// - Assumptions:
///  - We can divide the CPU frequency EXACTLY with timer/counter 
///    having no fractional remander.
///
/// @see ISR().
/// @return void.
MEMSPACE void install_timers_isr()
{
    cli();
    TCCR1B=(1<<WGM12) | TIMER1_PRE_1; 	// No Prescale
    TCCR1A=0;
    OCR1A=(TIMER1_COUNTS_PER_TIC-1);	// 0 .. count
    TIMSK1 |= (1<<OCIE1A);				//Enable the Output Compare A interrupt
    sei();
}

/// @brief AVR Timer Interrupt Vector
///
/// - calls execute_timers() - we call this the System task.
///
ISR(TIMER1_COMPA_vect)
{
    execute_timers();
}

#ifdef HAVE_HIRES_TIMER
/// @brief Read clock time into struct timepec *ts - POSIX function.
///
///  - Note: We ignore clk_id, and include low level counter offsets when available.
///
/// @param[in] clk_id:	unused hardware clock index.
/// @param[out] ts:		timespec result.
///
/// @return 0 on success.
/// @return -1 on error.
MEMSPACE
int clock_gettime(clockid_t clk_id, struct timespec *ts)
{
    uint16_t count1,count2;                       // must be same size as timer register
    uint32_t offset = 0;
    uint8_t pendingf = 0;
    int errorf = 0;

	// disable interrupts
    cli();

    count1 = TCNT1;

    ts->tv_sec = __clock.tv_sec;
    ts->tv_nsec = __clock.tv_nsec;

    count2 = TCNT1;

    if( TIFR1 & (1<<OCF1A) )
        pendingf = 1;

    if (count2 < count1)
    {
///  note: counter2 < count1 implies ISR flag must be set
        if( !pendingf )
            errorf = -1;    // counter overflow and NO pending is an error!
        offset = TIMER1_COUNTS_PER_TIC;           // overflow
    }
    else
    {
        if( pendingf )
            offset = TIMER1_COUNTS_PER_TIC;       // overflow
    }
    offset += count2;

	// enable interrupts
    sei();

    offset *= TIMER1_COUNTER_RES;

    ts->tv_nsec += offset;

    if (ts->tv_nsec >= 1000000000L)
    {
        ts->tv_nsec -= 1000000000L;
        ts->tv_sec++;
    }
    return(errorf);
}
#endif	// ifdef HAVE_HIRES_TIMER

#endif	// ifdef AVR
// =============================================

