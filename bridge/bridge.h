/**
 @file bridge.c

 @brief Serial bridge

 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
  Please retain a copy of this notice in any code you use it in.

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

#ifndef _BRIDGE_H_
#define _BRIDGE_H_

#include <user_config.h>
#include "queue.h"

/// @brief brige task queue an priority
enum
{
	bridge_task_id				= USER_TASK_PRIO_1,
	bridge_task_queue_length	= 16,
	BUFFER_SIZE 				= 1024,
};

/// @brief uart send and receive queue, @see queue.c
extern queue_t *uart_send_queue;
extern queue_t *uart_receive_queue;

/// @brief ESP8266 OS task queue
extern os_event_t bridge_task_queue[bridge_task_queue_length];

/* bridge.c */
MEMSPACE static void tcp_accept ( struct espconn *esp_config , esp_tcp *esp_tcp_config , uint16_t port , void (*connect_callback )(struct espconn *));
MEMSPACE static void tcp_data_sent_callback ( void *arg );
MEMSPACE static void tcp_data_receive_callback ( void *arg , char *data , uint16_t length );
MEMSPACE static void tcp_data_disconnect_callback ( void *arg );
MEMSPACE static void tcp_data_connect_callback ( struct espconn *new_connection );
MEMSPACE bridge_task_init ( int port );
MEMSPACE static void bridge_task ( os_event_t *events );

#endif
