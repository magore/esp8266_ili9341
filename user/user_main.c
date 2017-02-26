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
#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "user_config.h"
#include "fatfs.h"

#include "time.h"
#include "timer.h"

#include "display/ili9341.h"

#include "network/network.h"

#ifdef WIRECUBE
	#include "cordic/cordic.h"
	#include "wire.h"
	#include "cube_data.h"
#endif

#ifdef EARTH
	#include "cordic/cordic.h"
	#include "wire.h"
	#include "earth_data.h"
#endif

#ifdef ADF4351
	#include "adf4351.h"
#endif

#include "esp8266/system.h"
#include "lib/stringsup.h"

/* 
 * Window layouts    optional
 *
 *         wintop    winearth
 *         winmsg    wincube
 *         winbottom
 */


/* Master Window, Full size of TFT */
window *master;

/* Bottom status window */
window _winbottom;
window *winbottom = &_winbottom;

/* wintop and winearth have the same height
/* Top Left status window */
window _wintop;
window *wintop = &_wintop;

/* Earth Top Right status window */
window _winearth;
window *winearth = &_winearth;

/* winmsg and winearth have the same height
/* Middle Left status window */
window _winmsg;
window *winmsg = &_winmsg;

/* Right Wireframe window */
window _wincube;
window *wincube = &_wincube;

//extern int printf(const char *fmt, ...);
void ets_timer_disarm(ETSTimer *ptimer);
void ets_timer_setfn(ETSTimer *ptimer, ETSTimerFunc *pfunction, void *parg);


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

unsigned long ms_time = 0;

// ==================================================================
// ==================================================================

/// @brief  Clear 1000HZ timer 
/// We loop in case the update of ms_time is not "atomic" - done in a single instruction
/// @return  void.
MEMSPACE
void ms_clear()
{
	while(ms_time)
		ms_time = 0;
}

/// @brief  Read 1000HZ timer 
/// We loop in case the update of ms_time is not "atomic" - done in a single instruction
/// @return  time in milliseconds
MEMSPACE
unsigned long ms_read()
{
	unsigned long ret = 0;
	while(ret != ms_time)
		ret = ms_time;
	return(ret);
}

/**
 @brief 1000HZ timer task
 @return void
*/
void ms_task(void)
{
    ms_time++;
}

/// @brief  Initialize 1000HZ timer task
/// @return  void.
MEMSPACE
void ms_init()
{
	ms_time = 0;
    if(set_timers(ms_task,1) == -1)
        printf("Clock task init failed\n");
}


// FIXME we need to add time zone and timezone processing rules
int ntp_init = 0;
void ntp_setup(void)
{
    tv_t tv;
    tz_t tz;
	time_t sec;
	struct ip_info getinfo;


	// Wait until we have an IP address before we set the time
    if(!network_init)
		return;

	if(ntp_init == 0)
    {
        ip_addr_t *addr = (ip_addr_t *)safecalloc(sizeof(ip_addr_t),1);

		// form pool.ntp.org
		ipaddr_aton("206.108.0.131", addr);
		sntp_setserver(1,addr);
		ipaddr_aton("167.114.204.238", addr);
		sntp_setserver(2,addr);

		// Alternate time setting if the local router does NTP
#if 0
		if(wifi_get_ip_info(0, &getinfo))
		{
			printf("NTP:0 GW: %s\n", ipv4_2str(getinfo.gw.addr));
			printf("NTP:0 IP: %s\n", ipv4_2str(getinfo.ip.addr));
			sntp_setserver(1, & getinfo.gw);
			sntp_setserver(2, & getinfo.ip);
		}
		else
		{
			printf("NTP:0 failed to get GW address\n");
			return;
		}
#endif

        if( sntp_set_timezone(0) )
		{
			printf("NTP: set_timeone OK\n");
			sntp_init();
            os_free(addr);
		    ntp_init = 1;
            printf("NTP:1\n");
		}
		else
		{
			printf("NTP: set_timeone Failed\n");
		}
    }

	if(ntp_init == 1)
	{
		// they hard coded it to +8 hours from GMT
		if( (sec = sntp_get_current_timestamp()) > 10 )
		{
			sntp_stop();
			ntp_init = 2;
		}
	}
	if(ntp_init == 2)
	{
		time_t s;

		tm_t *p;

		printf("NTP:2\n");

		// they return GMT + 8
        // sec = sec - (8UL * 3600UL);

        tv.tv_sec = sec;
		printf("ntp_init: %s\n", asctime(gmtime(&sec)));
		printf("ntp_init: %s\n", ctime_gm(&sec));

        tv.tv_usec = 0;
        tz.tz_minuteswest = 300;
		tz.tz_dsttime = 0;

        settimeofday(&tv, &tz);

        printf("SEC:%ld\n",sec);
        printf("TIME:%s\n", ctime(&sec));
		printf("Zone: %d\n", (int) sntp_get_timezone());
		ntp_init = 3;

		set_dst(tv.tv_sec);

		print_dst_gmt();
		print_dst();

		p = gmtime(&tv.tv_sec);
		mktime(p);
		printf("Localtime: %s\n", asctime(p));
    }
}
 
