/**
 @file user_main.c

 @brief Main user and initialization code
  This initialize the platform and runs the main user task.
  All display updates and task are called from here.

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

#ifdef WIRECUBE
	#include "wire.h"
	#include "cube_data.h"
#endif

#ifdef EARTH
	#include "wire.h"
	#include "earth_data.h"
#endif

/* Master Full size Window */
window *master;

/* Top Left status window */
window _winstats;
window *winstats = &_winstats;

/* Top Right Wireframe window */
window _windemo;
window *windemo = &_windemo;

/* Top Left status window */
window _wintest;
window *wintest = &_wintest;

/* Top Left status window */
window _wintestdemo;
window *wintestdemo = &_wintestdemo;

extern int DEBUG_PRINTF(const char *fmt, ...);
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);

/* user/user_main.c */
MEMSPACE LOCAL void init_done_cb ( void );
MEMSPACE LOCAL void HighTask ( os_event_t *events );
MEMSPACE LOCAL void NormalTask ( os_event_t *events );
MEMSPACE LOCAL void IdleTask ( os_event_t *events );
MEMSPACE LOCAL void UserTask ( os_event_t *events );
MEMSPACE LOCAL void sendMsgToUserTask ( void *arg );
LOCAL void user_task ( void );
MEMSPACE void user_init ( void );


// Display offset
int16_t xpos,ypos;
int16_t ip_xpos,ip_ypos;
// Rotation angle
LOCAL double degree = 0.0;
// Rotation increment
LOCAL double deg_inc = 4;
// Scale increment
LOCAL double dscale_inc;
// Scale factor
LOCAL double dscale;
// Scale maximum
LOCAL double dscale_max;

LOCAL long count = 0;
LOCAL int rad;
LOCAL point V;
LOCAL point S;

LOCAL uint32_t adc_sum = 0;
LOCAL int adc_count = 0;
double voltage = 0;

// Delay timer in milliseconds
#define USER_TASK_DELAY_TIMER     	30
#define RUN_TASK 			0
#define UserTaskPrio        USER_TASK_PRIO_0
#define UserTaskQueueLen  	4

#define HighTaskPrio        USER_TASK_PRIO_MAX
#define HighTaskQueueLen    1

#define NormalTaskPrio		USER_TASK_PRIO_2
#define NormalTaskQueueLen	8

#define IdleTaskPrio        USER_TASK_PRIO_0
#define IdleTaskQueueLen    1

LOCAL os_timer_t UserTimerHandler;

os_event_t	  UserTaskQueue[UserTaskQueueLen];
os_event_t    HighTaskQueue[HighTaskQueueLen];
os_event_t    NormalTaskQueue[NormalTaskQueueLen];
os_event_t    IdleTaskQueue[IdleTaskQueueLen];

int ntp_init = 0;
void ntp_setup(void)
{
    tv_t tv;
    tz_t tz;
	time_t sec;

    if(!network_init)
		return;

	if(ntp_init == 0)
    {
        ip_addr_t *addr = (ip_addr_t *)os_zalloc(sizeof(ip_addr_t));
		ipaddr_aton("192.168.200.1", addr);
		sntp_setserver(1,addr);
		ipaddr_aton("192.168.200.240", addr);
		sntp_setserver(2,addr);
        sntp_init();
        os_free(addr);
		ntp_init = 1;
        DEBUG_PRINTF("NTP:1\n");
    }
	if(ntp_init == 1)
	{
		// they hard coded it to +8 hours from GMT
		sec = sntp_get_current_timestamp();
		if(sec > 10)
		{
			ntp_init = 2;
		}
	}
    if(ntp_init == 2)
    {
		sntp_stop();
		DEBUG_PRINTF("NTP:2\n");

		// they return GMT + 8
        sec = sec - (8UL * 3600UL);

		// we are GMT - 4
        sec = sec - (4UL * 3600UL);

        tv.tv_sec = sec;
        tv.tv_usec = 0;
        tz.tz_minuteswest = 0;

        settimeofday(&tv, &tz);

        DEBUG_PRINTF("SEC:%ld\n",sec);
        DEBUG_PRINTF("TIME:%s\n", ctime(&sec));
		ntp_init = 3;
    }
}
 
/**
 @brief Callback for system_init_done_cb()
  Sends message to run task
 @return void
*/
MEMSPACE 
LOCAL void init_done_cb( void)
{
	DEBUG_PRINTF("System init done \r\n");
}

