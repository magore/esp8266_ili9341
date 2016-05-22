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
window _wincube;
window *wincube = &_wincube;

/* Bottom Left status window */
window _winmsg;
window *winmsg = &_winmsg;

#if defined(EARTH)
/* Bootom Right Wireframe window */
window _winearth;
window *winearth = &_winearth;
#endif

//extern int printf(const char *fmt, ...);
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);



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
        printf("NTP:1\n");
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
		printf("NTP:2\n");

		// they return GMT + 8
        sec = sec - (8UL * 3600UL);

		// we are GMT - 4
        sec = sec - (4UL * 3600UL);

        tv.tv_sec = sec;
        tv.tv_usec = 0;
        tz.tz_minuteswest = 0;

        settimeofday(&tv, &tz);

        printf("SEC:%ld\n",sec);
        printf("TIME:%s\n", ctime(&sec));
		ntp_init = 3;
    }
}
 
// segnal strength update interval
int signal_loop = 0;

time_t seconds = 0;

/**
 @brief test task
  Runs corrected cube demo from Sem
  Optionally wireframe Earh viewer
 @return void
*/

int ip_msg_state_last = -1;
extern int ip_msg_state;
extern uint8_t ip_msg[];
char ip_msg_save[256];

void loop(void)
{
	extern int connections;
	uint32_t time1,time2;
	uint8_t red, blue,green;
	long timer = 0;
	uint16 system_adc_read(void);
	time_t sec;
	char *ptr;
	char buffer[260];

	// get current time

	time(&sec);

	if(sec != seconds)
	{
		tft_set_font(winstats,1);
		tft_font_var(winstats);
		tft_setpos(winstats, 0,0);
		ptr = ctime(&sec);
		//Tue May 17 18:56:01 2016
		ptr[10] = 0;
		ptr[19] = 0;
		tft_printf(winstats,"%s %s\n", ptr, ptr+20);
		tft_printf(winstats,"%s\n", ptr+11);

	}

	tft_set_font(winstats,0);
	tft_font_fixed(winstats);
	tft_setpos(winstats, xpos,ypos);

	count += 1;

#ifdef DEBUG_STATS
	tft_printf(winstats,"Iter:% 10ld, %+7.2f\n", count, degree);
#endif

#ifdef VOLTAGE_TEST
	// FIXME voltage not correct 
	//       make sure the pin function is assigned

	// Get system voltage 33 = 3.3 volts
	adc_sum += system_adc_read();

	// FIXME atomic access
	if(++adc_count == 10)
	{
		voltage = ((double) adc_sum / 100.0); 
		adc_count = 0;
		adc_sum = 0;
	}
#endif

	if(sec != seconds)
	{
		seconds=sec;
	
#ifdef DEBUG_STATS
		tft_printf(winstats,"Heap: %d, Conn:%d\n", 
			system_get_free_heap_size(), connections);
	
#ifdef VOLTAGE_TEST
		tft_printf(winstats,"Volt:%2.2f\n", (float)voltage);
#endif
	
		tft_printf(winstats,"%s\n", ip_msg);
	
		tft_printf(winstats,"CH:%02d, DB:-%02d\n", 
			wifi_get_channel(),
			wifi_station_get_rssi());
#else
		if(ip_msg[0] && strcmp(ip_msg,ip_msg_save) != 0)
		{
			strcpy(ip_msg_save,ip_msg);
			tft_printf(winmsg,"%s\n", ip_msg);
			printf("ip_msg:[%s]\n", ip_msg);
		}
#endif

	}
	
#ifdef NETWORK_TEST
	servertest_message(winmsg);
#endif

#ifdef WIRECUBE
	V.x = degree;
	V.y = degree;
	V.z = degree;
// Cube points were defined with sides of 1.0 
// We want a scale of +/- w/2
	wire_draw(wincube, cube_points, cube_edges, &V, wincube->w/2, wincube->h/2, dscale, wincube->bg);
#endif

#ifdef CIRCLE
	rad = dscale; // +/- 90
    tft_drawCircle(wincube, wincube->w/2, wincube->h/2, rad ,0);
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
    tft_drawCircle(wincube, wincube->w/2, wincube->h/2, dscale, tft_color565(red,green,blue));
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
	wire_draw(wincube, cube_points, cube_edges, &V, wincube->w/2, wincube->h/2, dscale, wincube->fg);
	//wire_draw(wincube, cube_points, cube_edges, &V, wincube->w/2, wincube->h/2, dscale, wincude->fg);
	//time2 = system_get_time();
#endif




	// NTP state machine
	ntp_setup();
// Buffered get line uses interrupts and queues
	if(uart0_gets(buffer,255))
	{
		int flag = 0;
		printf("Command:%s\n",buffer);
		if(!flag && user_tests(buffer))
		{
			flag = 1;
		}
#ifdef FATFS_TEST
		if(!flag && fatfs_tests(buffer))
		{
			flag = 1;
		}
#endif
		if(!flag)
		{
			printf("unknow command: %s\n", buffer);
		}
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
		// printf("x:%d,y:%d,c:%04x\n", x,y,color);
		printf("x:%d,y:%d,c:%04x\n", x,y,buffer[x]);
	}
	 printf("\n");
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
    tft_set_font(win,0);
    //tft_printf(win, "%x,%x", xxxp[0],xxxp[1]);
    xpos = win->xpos;
    ypos = win->ypos;
    for(i=0;i<8;++i)
    {
        tft_setpos(win, i*16,ypos);
        tft_printf(win, "%02x", read_flash8((uint32_t) &xxx+i));
    }
    tft_printf(win, "\n");
    tft_printf(win, "%08x, %08x", read_flash32((uint8_t *)&xxx), read_flash16((uint8_t *)&xxx));
}
#endif


