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

extern int ets_uart_printf(const char *fmt, ...);
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);

/* user/user_main.c */
MEMSPACE LOCAL void seconds_task ( void *arg );
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

LOCAL adc_sum = 0;
LOCAL adc_count = 0;
double voltage = 0;

// Delay timer in milliseconds
#define DELAY_TIMER     	10 
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


/** 
	@brief 1 Second timer task data
*/
LOCAL ETSTimer timer1sec;
LOCAL long seconds;

/**
 @brief 1 Second timer task
 @return void
*/
MEMSPACE 
LOCAL void seconds_task( void * arg) 
{
    seconds++;
}
 
/**
 @brief Callback for system_init_done_cb()
  Sends message to run task
 @return void
*/
MEMSPACE 
LOCAL void init_done_cb( void)
{
    os_timer_disarm(&timer1sec);
    os_timer_setfn(&timer1sec, ( os_timer_func_t *) seconds_task , NULL );
    os_timer_arm(&timer1sec, 1000 , 1);
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


/**
 @brief test task
  Runs corrected cube demo from Sem
  Optionally wireframe Earh viewer
 @return void
*/
LOCAL void user_task(void)
{
    char time[20];
	uint32_t time1,time2;
	uint8_t red, blue,green;
	long timer = 0;
	uint16 system_adc_read(void);
	extern uint8_t ip_msg[];


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

	// FIXME atomic access
	if(++adc_count == 10)
	{
		voltage = ((double) adc_sum / 100.0); 
		adc_count = 0;
		adc_sum = 0;
	}

	// ets_uart_printf("Degree: %d \r\n",(int)degree);
	// cube redraw count
	count += 1;
	tft_set_font(winstats,0);
	tft_setpos(winstats,ip_xpos,ip_ypos);
	tft_printf(winstats,"%s\n", ip_msg);
	tft_setpos(winstats,xpos,ypos);
	tft_printf(winstats,"Iter:% 9ld, %+7.2f\n", count, degree);
	// FIXME make this an atomic access
	timer = seconds;
	tft_printf(winstats,"Voltage: %2.2f\nTime: %9ld", (float)voltage, timer);
	
#ifdef NETWORK_TEST
	poll_network_message(wintest);
#endif
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
		// ets_uart_printf("x:%d,y:%d,c:%04x\n", x,y,color);
		ets_uart_printf("x:%d,y:%d,c:%04x\n", x,y,buffer[x]);
	}
	 ets_uart_printf("\n");
}
#endif

/**
 @brief Initialize user task
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
	ets_uart_printf("\r\nSystem init...\r\n");
	ets_uart_printf("\r\nDisplay Init\r\n");

// CPU
// 160MHZ
//  REG_SET_BIT(0x3ff00014, BIT(0));
// 80MHZ
//   REG_CLR_BIT(0x3ff00014, BIT(0));

	hspi_init(1);
	// Initialize TFT
	master = tft_init();
	ID = tft_ID;
	// Set master rotation
	tft_setRotation(1);

	/* Setup main status window */
	tft_window_init(winstats,0,0, master->w * 6 / 10, master->h/2);

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
	ets_uart_printf("\r\nDisplay ID=%08lx\r\n",ID);
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
	read_tests(wintest);
#endif

/* Draw cube in the second window as a test */
#if defined(WIRECUBE) && !defined(EARTH)
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

	ets_uart_printf("\r\nDisplay Init Done\r\n");

	ets_wdt_disable();

	// Set up a timer to send the message to User Task
	os_timer_disarm(&UserTimerHandler);
	os_timer_setfn(&UserTimerHandler,(os_timer_func_t *)sendMsgToUserTask,(void *)0);
	os_timer_arm(&UserTimerHandler, DELAY_TIMER, 1);

	// Setup the user task
	system_os_task(UserTask, UserTaskPrio, UserTaskQueue, UserTaskQueueLen);

#if 0
	// Task init
	system_os_task(HighTask, HighTaskPrio, HighTaskQueue, HighTaskQueueLen);
	system_os_task(NormalTask, NormalTaskPrio, NormalTaskQueue, NormalTaskQueueLen);
	system_os_task(IdleTask, IdleTaskPrio, IdleTaskQueue, IdleTaskQueueLen);
	system_os_post(IdleTaskPrio, 0, 0);
#endif

    seconds = 0;
    system_init_done_cb(init_done_cb);

#ifdef NETWORK_TEST
	setup_networking();
#endif

    ets_uart_printf("Heap Size(%d) bytes\n" , system_get_free_heap_size());
	ets_uart_printf("System init done \r\n");
}
