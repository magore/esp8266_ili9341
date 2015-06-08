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

extern window *tft;
window twindow;
window *twin = &twindow;

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
// Rotation angle
LOCAL double degree = 0.0;
// Scale factor
LOCAL double scale = 10.0;
// Rotation increment
LOCAL double deg_inc = 4;
// Scale increment
LOCAL double scale_inc = 1.0;

LOCAL long count = 0;
LOCAL int rad;
LOCAL point V;
LOCAL point S;

LOCAL adc_sum = 0;
LOCAL adc_count = 0;

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
LOCAL uint32_t seconds;

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
	window *win;
	uint16_t xcenter;
	uint16_t ycenter;
	uint8_t red, blue,green;

	uint16 system_adc_read(void);


	xcenter = tft->w - 50;
	ycenter = 50;

#ifdef WIRECUBE
	V.x = degree;
	V.y = degree;
	V.z = degree;
	wire_draw(tft, cube_points, cube_edges, &V, xcenter, ycenter, scale, 0);
	//wire_draw(tft, cube_points, cube_edges, &V, xcenter, ycenter, scale/2, 0);
#endif

#ifdef EARTH
	scale = 10;
	// erase earth
	V.x = -90;
	V.y = -90;
	V.z = -90;
	wire_draw(tft, earth_data, NULL, &V, xcenter, ycenter, scale, 0);
#endif

#ifdef CIRCLE
	rad = scale; // +/- 90
    tft_drawCircle(tft, xcenter, ycenter, rad ,0);
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
    tft_drawCircle(tft, xcenter, ycenter, scale, tft_color565(red,green,blue));
#endif

    degree += deg_inc;
    scale += scale_inc;

	if(degree <= -360)
		deg_inc = 4;
	if(degree >= 360)
		deg_inc = -4;

    if(scale < 10.0)
        scale_inc = 1;
    if(scale > 25.0)
        scale_inc = -1;

#ifdef EARTH
	scale = 25.0;

	V.x = -90;
	V.y = -90;
	V.z = -90;
	// draw earth
	//time1 = system_get_time();
	wire_draw(tft, earth_data, NULL, &V, xcenter, ycenter, scale, 0xffff);
	//time2 = system_get_time();
#endif

#ifdef WIRECUBE
	V.x = degree;
	V.y = degree;
	V.z = degree;
	//time1 = system_get_time();
	wire_draw(tft, cube_points, cube_edges, &V, xcenter, ycenter, scale, 0xffff);
	//wire_draw(tft, cube_points, cube_edges, &V, xcenter, ycenter, scale/2, 0xffff);
	//time2 = system_get_time();
#endif

	// ets_uart_printf("Degree: %d \r\n",(int)degree);
	count += 1;
	tft_setpos(tft,xpos,ypos);
	tft_set_font(tft,0);
	tft_printf(tft,"c:% 9ld, %+7.2f\n", count, degree);

// Get system voltage 33 = 3.3 volts
	adc_sum += system_adc_read();

	if(++adc_count == 10)
	{
		float voltage = ((float) adc_sum / 100.0); 
		tft_printf(tft,"Voltage: %2.2f\n", voltage);
		adc_count = 0;
		adc_sum = 0;
	}
	
#ifdef NETWORK_TEST
	poll_network_message(twin);
#endif
}


/**
 @brief Initialize user task
 @return void
*/
MEMSPACE 
void user_init(void)
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

	/* Setup main window */
	tft_setRotation(1);
	tft_setpos(tft, 0,0);
	tft_set_font(tft,3);
	tft_font_var(tft);
	tft->wrap = 1;

	tft_printf(tft, "DISP ID: %04lx\n", ID);
	ets_uart_printf("\r\nDisplay ID=%08lx\r\n",ID);

	tft_font_fixed(tft);


/* Setup second window for testing*/
	tft_window_init(twin,0,tft->h/2, tft->w, tft->h/2);
	tft_setTextColor(twin, 0xffff,ILI9341_BLUE);
    tft_fillWin(twin, twin->bg);
	tft_set_font(twin,3);
	tft_font_var(twin);
	twin->wrap = 1;

	tft_setpos(twin, 0,3);
	tft_printf(twin, "Test1\nTest2\nTest3");
#ifdef WIRECUBE
	V.x = 45;
	V.y = 45;
	V.z = 45;
	wire_draw(twin, cube_points, cube_edges, &V, 150, 50, 20, ILI9341_RED);
#endif

	// Save last display offset for user task
	xpos = tft->x;
	ypos = tft->y;
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