/**
 @brief High Priority Task Queue
 @param[in] events: event structure
 @return void
*/
MEMSPACE 
LOCAL void HighTask(os_event_t *events)
{
}

/**
 @brief Normal Priority Task Queue
 @param[in] events: event structure
 @return void
*/
MEMSPACE 
LOCAL void NormalTask(os_event_t *events)
{
}

/**
 @brief Idle Priority Task Queue
 @param[in] events: event structure
 @return void
*/
MEMSPACE 
LOCAL void IdleTask(os_event_t *events)
{
//Add task to add IdleTask back to the queue
	system_os_post(IdleTaskPrio, 0, 0);
}



/**
 @brief User Message handler task
  Runs corrected cube demo from Sem
  Optionally wireframe Earth viewer
 @param[in] events: event structure
 @return void
*/
MEMSPACE 
LOCAL void UserTask(os_event_t *events)
{
	switch(events->sig)
	{
		case RUN_TASK: 	
			user_task(); 
			break;
		default: 
			break;
	}
}



/**
 @brief Message passing function
  Sends message to run task
 @return void
*/
MEMSPACE 
LOCAL void sendMsgToUserTask(void *arg)
{
	system_os_post(USER_TASK_PRIO_0, RUN_TASK, 'a');
}

int signal_loop = 0;

/**
 @brief test task
  Runs corrected cube demo from Sem
  Optionally wireframe Earh viewer
 @return void
*/
LOCAL void user_task(void)
{
	uint32_t time1,time2;
	uint8_t red, blue,green;
	long timer = 0;
	uint16 system_adc_read(void);
	extern uint8_t ip_msg[];
	time_t sec;
	char buffer[256];


	

#ifdef WIRECUBE
	V.x = degree;
	V.y = degree;
	V.z = degree;
// Cube points were defined with sides of 1.0 
// We want a scale of +/- w/2
	wire_draw(windemo, cube_points, cube_edges, &V, windemo->w/2, windemo->h/2, dscale, 0);
	//wire_draw(windemo, cube_points, cube_edges, &V, windemo->w/2, windemo->h/2, dscale, 0);
#endif

#ifdef CIRCLE
	rad = dscale; // +/- 90
    tft_drawCircle(windemo, windemo->w/2, windemo->h/2, rad ,0);
	Display bounding circle that changes color around the cube
	if(dscale_inc < 0.0)
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
    tft_drawCircle(windemo, windemo->w/2, windemo->h/2, dscale, tft_color565(red,green,blue));
#endif

    degree += deg_inc;
    dscale += dscale_inc;

	if(degree <= -360)
		deg_inc = 4;
	if(degree >= 360)
		deg_inc = -4;

    if(dscale < dscale_max/2)
	{
	   dscale_inc = -dscale_inc;
	}
    if(dscale > dscale_max)
	{
	   dscale_inc = -dscale_inc;
	}


#ifdef WIRECUBE
	V.x = degree;
	V.y = degree;
	V.z = degree;
	//time1 = system_get_time();
	wire_draw(windemo, cube_points, cube_edges, &V, windemo->w/2, windemo->h/2, dscale, ILI9341_WHITE);
	//wire_draw(windemo, cube_points, cube_edges, &V, windemo->w/2, windemo->h/2, dscale, ILI9341_WHITE);
	//time2 = system_get_time();
#endif


// Get system voltage 33 = 3.3 volts
	adc_sum += system_adc_read();
	//adc_sum += system_get_vdd33();

	// FIXME atomic access
	if(++adc_count == 10)
	{
		voltage = ((double) adc_sum / 100.0); 
		adc_count = 0;
		adc_sum = 0;
	}

	// DEBUG_PRINTF("Degree: %d \r\n",(int)degree);
	// cube redraw count
	count += 1;
	tft_set_font(winstats,0);
	tft_setpos(winstats,ip_xpos,ip_ypos);
	tft_printf(winstats,"%-26s\n", ip_msg);
	if(!signal_loop--)
	{
		signal_loop = 100;
		tft_printf(winstats,"CH:%02d, DB:-%02d\n", 
			wifi_get_channel(),
			wifi_station_get_rssi());
		signal_loop = 0;
	}
	tft_setpos(winstats,xpos,ypos);
	tft_printf(winstats,"Heap: %d\n", system_get_free_heap_size());
	tft_printf(winstats,"Iter:% 9ld, %+7.2f\n", count, degree);
	
	// NTP state machine
	ntp_setup();

	// get current time
	time(&sec);

	tft_printf(winstats,"Volt:%2.2f\n%s\n", (float)voltage, ctime(&sec));
	
#ifdef NETWORK_TEST
	poll_network_message(wintest);
#endif

// Buffered get line uses interrupts and queues
	if(uart0_gets(buffer,255))
	{
		DEBUG_PRINTF("line:%s\n",buffer);
		fatfs_tests(buffer);
	}
}

