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



#ifdef NETWORK_TEST

#include "user_config.h"
#include "network.h"

#ifndef SSID
	//#define SSID "your wifi SSID"
	//#define SSID_PASS "your wifi password"

	/* I define my private SSID and SSID_PASS in ssid.h.
       For security reasons my ssid.h is not part of this github project 8-)
	*/
	#include "../../ssid.h"
#endif

uint8_t network_msg[256];
static struct espconn *TCP_Server;

#ifndef TCP_PORT
//#define TCP_PORT 31416
#error Please define TCP_PORT
#endif

#define SERVER_TIMEOUT 5
#define MAX_CONNS 5

static struct station_config StationConfig;
static struct ip_info info;
static uint8_t macaddr[6];
static uint8_t received = 0;

/**
 @brief Network receive task 
 @return void
*/

MEMSPACE
void poll_network_message(window *win)
{
	if(received)
	{
		//ets_uart_printf("%s\n", network_msg);
		//tft_fillWin(win, win->bg);
		//tft_setpos(win,0,0);
		//tft_set_font(win,5);
		//tft_font_fixed(win);
		if(win->x > 0)
			tft_printf(win,"\n");
		tft_printf(win,"%s", network_msg);
		received = 0;
	}
}

/**
 @brief Network receive task 
 @return void
*/
MEMSPACE 
void my_receive(void *arg, char *pdata, unsigned short len)
{
	if(len < (sizeof(network_msg) - 1)) 
	{
		os_memcpy(network_msg,pdata,len);
		network_msg[len] = 0;
		received = len;
	}
}


/**
  @brief WIFI Event Callback
  This callback status code borrowed from David Ogilvy(MetalPhreak)
 @return void
*/
MEMSPACE 
void wifi_event_cb( System_Event_t *event_p)
{
    ets_uart_printf("WiFi Event: %02x\n", event_p->event);
    switch( event_p->event) 
	{
        case EVENT_STAMODE_CONNECTED :
            ets_uart_printf( "Connect to [%s] ssid, channel[%d] \n " ,
                    event_p->event_info.connected.ssid ,
                    event_p->event_info.connected.channel);
            break;
        case EVENT_STAMODE_DISCONNECTED :
            ets_uart_printf( "Disconnect from ssid:[%s], Reason:%02x\n " ,
                    event_p->event_info.disconnected.ssid ,
                    event_p->event_info.disconnected.reason);
// Reconnect ???
// wifi_station_connect();
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE :
            ets_uart_printf( "New AuthMode: %02x -> %02x\n " ,
                    event_p->event_info.auth_change.old_mode ,
                    event_p->event_info.auth_change.new_mode);
            break;
        case EVENT_STAMODE_GOT_IP :
            ets_uart_printf("IP:[" IPSTR "], Mask:[" IPSTR "], GW:[" IPSTR "]\n",
                    IP2STR( & event_p->event_info.got_ip.ip) ,
                    IP2STR( & event_p->event_info.got_ip.mask) ,
                    IP2STR( & event_p->event_info.got_ip.gw) );
            ets_uart_printf( "MAC:[" MACSTR "]\n" ,
                    MAC2STR( event_p->event_info.sta_connected.mac) );
            break;
        case EVENT_SOFTAPMODE_STACONNECTED :
            ets_uart_printf( "STACONNECTED MAC:[" MACSTR "], AID:%02x\n " ,
                    MAC2STR( event_p->event_info.sta_connected.mac) ,
                    event_p->event_info. sta_connected.aid);
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED :
            ets_uart_printf( "STADISCONNECTED MAC:[%s], AID:%02x\n " ,
                    MAC2STR( event_p->event_info.sta_connected.mac) ,
                    event_p->event_info. sta_connected.aid);
            break;
        }
}


