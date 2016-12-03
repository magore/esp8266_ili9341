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

#ifdef NETWORK_TEST
	#include "server/server.h"
#endif

#ifndef SSID
	//#define SSID "your wifi SSID"
	//#define SSID_PASS "your wifi password"

	/* I define my private SSID and SSID_PASS in ssid.h.
       For security reasons my ssid.h is not part of this github project 8-)
	*/
	#include "../../ssid.h"
#endif


int network_init = 0;
uint8_t ip_msg[64];
int ip_msg_state;

static struct station_config StationConfig;
static struct ip_info info;
static uint8_t macaddr[6];
static uint8_t received = 0;

/**
  @brief Convert IP address to string
  @param[in] ip: IPV4 address
  @return char * pointer to string result
*/
static char _ipv4_2str[32];
MEMSPACE 
char *ipv4_2str(uint32_t ip)
{
	uint8_t *str = (uint8_t *) &ip;
	snprintf(_ipv4_2str,sizeof(_ipv4_2str)-1,"%d.%d.%d.%d", str[0],str[1],str[2],str[3]);
	return(_ipv4_2str);
}

/**
  @brief WIFI Event Callback
  @param[in] *event_p: Wifi event structure
  This callback status code borrowed from David Ogilvy(MetalPhreak)
 @return void
*/
MEMSPACE 
void wifi_event_cb( System_Event_t *event_p)
{
    printf("WiFi Event: %02x\n", event_p->event);
	ip_msg_state = event_p->event;

    switch( event_p->event) 
	{
        case EVENT_STAMODE_CONNECTED :
            printf( "Connect to [%s] ssid, channel[%d] \n " ,
                    event_p->event_info.connected.ssid ,
                    event_p->event_info.connected.channel);
			snprintf(ip_msg, sizeof(ip_msg), "Connected");
            break;
        case EVENT_STAMODE_DISCONNECTED :
            printf( "Disconnect from ssid:[%s], Reason:%02x\n " ,
                    event_p->event_info.disconnected.ssid ,
                    event_p->event_info.disconnected.reason);
// Reconnect ???
// wifi_station_connect();
			snprintf(ip_msg, sizeof(ip_msg), "Disconnected");
            break;
        case EVENT_STAMODE_AUTHMODE_CHANGE :
            printf( "New AuthMode: %02x -> %02x\n " ,
                    event_p->event_info.auth_change.old_mode ,
                    event_p->event_info.auth_change.new_mode);
			snprintf(ip_msg, sizeof(ip_msg), "STA Auth change");
            break;
        case EVENT_STAMODE_GOT_IP :
            printf("IP:[" IPSTR "], Mask:[" IPSTR "], GW:[" IPSTR "]\n",
                    IP2STR( & event_p->event_info.got_ip.ip) ,
                    IP2STR( & event_p->event_info.got_ip.mask) ,
                    IP2STR( & event_p->event_info.got_ip.gw) );
            printf( "MAC:[" MACSTR "]\n" ,
                    MAC2STR( event_p->event_info.sta_connected.mac) );

			snprintf(ip_msg, sizeof(ip_msg), "IP:" IPSTR , 
					IP2STR( & event_p->event_info.got_ip.ip));

			network_init = 1;

            break;
        case EVENT_SOFTAPMODE_STACONNECTED :
            printf( "STACONNECTED MAC:[" MACSTR "], AID:%02x\n " ,
                    MAC2STR( event_p->event_info.sta_connected.mac) ,
                    event_p->event_info. sta_connected.aid);
			snprintf(ip_msg, sizeof(ip_msg), "STA Connected");
            break;
        case EVENT_SOFTAPMODE_STADISCONNECTED :
            printf( "STADISCONNECTED MAC:[%s], AID:%02x\n " ,
                    MAC2STR( event_p->event_info.sta_connected.mac) ,
                    event_p->event_info. sta_connected.aid);
			snprintf(ip_msg, sizeof(ip_msg), "STA Disconnected");
            break;
        }
}