#if ILI9341_DEBUG & 1
MEMSPACE
void read_tests(window *win)
{
	int x,y;
	uint16_t color;
	uint16_t buffer[3*16];	// 16 RGB sets
	y = 4;
	tft_readRect(win, x, y, 16, 1, buffer);
	for(x=0;x<16;++x)
	{
		//color = tft_readPixel(win,x,y);
		// DEBUG_PRINTF("x:%d,y:%d,c:%04x\n", x,y,color);
		DEBUG_PRINTF("x:%d,y:%d,c:%04x\n", x,y,buffer[x]);
	}
	 DEBUG_PRINTF("\n");
}
#endif


#ifdef TEST_FLASH
ICACHE_RODATA_ATTR uint8_t xxx[] = { 1,2,3,4,5,6,7,8 };
/// @brief Test flash read code
/// @param[in] *win: window structure
/// @return  void
MEMSPACE
void test_flashio(window *win)
{
    uint16_t xpros,ypos;
    tft_set_font(win,1);
    //tft_printf(win, "%x,%x", xxxp[0],xxxp[1]);
    xpos = win->x;
    ypos = win->y;
    for(i=0;i<8;++i)
    {
        tft_setpos(win, i*16,ypos);
        tft_printf(win, "%02x", read_flash8((uint32_t) &xxx+i));
    }
    tft_printf(win, "\n");
    tft_printf(win, "%08x, %08x", read_flash32((uint8_t *)&xxx), read_flash16((uint8_t *)&xxx));
}
#endif



