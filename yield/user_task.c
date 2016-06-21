/*
 user_task.c - was main.cpp - platform initialization and context switching
 emulation

 Copyright (c) 2014 Ivan Grokhotkov. All rights reserved.
 This file is part of the esp8266 core for Arduino environment.

 This library is free software; you can redistribute it and/or
 modify it under the terms of the GNU Lesser General Public
 License as published by the Free Software Foundation; either
 version 2.1 of the License, or (at your option) any later version.

This library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public
License along with this library; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/

//This may be used to change user task stack size:
// Can override stack in cont.h by defining it here
// FIXME DEBUG
#define CONT_STACKSIZE 4096
#include "user_config.h"

#include "ets_sys.h"
#include "os_type.h"
#include "osapi.h"
#include "mem.h"
#include "user_interface.h"

#define LOOP_TASK_PRIORITY 0
#define LOOP_QUEUE_SIZE    1

#define OPTIMISTIC_YIELD_TIME_US 16000

struct rst_info resetInfo;

int atexit(void (*func)())
{
    return 0;
}


extern void loop();
extern void setup();

cont_t g_cont __attribute__ ((aligned (16)));
static os_event_t g_loop_queue[LOOP_QUEUE_SIZE];

uint32_t g_micros_at_task_start;

void abort()
{
	printf("\nABORT!\n");
    do
    {
        *((int*)0) = 0;
    } while(true);
}


void esp_yield()
{
// FIXME DEBUG
	hspi_waitReady();

    if (cont_can_yield(&g_cont))
    {
        cont_yield(&g_cont);
    }
}


void esp_schedule()
{
    system_os_post(LOOP_TASK_PRIORITY, 0, 0);
}


//void __yield()
void yield()
{
    if (cont_can_yield(&g_cont))
    {
        esp_schedule();
        esp_yield();
    }
    else
    {
        abort();
    }
}

//void yield(void) __attribute__ ((weak, alias("__yield")));


void optimistic_yield(uint32_t interval_us)
{
    if (cont_can_yield(&g_cont) &&
        (system_get_time() - g_micros_at_task_start) > interval_us)
    {
        yield();
    }
}


bool setup_done = false;
void loop_wrapper()
{
	extern void loop(void);
	extern void web_task();

    if(!setup_done)
    {
        setup();
        setup_done = true;
    }
// FIXME DEBUG
	REG_SET_BIT(0x3ff00014, BIT(0));
	hspi_waitReady();

#ifdef WEBSERVER
	web_task();
#endif

// USER TASK
    loop();
    esp_schedule();
}


static void loop_task(os_event_t *events)
{
    g_micros_at_task_start = system_get_time();
    cont_run(&g_cont, &loop_wrapper);
    if(cont_check(&g_cont) != 0)
    {
        printf("\nsketch stack overflow detected\n");
        abort();
    }
}


void init_done()
{
printf("\nInit Done with Yield support!\n");
printf("=============================\n");
	// disable os_printf at this time
	//system_set_os_print(0);

    esp_schedule();
}


void user_init(void)
{

    if(!setup_done)
    {
        setup();
        setup_done = true;
    }

    cont_init(&g_cont);

    system_os_task(loop_task,
        LOOP_TASK_PRIORITY, g_loop_queue,
        LOOP_QUEUE_SIZE);

	printf("loop_task installed\n");

    system_init_done_cb(&init_done);
}