void user_help()
{
	printf("help\n"
	"setdate YYYY MM DD HH:MM:SS\n"
	"time\n"
	);
#ifdef FATFS_TEST
	fatfs_help();
#endif
}

/// @brief help functions test parser
///
/// - Keywords and arguments are matched against test functions
/// If ther are matched the function along with its argements are called.

/// @param[in] str: User supplied command line 
/// 
/// @return 1 The ruturn code indicates a command matched.
/// @return 0 if no rules matched
MEMSPACE
int user_tests(char *str)
{

    int res;
    int len;
    char *ptr;
    long p1, p2;
	time_t t;

    ptr = skipspaces(str);

    if ((len = token(ptr,"help")) )
    {
        ptr += len;
        user_help();
        return(1);
    }
    if ((len = token(ptr,"setdate")) )
    {
        ptr += len;
		ptr = skipspaces(str);
        setdate_r(ptr);
        return(1);
    }
    if ((len = token(ptr,"time")) )
    {
		t = time(0);	
		printf("TIME:%s\n", ctime(&t));
        return(1);
	}
	return(0);
}



/**
 @brief main() Initialize user task
 @return void
*/
MEMSPACE 
void setup(void)
{
	int i;
    char time[20];
	int ret;
	uint16_t *ptr;
	uint32_t time1,time2;
	uint32_t ID;
	extern uint16_t tft_ID;
	double ang;
	extern web_init();

	ip_msg[0] = 0;
	ip_msg_save[0] = 0;

// CPU
// 160MHZ
   REG_SET_BIT(0x3ff00014, BIT(0));
// 80MHZ
//   REG_CLR_BIT(0x3ff00014, BIT(0));

	os_delay_us(200000L);	// Power Up dalay - lets power supplies and devices settle

	// Configure the UART
	uart_init(BIT_RATE_115200,BIT_RATE_115200);

	printf("\n");
	printf("System init...\n");

    PrintRam();

	if ( espconn_tcp_set_max_con(MAX_CONNECTIONS+2) )
		printf("espconn_tcp_set_max_con(%d) - failed!\n", MAX_CONNECTIONS+2);
	else
		printf("espconn_tcp_set_max_con(%d) - success!\n", MAX_CONNECTIONS+2);

	printf("HSPI init...\n");
	hspi_init(1,0);

	printf("Timers init...\n");
	init_timers();

#ifdef FATFS_TEST
	printf("SD Card init...\n");
	mmc_init(1);
#endif

	printf("Display Init\n");

	// Initialize TFT
	master = tft_init();
	ID = tft_ID;
	// Set master rotation
	tft_setRotation(1);

#if ILI9341_DEBUG & 1
	printf("\nDisplay ID=%08lx\n",ID);
#endif

	/* Setup main status window */
#ifdef DEBUG_STATS
	tft_window_init(winstats,0,0, 
		master->w * 7 / 10, master->h/2);
#else
	tft_window_init(winstats,0,0, 
		master->w * 7 / 10, master->h/4);
#endif

	tft_setTextColor(winstats, ILI9341_WHITE,0);
    tft_fillWin(winstats, winstats->bg);


	tft_set_font(winstats,1);
	tft_font_var(winstats);
	tft_setpos(winstats, 0,0);
	// TIME,DATE
	tft_printf(winstats, "\n\n");
	// Save last display offset for additiona status messages
	tft_font_fixed(winstats);
	xpos = winstats->xpos;
	ypos = winstats->ypos;

	/* Setup cube/wireframe demo window */
	/* This is to the right of the winstats window and the same height */
    tft_window_init(wincube,winstats->w,0, 
		master->w - winstats->w, winstats->h);
	tft_setTextColor(wincube, ILI9341_WHITE,0);
    tft_fillWin(wincube, wincube->bg);

// Cube points were defined with sides of 1.0 
// We want a scale of +/- w/2
	if(wincube->w < wincube->h) 
		dscale_max = wincube->w/2;
	else
		dscale_max = wincube->h/2;

	dscale = dscale_max;
	dscale_inc = dscale_max / 100;

#if defined(EARTH)
	tft_window_init(winmsg,0, winstats->h, 
		master->w * 7 / 10, master->h-winstats->h-1);
#else
	tft_window_init(winmsg,0, winstats->h-1, 
		master->w-1, master->h-winstats->h-1);
#endif

	tft_setTextColor(winmsg, ILI9341_WHITE,ILI9341_NAVY);
    tft_fillWin(winmsg, winmsg->bg);
	// write some text
	tft_set_font(winmsg,1);
	tft_font_var(winmsg);
	tft_setpos(winmsg, 0,0);

	tft_setTextColor(winmsg, ILI9341_RED,ILI9341_NAVY);
	tft_printf(winmsg, "DISP ID: %04lx\n", ID);
	tft_setTextColor(winmsg, ILI9341_WHITE,ILI9341_NAVY);

#if ILI9341_DEBUG & 1
	printf("Test Display Read\n");
	read_tests(winmsg);
#endif

#ifdef EARTH
	/* Test demo area window */
	tft_window_init(winearth,winmsg->w,winstats->h, 
		master->w-winmsg->w,master->h-winstats->h);
	tft_setTextColor(winearth, ILI9341_WHITE,ILI9341_NAVY);
    tft_fillWin(winearth, winearth->bg);
	tft_set_font(winearth,1);
	tft_font_var(winearth);
	printf("Draw Earth\n");

// Earth points were defined with radius of 0.5, diameter of 1.0
// We want a scale of +/- w/2
	double tscale_max;
	if(winearth->w < winearth->h) 
		tscale_max = winearth->w;
	else
		tscale_max = winearth->h;
	V.x = -90;
	V.y = -90;
	V.z = -90;
	// draw earth
// Earth points were defined over with a scale of -0.5/+0.5 scale - so scale must be 1 or less
	wire_draw(winearth, earth_data, NULL, &V, winearth->w/2, winearth->h/2, tscale_max, winearth->fg);
#endif

	wdt_reset();

	printf("Setup Tasks\n");

#ifdef TELNET_SERIAL
	printf("Setup Network Serial Bridge\n");
	bridge_task_init(23);
#endif

	setup_networking();

#ifdef NETWORK_TEST
	printf("Setup Network TFT DIsplay Client\n");
	servertest_setup(TCP_PORT);
#endif

#ifdef WEBSERVER
	printf("Setup Network WEB SERVER\n");
	web_init(80);
#endif

    PrintRam();

	system_set_os_print(0);

}


