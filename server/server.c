/**
 @file network.c

 @brief Network test client
  This code receives a message and displays iit.

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

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "display/ili9341.h"
#include "network/network.h"
#include "server/server.h"

static struct espconn *TCP_Server;
uint8_t network_msg[256];

#ifndef TCP_PORT
	//#define TCP_PORT 31416
	#error Please define TCP_PORT
#endif

#define SERVER_TIMEOUT 1
#define MAX_CONNS 5


static received = 0;
/**
 @brief Network receive task 
 @param[in] *win: window pointer
 @return void
*/

MEMSPACE
void servertest_message(window *win)
{
	if(received)
	{
		//printf("%s\n", network_msg);
		//tft_fillWin(win, win->bg);
		//tft_setpos(win,0,0);
		//tft_set_font(win,5);
		//tft_font_fixed(win);
		if(win->xpos > 0)
			tft_printf(win,"\n");
		tft_printf(win,"%s", network_msg);
		received = 0;
	}
}


/**
 @brief Network receive task 
 @param[in] *arg: network callback arg
 @param[in] *pdata: data buffer
 @param[in] len: buffer length
 @return void
*/
MEMSPACE 
void servertest_receive(void *arg, char *pdata, unsigned short len)
{
	if(len < (sizeof(network_msg) - 1)) 
	{
		os_memcpy(network_msg,pdata,len);
		network_msg[len] = 0;
		received = len;
		espconn_delete(TCP_Server);
	}
}


/**
  @brief Setup Server Task
  @param[in] port: TCP port for service
  Credits: ideas borrowed from David Ogilvy(MetalPhreak)
 @return void
*/
MEMSPACE 
void servertest_setup(int port)
{
	received = 0;
    os_memset(network_msg,0,sizeof(network_msg));

	// TCP server
	TCP_Server =(struct espconn *)safecalloc(sizeof(struct espconn),1);
	TCP_Server->type = ESPCONN_TCP;
	TCP_Server->state = ESPCONN_NONE;
	TCP_Server->proto.tcp =(esp_tcp *)safecalloc(sizeof(esp_tcp),1);
	TCP_Server->proto.tcp->local_port = port;
	// Receive Callback
	espconn_regist_recvcb(TCP_Server, servertest_receive);
	espconn_accept(TCP_Server);
	espconn_regist_time(TCP_Server, SERVER_TIMEOUT, 0);
	printf( "Server test setup done, waiting on port:%d\n", TCP_PORT);
}