/**
 @brief main() Initialize user task
 @return void
*/
MEMSPACE 
void user_init(void)
{
	int i;
	os_event_t *handlerQueue;
    char time[20];
	int ret;
	uint16_t *ptr;
	uint32_t time1,time2;
	uint32_t ID;
	extern uint16_t tft_ID;
	double ang;

	os_delay_us(200000L);	// Power Up dalay - lets power supplies and devices settle

	// Configure the UART
	uart_init(BIT_RATE_115200,BIT_RATE_115200);

	DEBUG_PRINTF("\n");
	DEBUG_PRINTF("System init...\n");

	DEBUG_PRINTF("HSPI init...\n");
	hspi_init(1,0);

	DEBUG_PRINTF("Timers init...\n");
	init_timers();

	DEBUG_PRINTF("SD Card init...\n");
	mmc_init(1);

// CPU
// 160MHZ
//  REG_SET_BIT(0x3ff00014, BIT(0));
// 80MHZ
//   REG_CLR_BIT(0x3ff00014, BIT(0));


	DEBUG_PRINTF("Display Init\n");
	// Initialize TFT
	master = tft_init();
	ID = tft_ID;
	// Set master rotation
	tft_setRotation(1);

	/* Setup main status window */
	tft_window_init(winstats,0,0, master->w * 7 / 10, master->h/2);

	tft_setpos(winstats, 0,0);
	tft_set_font(winstats,5);
	tft_font_var(winstats);
	tft_setTextColor(winstats, ILI9341_RED,0);
	tft_printf(winstats, "DISP ID: %04lx\n", ID);
	ip_xpos = winstats->x;
	ip_ypos = winstats->y;
	tft_printf(winstats, "\n");
	tft_setTextColor(winstats, ILI9341_WHITE,0);
#if ILI9341_DEBUG & 1
	DEBUG_PRINTF("\nDisplay ID=%08lx\n",ID);
#endif
	tft_font_fixed(winstats);

	// Save last display offset of the status update line - used in the demo task
	xpos = winstats->x;
	ypos = winstats->y;

	/* Setup cube/wireframe demo window */
	tft_window_init(windemo,winstats->w,0, master->w - winstats->w, master->h/2);

// Cube points were defined with sides of 1.0 
// We want a scale of +/- w/2
	if(windemo->w < windemo->h) 
		dscale_max = windemo->w/2;
	else
		dscale_max = windemo->h/2;

	dscale = dscale_max;
	dscale_inc = dscale_max / 100;

/* Setup second window for window testing*/
	tft_window_init(wintest,0,master->h/2, master->w/2, master->h/2);
	tft_setTextColor(wintest, ILI9341_WHITE,ILI9341_NAVY);
    tft_fillWin(wintest, wintest->bg);
	tft_set_font(wintest,3);
	//tft_font_var(wintest);

	// write some text
	tft_setpos(wintest, 0,0);
	tft_printf(wintest, "Test1\nTest2\nTest3");

	/* Test demo area window */
	tft_window_init(wintestdemo,master->w/2,master->h/2,master->w/2, master->h/2);
	tft_setTextColor(wintestdemo, ILI9341_WHITE,0);
    tft_fillWin(wintestdemo, wintestdemo->bg);
	tft_set_font(wintestdemo,3);
	tft_font_var(wintestdemo);

#if ILI9341_DEBUG & 1
	DEBUG_PRINTF("Test Display Read\n");
	read_tests(wintest);
#endif

/* Draw cube in the second window as a test */
#if defined(WIRECUBE) && !defined(EARTH)
	DEBUG_PRINTF("Draw Cube\n");
// Cube points were defined with sides of 1.0 
// We want a scale of +/- w/2
	double tscale_max;
	if(wintestdemo->w < wintestdemo->h) 
		tscale_max = wintestdemo->w/2;
	else
		tscale_max = wintestdemo->h/2;
	ang = 45.0;
	V.x = ang;
	V.y = ang;
	V.z = ang;
	wire_draw(wintestdemo, cube_points, cube_edges, &V, wintestdemo->w/2, wintestdemo->h/2, tscale_max, ILI9341_RED);
#endif

#ifdef EARTH
	DEBUG_PRINTF("Draw Earth\n");
// Earth points were defined with radius of 0.5, diameter of 1.0
// We want a scale of +/- w/2
	double tscale_max;
	if(wintestdemo->w < wintestdemo->h) 
		tscale_max = wintestdemo->w;
	else
		tscale_max = wintestdemo->h;
	V.x = -90;
	V.y = -90;
	V.z = -90;
	// draw earth
	//time1 = system_get_time();
// Earth points were defined over with a scale of -0.5/+0.5 scale - so scale must be 1 or less
	wire_draw(wintestdemo, earth_data, NULL, &V, wintestdemo->w/2, wintestdemo->h/2, tscale_max, wintestdemo->fg);
	//time2 = system_get_time();
#endif

	wdt_reset();

	DEBUG_PRINTF("User Task\n");
	// Set up a timer to send the message to User Task
	os_timer_disarm(&UserTimerHandler);
	os_timer_setfn(&UserTimerHandler,(os_timer_func_t *)sendMsgToUserTask,(void *)0);
	os_timer_arm(&UserTimerHandler, USER_TASK_DELAY_TIMER, 1);
	// Setup the user task
	system_os_task(UserTask, UserTaskPrio, UserTaskQueue, UserTaskQueueLen);

#ifdef TELNET_SERIAL
	DEBUG_PRINTF("Setup Network Serial Bridge\n");
	bridge_task_init(23);
#endif

#ifdef NETWORK_TEST
	DEBUG_PRINTF("Setup Network TFT DIsplay Client\n");
	setup_networking(TCP_PORT);
#endif

#if 0
	// Misc Task init - testing only
	system_os_task(HighTask, HighTaskPrio, HighTaskQueue, HighTaskQueueLen);
	system_os_task(NormalTask, NormalTaskPrio, NormalTaskQueue, NormalTaskQueueLen);
	system_os_task(IdleTask, IdleTaskPrio, IdleTaskQueue, IdleTaskQueueLen);
	system_os_post(IdleTaskPrio, 0, 0);
#endif
	DEBUG_PRINTF("Done Setup\n");
    DEBUG_PRINTF("Heap Size(%d) bytes\n" , system_get_free_heap_size());

	// disable os_printf at this time
	system_set_os_print(0);

    system_init_done_cb(init_done_cb);
}