// ===========================================================
// We are using the Arduino Yield code for our main task now
#ifndef YIELD_TASK

/* user/user_main.c */
MEMSPACE LOCAL void init_done_cb ( void );
MEMSPACE LOCAL void HighTask ( os_event_t *events );
MEMSPACE LOCAL void NormalTask ( os_event_t *events );
MEMSPACE LOCAL void IdleTask ( os_event_t *events );
MEMSPACE LOCAL void UserTask ( os_event_t *events );
MEMSPACE LOCAL void sendMsgToUserTask ( void *arg );
LOCAL void loop( void );
MEMSPACE void user_init ( void );

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


// ===========================================================

// ===========================================================
// Delay timer in milliseconds
#define USER_TASK_DELAY_TIMER     	30
#define RUN_TASK 			0
#define UserTaskPrio        USER_TASK_PRIO_0
#define UserTaskQueueLen  	4


/**
 @brief Callback for system_init_done_cb()
  Sends message to run task
 @return void
*/
MEMSPACE 
LOCAL void init_done_cb( void)
{
	printf("System init done \r\n");
	// disable os_printf at this time
	system_set_os_print(0);
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
			loop(); 
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


void user_init(void)
{

	setup();

	// Set up a timer to send the message to User Task
	os_timer_disarm(&UserTimerHandler);
	os_timer_setfn(&UserTimerHandler,(os_timer_func_t *)sendMsgToUserTask,(void *)0);
	os_timer_arm(&UserTimerHandler, USER_TASK_DELAY_TIMER, 1);
	// Setup the user task
	system_os_task(UserTask, UserTaskPrio, UserTaskQueue, UserTaskQueueLen);

#if 0
	// Misc Task init - testing only
	system_os_task(HighTask, HighTaskPrio, HighTaskQueue, HighTaskQueueLen);
	system_os_task(NormalTask, NormalTaskPrio, NormalTaskQueue, NormalTaskQueueLen);
	system_os_task(IdleTask, IdleTaskPrio, IdleTaskQueue, IdleTaskQueueLen);
	system_os_post(IdleTaskPrio, 0, 0);
#endif
	printf("User Init Done!\n");


    system_init_done_cb(init_done_cb);
}
#endif