loop_task()
{
	char buffer[260];

	// NTP state machine
	ntp_setup();
	if(kbhit(0, 1)) /// second argument 1 = only report when an EOL is detectedEOL
	{
		int flag = 0;
		fgets(buffer,255,stdin);
		printf("Command:[%s]\n",buffer);
		if(!flag && user_tests(buffer))
		{
			flag = 1;
		}
#ifdef FATFS_SUPPORT
		if(!flag && fatfs_tests(buffer))
		{
			flag = 1;
		}
#endif
		if(!flag)
		{
			printf("unknown command:[%s]\n", buffer);
		}
	}
}
// Signal strength update interval
int signal_loop = 0;

time_t seconds = 0;

/**
 @brief test task
  Runs corrected cube demo from Sem
  Optionally wireframe Earth viewer
 @return void
*/

extern uint8_t ip_msg[];

int skip = 0;

unsigned long last_time10 = 0;
unsigned long last_time50 = 0;

int loop_cnt = 0;

int inloop = 0;

void loop(void)
{
	extern int connections;
	uint32_t time1,time2;
	uint8_t red, blue,green;
	int ret;
	unsigned long t;
	uint16_t system_adc_read(void);
	time_t sec;
	// getinfo.ip.addr, getinfo.gw.addr, getinfo.netmask.addr
	struct ip_info getinfo;
	char *ptr;

	if(inloop)
	{
		printf("Error: loop() task overrun\n");
		return;
	}

	ret = spi_chip_select_status();
	if(ret != 0xff)
	{
		printf("Error: loop() entered with spi_cs = %d\n",ret);
		spi_end(ret);
		return;
	}


	loop_task();

	ret = spi_chip_select_status();
	if(ret != 0xff)
	{
		printf("Error: loop_task() exit with spi_cs = %d\n",ret);
		spi_end(ret);
		return;
	}

	inloop = 0;

	// Only run every 1mS
	t = ms_read();
	if((t - last_time10) >= 1U)
	{
#ifdef ADF4351
		ADF4351_task();
#endif
		last_time10 = t;
	}

	// run remaining tests every 50ms
	if((t - last_time50) < 50U)
		return;
	last_time50 = t;

	time(&sec);

	// only update text messages once a second
	if(sec != seconds)
	{
		char tmp[32];

		tft_set_textpos(winbottom, 0,0);
		//Tue May 17 18:56:01 2016
		strncpy(tmp,ctime(&sec),31);
		tmp[19] = 0;
		tft_printf(winbottom," %s", tmp);
		tft_cleareol(winbottom);
		tft_set_textpos(winbottom, 0,1);

		// detailed connection state
		//tft_printf(winbottom," %s", ip_msg);

		// IP and disconnected connection state only
		if(wifi_get_ip_info(0, &getinfo))
			tft_printf(winbottom," %s", ipv4_2str(getinfo.ip.addr));
		else
			tft_printf(winbottom," Disconnected");
		tft_cleareol(winbottom);
	}

	count += 1;

#ifdef DEBUG_STATS
	tft_set_textpos(wintop, 0,0);
	tft_set_font(wintop,0);
	tft_font_fixed(wintop);
	tft_printf(wintop,"Iter:% 10ld, %+7.2f\n", count, degree);

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

#endif

	if(sec != seconds)
	{
		seconds=sec;
#ifdef DEBUG_STATS
		tft_set_textpos(wintop, 0,1);
		tft_printf(wintop,"Heap: %d, Conn:%d\n", 
			system_get_free_heap_size(), connections);
	
#ifdef VOLTAGE_TEST
		tft_printf(wintop,"Volt:%2.2f\n", (float)voltage);
#endif
	
		tft_printf(wintop,"CH:%02d, DB:%+02d\n", 
			wifi_get_channel(),
			wifi_station_get_rssi());
#endif
	}
	
#ifdef NETWORK_TEST
	servertest_message(winmsg);
#endif

#ifdef CIRCLE
	rad = dscale; // +/- 90
    tft_drawCircle(wincube, wincube->w/2, wincube->h/2, rad ,wincube->bg);
	// RGB 
#endif

// reset cube to background
#ifdef WIRECUBE
	V.x = degree;
	V.y = degree;
	V.z = degree;
// Cube points were defined with sides of 1.0 
// We want a scale of +/- w/2
	wire_draw(wincube, cube_points, cube_edges, &V, wincube->w/2, wincube->h/2, dscale, wincube->bg);
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
	wire_draw(wincube, cube_points, cube_edges, &V, wincube->w/2, wincube->h/2, dscale, ILI9341_WHITE);
#endif

#ifdef CIRCLE
	// Display bounding circle that changes color around the cube
	if(dscale_inc < 0.0)
	{
		red = 255;
		blue = 0;
		green = 0;
	}
	else
	{
		red = 0;
		blue = 0;
		green = 255;
	}
	rad = dscale; // +/- 90
    tft_drawCircle(wincube, wincube->w/2, wincube->h/2, rad, tft_RGBto565(red,green,blue));
#endif

	ret = spi_chip_select_status();
	if(ret != 0xff)
	{
		printf("Exit: loop() with spi_cs = %d\n",ret);
		spi_end(ret);
		return;
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
#ifdef FATFS_SUPPORT
	fatfs_help();
#endif
#ifdef ADF4351
	adf4351_help();
#endif
	printf("help\n");
    printf("mem\n");
	printf("time\n");
	printf("setdate YYYY MM DD HH:MM:SS\n");
	printf("\n");
}

/// @brief help functions test parser
///
/// - Keywords and arguments are matched against test functions
/// If there are matched the function along with its arguments are called.

/// @param[in] str: User supplied command line 
/// 
/// @return 1 The return code indicates a command matched.
/// @return 0 if no rules matched
MEMSPACE
int user_tests(char *str)
{

    int res;
    int len;
    char *ptr;
    long p1, p2;
	time_t t;
	double freq;

    ptr = skipspaces(str);

    if ((len = token(ptr,"help")) )
    {
        ptr += len;
        user_help();
        return(1);
    }
    else if ((len = token(ptr,"setdate")) )
    {
        ptr += len;
		ptr = skipspaces(ptr);
        setdate_r(ptr);
        return(1);
    }

#ifdef ADF4351
    else if ((len = token(ptr,"adf4351")) )
    {
        ptr += len;
		adf4351_cmd(ptr);
		return(1);
	}
#endif

    else if ((len = token(ptr,"time")) )
    {
		t = time(0);	
		printf("TIME:%s\n", ctime(&t));
        return(1);
	}
    else if ((len = token(ptr,"mem")) )
    {
		PrintRam();
        return(1);
	}
    else if ((len = token(ptr,"timetest")) )
    {
        ptr += len;
		timetests(ptr,0);
        return(1);
	}
	return(0);
}

/**
 test byte order and basic type sizes
*/
test_types()
{
	int i;
// Test byte order
// extensa has LSB to MSB byte order LITTLE_ENDIAN
	union UUU
	{
	  unsigned int  wide;
	  unsigned char byte8[sizeof(unsigned int)];
	};
	volatile union UUU u;

	printf("Byte Order of 0x12345678:\n");
	u.wide = 0x12345678;
	for (i=0; i < sizeof(unsigned int); i++)
		printf("byte[%d] = %02x\n", i, u.byte8[i]);

// Test basic type sizes
    printf("sizeof (double) = %d\n", sizeof (double ) );
    printf("sizeof (float) = %d\n", sizeof (float ) );
    printf("sizeof (long long) = %d\n", sizeof (long long ) );
    printf("sizeof (long) = %d\n", sizeof (long ) );
    printf("sizeof (int) = %d\n", sizeof (int ) );
    printf("sizeof (char) = %d\n", sizeof (char ) );

    printf("long: %ld\n", 123456789L);
    printf("unsigned long: %lu\n", 123456789L);
    printf("long hex: %lx\n", 123456789L);

    printf("int : %d\n", 123456789L);
    printf("unsigned int: %u\n", 123456789L);
    printf("int hex: %lx\n", 123456789L);

    printf("int: %d\n", 12345);
    printf("unsigned int: %u\n", 12345);
    printf("int hex: %x\n", 12345);

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
	int w,h;

	ip_msg[0] = 0;

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

	printf("HSPI init...\n");
	hspi_init(1,0);

	printf("Timers init...\n");
	init_timers();

	// 1000HZ timer
	ms_init();


	test_types();

	// Functions manage chip selects
#ifdef MMC_CS
	chip_select_init(MMC_CS);
#endif
#ifdef ADF4351_CS
	chip_select_init(ADF4351_CS);
#endif
#ifdef ILI9341_CS
	chip_select_init(ILI9341_CS);
#endif
	// Functions manage user defined address pins
	chip_addr_init();

	// Initialize TFT
	master = tft_init();
	ID = tft_ID;
	// Set master rotation
	tft_setRotation(1);

#if ILI9341_DEBUG & 1
	printf("\nDisplay ID=%08lx\n",ID);
#endif

#ifdef ADF4351
	ADF4351_Init();
	printf("ADF4351 init done\n");
#endif

// Make sure all other GPIO pins are initialized BEFORE SD card
#ifdef FATFS_SUPPORT
	printf("SD Card init...\n");
	mmc_init(1);
#endif

// Message window setup
#ifdef EARTH
	w = master->w * 7 / 10;
#else
	w = master->w;
#endif
	// TOP
#ifdef DEBUG_STATS
	tft_window_init(wintop,0,0, w, font_H(0)*4);
	tft_setTextColor(wintop, ILI9341_WHITE, ILI9341_NAVY);
#else
	tft_window_init(wintop,0,0, w, font_H(2)*2);
	tft_setTextColor(wintop, ILI9341_WHITE, tft_RGBto565(0,64,255));
#endif
	tft_set_font(wintop,0);
	tft_font_var(wintop);
    tft_fillWin(wintop, wintop->bg);
	tft_set_textpos(wintop, 0,0);

#ifdef EARTH
	tft_window_init(winearth,w,0, master->w - w + 1, wintop->h);
	tft_setTextColor(winearth, ILI9341_WHITE, ILI9341_NAVY);
    tft_fillWin(winearth, winearth->bg);
#endif

	// BOTOM
	// TIME,DATE
	tft_window_init(winbottom, 0, master->h - 1 - font_H(2)*2, 
		master->w, font_H(2)*2);
    tft_set_font(winbottom,2);
    tft_font_var(winbottom);
	tft_setTextColor(winbottom, 0, tft_RGBto565(0,255,0));
	tft_fillWin(winbottom, winbottom->bg);
    tft_set_textpos(winbottom, 0,0);

// Message window setup
#ifdef WIRECUBE
	w = master->w * 7 / 10;
#else
	w = master->w;
#endif

	// MSG
    tft_window_init(winmsg,0,wintop->h,
            w, master->h - (wintop->h + winbottom->h));

    tft_setTextColor(winmsg, ILI9341_WHITE,ILI9341_BLUE);
    tft_fillWin(winmsg, winmsg->bg);
    // write some text
    tft_set_font(winmsg,2);
    tft_font_var(winmsg);
    tft_set_textpos(winmsg, 0,0);

// CUBE setup
#ifdef WIRECUBE
	/* Setup cube/wireframe demo window */
	/* This is to the right of the winmsg window and the same height */
	tft_window_init(wincube, winmsg->w, wintop->h, master->w - winmsg->w, winmsg->h);
	tft_setTextColor(wincube, ILI9341_WHITE,ILI9341_BLUE);
    tft_fillWin(wincube, wincube->bg);
#endif

#ifdef DEBUG_STATS
	// Display ID
	tft_setTextColor(winmsg, ILI9341_RED,winmsg->bg);
	tft_printf(winmsg, "DISP ID: %04lx\n", ID);
	tft_setTextColor(winmsg, ILI9341_WHITE,winmsg->bg);
#endif


// Cube points were defined with sides of 1.0 
// We want a scale of +/- w/2
#ifdef WIRECUBE
	if(wincube->w < wincube->h) 
		dscale_max = wincube->w/2;
	else
		dscale_max = wincube->h/2;

	dscale = dscale_max;
	dscale_inc = dscale_max / 100;
#endif

#if ILI9341_DEBUG & 1
	printf("Test Display Read\n");
	read_tests(winmsg);
#endif

// Draw Wireframe earth in message area
#ifdef EARTH
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
	if ( espconn_tcp_set_max_con(MAX_CONNECTIONS+2) )
		printf("espconn_tcp_set_max_con(%d) - failed!\n", MAX_CONNECTIONS+2);
	else
		printf("espconn_tcp_set_max_con(%d) - success!\n", MAX_CONNECTIONS+2);

#ifdef NETWORK_TEST
	printf("Setup Network TFT Display Client\n");
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
