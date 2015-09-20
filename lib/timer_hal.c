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


#include <user_config.h>

#include "timer.h"
#include "time.h"

/// @brief  System Clock Time
volatile ts_t __clock;

/// @brief  System Time Zone
tz_t __tzone;


static int timers_enabled = 0;
static int timers_configured = 0;

/// @brief  array or user timers
TIMERS timer_irq[MAX_TIMER_CNT];

#ifdef ESP8266
    static ETSTimer task_1ms;
#endif


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
#ifdef ESP8266
        os_timer_disarm(&task_1ms);
#endif
#ifdef AVR
        cli();
#endif
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
#ifdef ESP8266
        os_timer_arm(&task_1ms, 2, 1);
#endif
#ifdef AVR
		sei();
#endif
        timers_enabled = 1;
    }
}
/// @brief  Execute all user timers at SYSTEM_HZ rate.
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


/// @brief  Initialize clock task
/// @return  void.
MEMSPACE
void clock_init()
{
    clock_clear();
///  See time.c
    if(set_timers(clock_task,1) == -1)
        printf("Clock task init failed\n");
}

/**
 @brief 1000HZ timer task
 @param[in] *arg: ignored
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
		setup_timers_isr();
        timers_configured = 1;
        timers_enabled = 0;
        printf("Timers configured\n");
    }

    delete_all_timers();
///  See time.c
    clock_init();

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




/// @brief Setup main timers ISR - this ISR calls execute_timers() task.
///
/// - Notes:
///  - We attempt to use the largest reload count for a given SYSTEM_HZ interrupt rate. This permits using hardware counter offset to increase resolution to the best possible amount.
/// - Assumptions:
///  - We can divide the CPU frequency EXACTLY with timer/counter having no fractional remander.
///
/// @see ISR().
/// @return void.
MEMSPACE void setup_timers_isr()
{
#ifdef ESP8266
	os_timer_disarm(&task_1ms);
	os_timer_setfn(&task_1ms, ( os_timer_func_t *) execute_timers, NULL );
#endif
#ifdef AVR
    cli();

    TCCR1B=(1<<WGM12) | TIMER1_PRE_1;             // No Prescale
    TCCR1A=0;
    OCR1A=(TIMER1_COUNTS_PER_TIC-1);              // 0 .. count
    TIMSK1 |= (1<<OCIE1A);                        //Enable the Output Compare A interrupt

    sei();
#endif
}

#ifdef AVR
/// AVR specific code

/// @brief AVR Timer Interrupt Vector
///
/// - calls execute_timers() - we call this the System task.
///
ISR(TIMER1_COMPA_vect)
{
    execute_timers();
}
#endif

/// @brief Set system clock using seconds and nonoseconds - POSIX function.
///
/// @param[in] clk_id: Hardware timer index used when there are more then one.
///  - Note: We ignore clk_id for now.
/// @param[in] tp: struct timespec input.
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


#if defined(HAVE_HIRES_TIMER) | defined(AVR)
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
            errorf = -1;                          // counter overflow and NO pending is an error!

        offset = TIMER1_COUNTS_PER_TIC;           // overflow
    }
    else
    {
        if( pendingf )
            offset = TIMER1_COUNTS_PER_TIC;       // overflow
    }
    offset += count2;

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

#else
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
