/**
 @file user_main.c

 @par Copyright &copy; 2015 Mike Gore, GPL License

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option)
 any later version.

 user_main.c is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "user_config.h"

#ifdef WIRECUBE
	#include "wire.h"
	#include "cube_data.h"
#endif

#ifdef EARTH
	#include "wire.h"
	#include "earth_data.h"
#endif

extern int ets_uart_printf(const char *fmt, ...);
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);

LOCAL os_timer_t timerHandler;

// Rotation angle
LOCAL double degree = 0.0;
// Scale factor
LOCAL double scale = 1.0;
// Rotation increment
LOCAL double deg_inc = 4;
// Scale increment
LOCAL double scale_inc = .1;

LOCAL long count = 0;
int rad = 80;
point V;
point S;

/**
 @brief test task
  Runs corrected cube demo from Sem
  Optionally wireframe Earh viewer
 @return void
*/
static void test(void)
{
    char time[20];
	uint32_t time1,time2;

	uint8_t red, blue,green;

#ifdef WIRECUBE
	V.x = degree;
	V.y = degree;
	V.z = degree;
	wire_draw(wire_cube, &V, scale, 0);
#endif

#ifdef EARTH
	// erase earth
	V.x = -90;
	V.y = -90;
	V.z = -90;
	wire_draw(earth_data, &V, scale, 0);
#endif

#ifdef CIRCLE
    tft_drawCircle(120, 160, rad,0);
	rad = scale * 60; // +/- 90
	Display bounding circle that changes color around the cube
	if(scale_inc < 0.0)
	{
		red = 255;
		blue = 0;
		green = 0;
	}
	else
	{
		red = 0;
		blue = 255;
		green = 0;
	}
	// RGB - YELLOW
    tft_drawCircle(120, 160, rad, tft_color565(red,green,blue));
#endif

    degree += deg_inc;
    scale += scale_inc;

	if(degree <= -360)
		deg_inc = 4;
	if(degree >= 360)
		deg_inc = -4;

    if (scale < 0.5)
        scale_inc = .1;
    if (scale > 2.5)
        scale_inc = -.1;

#ifdef EARTH
	scale = 2.5;

	V.x = -90;
	V.y = -90;
	V.z = -90;
	// draw earth
	time1 = system_get_time();
	wire_draw(earth_data, &V, scale, 0xffff);
	time2 = system_get_time();
#endif

#ifdef WIRECUBE
	V.x = degree;
	V.y = degree;
	V.z = degree;
	time1 = system_get_time();
	wire_draw(wire_cube, &V, scale, 0xffff);
	time2 = system_get_time();
#endif

	// ets_uart_printf("Degree: %d \r\n", (int)degree);
	count += 1;
	tft_printf(0,20,0,"c:% 9ld, %+7.2f", count, degree);
	tft_printf(0,40,0,"t:% 9ld", time2-time1);
}

/**
 @brief Message passing function
  Sends message to run task
 @return void
*/
MEMSPACE void sendMsgToHandler(void *arg)
{
	system_os_post(USER_TASK_PRIO_0, RUN_TEST, 'a');
}

/**
 @brief Message handler task
  Runs corrected cube demo from Sem
  Optionally wireframe Earh viewer
 @return void
*/
MEMSPACE void handler_task (os_event_t *e)
{
	switch (e->sig)
	{
		case RUN_TEST: test(); break;
		default: break;
	}
}

#ifdef TEST_FLASH
ICACHE_RODATA_ATTR uint8_t xxx[] = { 1,2,3,4,5,6,7,8 };
#endif

/**
 @brief Initialize user tassk
 @return void
*/
MEMSPACE void user_init(void)
{
	int i;
	os_event_t *handlerQueue;
	uint32_t ID;
    char time[20];
	int ret;
	uint32_t time1,time2;

	os_delay_us(200000L);	// Power Up dalay - lets power supplies and devices settle

	// Configure the UART
	uart_init(BIT_RATE_115200,BIT_RATE_115200);
	ets_uart_printf("\r\nSystem init...\r\n");
	ets_uart_printf("\r\nDisplay Init\r\n");
	// Initialize TFT
	ID = tft_init();

	tft_font_var();
	tft_printf(0,0,3, "DISP ID: %04lx", ID);

	tft_font_fixed();

#ifdef TEST_FLASH
	//tft_printf(0,48,1, "%x,%x", xxxp[0],xxxp[1]);
	for(i=0;i<8;++i)
	{
		tft_printf(i*16,48,1, "%02x", read_flash8((uint32_t) &xxx+i));
	}
	tft_printf(0,0,1, "%08x, %08x", read_flash32((uint8_t *)&xxx), read_flash16((uint8_t *)&xxx));
#endif
	//ID = tft_readId();
	ets_uart_printf("\r\nDisplay ID=%08lx\r\n",ID);
	ets_uart_printf("\r\nDisplay Init Done\r\n");


	ets_wdt_disable();

	// Set up a timer to send the message to handler
	os_timer_disarm(&timerHandler);
	os_timer_setfn(&timerHandler, (os_timer_func_t *)sendMsgToHandler, (void *)0);
	os_timer_arm(&timerHandler, DELAY_TIMER, 1);

	// Set up a timerHandler to send the message to handler
	handlerQueue = (os_event_t *)os_malloc(sizeof(os_event_t)*TEST_QUEUE_LEN);
	system_os_task(handler_task, USER_TASK_PRIO_0, handlerQueue, TEST_QUEUE_LEN);

	ets_uart_printf("System init done \r\n");
}