/**
  @brief Code fragments
    if(!wifi_station_dhcpc_stop()) 
		printf( "ERROR wifi_station_dhcpc_stop() \n ");

    if(!wifi_set_ip_info( STATION_IF , &info)) 
		printf( "ERROR wifi_set_ip_info()\n");

    if(!wifi_station_connect()) 
		printf( "ERROR wifi_station_connect()\n");

    if(!wifi_station_set_auto_connect( TRUE) ) 
		printf("ERROR wifi_station_set_auto_connect(1)\n");
    IP4_ADDR( & info.ip , 192 , 168 , 200 , 123);
    IP4_ADDR( & info.netmask , 255 , 255 , 255 , 0);
    IP4_ADDR( & info.gw, 192 , 168 , 200 , 1);
    if(!wifi_set_ip_info( STATION_IF , &info) ) 
		printf( "ERROR wifi_set_ip_info() \n ");
    if(!wifi_station_set_auto_connect( TRUE) ) 
		printf( "ERROR wifi_station_set_auto_connect(1) \n ");
*/

/**
  @brief Setup Wifi Network
  Credits: ideas borrowed from David Ogilvy(MetalPhreak)
 @return void
*/
MEMSPACE 
void setup_networking()
{

	printf( "Networking setup start\n");

	os_memset(&StationConfig, 0, sizeof(StationConfig));
	os_memset(&info, 0, sizeof(info));
	const char *ssid = SSID;
	const char *password = SSID_PASS;

	os_memset(ip_msg,0,sizeof(ip_msg));
	strcpy(ip_msg,"Not Connected");

    if(wifi_get_opmode() != STATION_MODE)
    {
        printf("ESP8266 not in STATION mode, restarting in STATION mode...\r\n");
		if(!wifi_station_disconnect()) 
			printf( "ERROR wifi_station_disconnect()\n");
		if(!wifi_set_opmode( STATION_MODE)) 
			printf( "ERROR wifi_set_opmode(STATION_MODE)\n");
		if(!wifi_station_set_reconnect_policy(FALSE))
			printf( "ERROR wifi_station_set_reconnect_policy(FALSE)\n");
		if(!wifi_station_set_auto_connect(FALSE) ) 
			printf( "ERROR wifi_station_set_auto_connect(FALSE)\n");
		if(wifi_get_phy_mode() != PHY_MODE_11N)
			wifi_set_phy_mode(PHY_MODE_11N);
    }

    if(wifi_get_opmode() == STATION_MODE)
    {
        printf("ESP8266 in STATION mode...\r\n");
        wifi_station_get_config(&StationConfig);
        os_sprintf(StationConfig.ssid, "%s", ssid);
        os_sprintf(StationConfig.password, "%s", password);
        if(!wifi_station_set_config(&StationConfig))
			printf( "ERROR wifi_set_config()\n");
        if(!wifi_station_get_config(&StationConfig))
			printf( "ERROR wifi_station_get_config()\n");
        if(!wifi_get_macaddr(STATION_IF, macaddr))
			printf( "ERROR wifi_get_macaddr()\n");
        printf("wifi_get_opmode: %d, SSID:[%s], PASSWORD:[%s]\n",
            (int)wifi_get_opmode(),
            StationConfig.ssid,
            "**********"
			/* StationConfig.password */
			);
    }
	else
	{
        printf("ESP8266 not in STATION mode\n");
	}

	// Wifi Connect
    if(!wifi_station_set_reconnect_policy(TRUE))
		printf( "ERROR wifi_station_set_reconnect_policy(TRUE)\n");

	if(!wifi_station_set_auto_connect(TRUE) ) 
		printf( "ERROR wifi_station_set_auto_connect(TRUE)\n");

	if(!wifi_station_connect())
		printf( "ERROR wifi_station_connect()\n");

    if(!wifi_station_dhcpc_start()) 
		printf("ERROR wifi_station_dhcpc_start()\n");
	// Monitor
    wifi_set_event_handler_cb( wifi_event_cb);

	printf( "Networking setup done\n");
}