/**
  @brief Code fragments
    if(!wifi_station_dhcpc_stop()) 
		ets_uart_printf( "ERROR wifi_station_dhcpc_stop() \n ");

    if(!wifi_set_ip_info( STATION_IF , &info)) 
		ets_uart_printf( "ERROR wifi_set_ip_info()\n");

    if(!wifi_station_connect()) 
		ets_uart_printf( "ERROR wifi_station_connect()\n");

    if(!wifi_station_set_auto_connect( TRUE) ) 
		ets_uart_printf("ERROR wifi_station_set_auto_connect(1)\n");
    IP4_ADDR( & info.ip , 192 , 168 , 200 , 123);
    IP4_ADDR( & info.netmask , 255 , 255 , 255 , 0);
    IP4_ADDR( & info.gw, 192 , 168 , 200 , 1);
    if(!wifi_set_ip_info( STATION_IF , &info) ) 
		ets_uart_printf( "ERROR wifi_set_ip_info() \n ");
    if(!wifi_station_set_auto_connect( TRUE) ) 
		ets_uart_printf( "ERROR wifi_station_set_auto_connect(1) \n ");
*/

/**
  @brief Setup Wifi Network
  Credits: ideas borrowed from David Ogilvy(MetalPhreak)
 @return void
*/
MEMSPACE 
void setup_networking(void)
{

	os_memset(&StationConfig, 0, sizeof(StationConfig));
	os_memset(&info, 0, sizeof(info));
	const char *ssid = SSID;
	const char *password = SSID_PASS;

	os_memset(network_msg,0,sizeof(network_msg));

    if(wifi_get_opmode() != STATION_MODE)
    {
        ets_uart_printf("ESP8266 not in STATION mode, restarting in STATION mode...\r\n");
		if(!wifi_station_disconnect()) 
			ets_uart_printf( "ERROR wifi_station_disconnect()\n");
		if(!wifi_set_opmode( STATION_MODE)) 
			ets_uart_printf( "ERROR wifi_set_opmode(STATION_MODE)\n");
		if(!wifi_station_set_reconnect_policy(FALSE))
			ets_uart_printf( "ERROR wifi_station_set_reconnect_policy(FALSE)\n");
		if(!wifi_station_set_auto_connect(FALSE) ) 
			ets_uart_printf( "ERROR wifi_station_set_auto_connect(FALSE)\n");
		if(wifi_get_phy_mode() != PHY_MODE_11N)
			wifi_set_phy_mode(PHY_MODE_11N);
    }

    if(wifi_get_opmode() == STATION_MODE)
    {
        ets_uart_printf("ESP8266 in STATION mode...\r\n");
        wifi_station_get_config(&StationConfig);
        os_sprintf(StationConfig.ssid, "%s", ssid);
        os_sprintf(StationConfig.password, "%s", password);
        if(!wifi_station_set_config(&StationConfig))
			ets_uart_printf( "ERROR wifi_set_config()\n");
        if(!wifi_station_get_config(&StationConfig))
			ets_uart_printf( "ERROR wifi_station_get_config()\n");
        if(!wifi_get_macaddr(STATION_IF, macaddr))
			ets_uart_printf( "ERROR wifi_get_macaddr()\n");
        ets_uart_printf("wifi_get_opmode: %d, SSID:[%s], PASSWORD:[%s]\n",
            wifi_get_opmode(),
            StationConfig.ssid,
            "**********"
			/* StationConfig.password */
			);
    }
	else
	{
        ets_uart_printf("ESP8266 not in STATION mode\n");
	}

	// Wifi Connect
    if(!wifi_station_set_reconnect_policy(TRUE))
		ets_uart_printf( "ERROR wifi_station_set_reconnect_policy(TRUE)\n");

	if(!wifi_station_connect())
		ets_uart_printf( "ERROR wifi_station_connect()\n");

    if(!wifi_station_dhcpc_start()) 
		ets_uart_printf("ERROR wifi_station_dhcpc_start()\n");

	// Monitor
    wifi_set_event_handler_cb( wifi_event_cb);

	// TCP server
	TCP_Server =(struct espconn *)os_zalloc(sizeof(struct espconn));
	TCP_Server->type = ESPCONN_TCP;
	TCP_Server->state = ESPCONN_NONE;
	TCP_Server->proto.tcp =(esp_tcp *)os_zalloc(sizeof(esp_tcp));
	TCP_Server->proto.tcp->local_port = TCP_PORT;
	// Receive Callback
	espconn_regist_recvcb(TCP_Server, my_receive);
	espconn_accept(TCP_Server);
	espconn_regist_time(TCP_Server, SERVER_TIMEOUT, 0);
	ets_uart_printf( "Networking setup done, waiting on port:%d\n", TCP_PORT);
}

#endif	// NETWORK_TEST

