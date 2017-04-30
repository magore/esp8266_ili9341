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

#include "user_config.h"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "queue.h"
#include "bridge.h"

os_event_t bridge_task_queue[bridge_task_queue_length];

///@brief uart send queue
///@see queue.c
queue_t *bridge_send_queue;
///@brief uart receive queue
///@see queue.c
queue_t *bridge_receive_queue;
///@brief TCP send buffer
static char *tcp_data_send_buffer;
///@brief TCP send buffer busy flag
static char tcp_data_send_buffer_busy;


///@brief network connection
static struct espconn *esp_data_tcp_connection;

/**
  @brief Accept an incoming connection, setup connect_callback
  @param[in] *esp_config, ESP8266 network type an mode configuration structure
  @param[in] *esp_tcp_config,  network protocol structure
  @param[in] port,  network port to listen on
  @param[in] connect_callback: connection callback function pointer
  @return void
*/
MEMSPACE
static void tcp_accept(struct espconn *esp_config, esp_tcp *esp_tcp_config,
		uint16_t port, void (*connect_callback)(struct espconn *))
{
	memset(esp_tcp_config, 0, sizeof(*esp_tcp_config));
	esp_tcp_config->local_port = port;
	memset(esp_config, 0, sizeof(*esp_config));
	esp_config->type = ESPCONN_TCP;
	esp_config->state = ESPCONN_NONE;
	esp_config->proto.tcp = esp_tcp_config;
	espconn_regist_connectcb(esp_config, (espconn_connect_callback)connect_callback);
	espconn_accept(esp_config);
	if(espconn_tcp_set_max_con_allow(esp_config, 1))
	{
        printf("espconn_tcp_set_max_con_allow(%d) != (%d) failed\n",
            1, espconn_tcp_get_max_con_allow(esp_config));
	}

}


/**
  @brief Network transmit finished callback function
  @param[in] *arg: unused
  @return void
*/
MEMSPACE
static void tcp_data_sent_callback(void *arg)
{
    tcp_data_send_buffer_busy = 0;
	// retry to send data still in the fifo
	system_os_post(bridge_task_id, 0, 0);
}

/**
  @brief Network receive callback function
  @param[in] *arg: unused
  @param[in] *data: Data received
  @param[in] length: Length of data received
  @return void
*/
MEMSPACE
static void tcp_data_receive_callback(void *arg, char *data, uint16_t length)
{
	uint16_t current;
	uint8_t byte;

// Echo debug
#if 0
	for(current = 0; current < length; current++)
		uart0_putc(data[current]);
#endif

// FIXME NOT WORKING
//
	for(current = 0; (current < length) && queue_space(bridge_send_queue); current++)
	{
		byte = (uint8_t)data[current];
		queue_pushc(bridge_send_queue, byte);
	}
	if(queue_empty(bridge_send_queue) && tx_fifo_empty(0))
		uart_tx_disable(0);
	else
		uart_tx_enable(0);
}

/**
  @brief Network disconnect callback function
  @param[in] *arg: unused
  @return void
*/
MEMSPACE
static void tcp_data_disconnect_callback(void *arg)
{
	// FIXME - make sure uart task notices this
	esp_data_tcp_connection = 0;
}

/**
  @brief incoming connection setup callbacks
  @param[in] *new_connection:
  @return void
*/
MEMSPACE
static void tcp_data_connect_callback(struct espconn *new_connection)
{
	if(esp_data_tcp_connection)
		espconn_disconnect(new_connection);
	else
	{
		esp_data_tcp_connection	= new_connection;
		tcp_data_send_buffer_busy = 0;

		espconn_regist_recvcb(esp_data_tcp_connection, tcp_data_receive_callback);
		espconn_regist_sentcb(esp_data_tcp_connection, tcp_data_sent_callback);
		espconn_regist_disconcb(esp_data_tcp_connection, tcp_data_disconnect_callback);

		espconn_set_opt(esp_data_tcp_connection, ESPCONN_REUSEADDR);

		queue_flush(bridge_send_queue);
		queue_flush(bridge_receive_queue);
	}
}

/**
  @brief Serial Bridge task initialize
  @param[in] port: network port
*/
MEMSPACE
bridge_task_init(int port)
{
	static struct espconn esp_data_config;
	static esp_tcp esp_data_tcp_config;

	if(!(bridge_send_queue = queue_new(BUFFER_SIZE)))
		reset();

	if(!(bridge_receive_queue = queue_new(BUFFER_SIZE)))
		reset();

	if(!(tcp_data_send_buffer = safecalloc(BUFFER_SIZE,1)))
		reset();

	wifi_set_sleep_type(NONE_SLEEP_T);

	tcp_accept(&esp_data_config, &esp_data_tcp_config, port, tcp_data_connect_callback);
	espconn_regist_time(&esp_data_config, 0, 0);
	esp_data_tcp_connection = 0;

	system_os_task(bridge_task, bridge_task_id, bridge_task_queue, bridge_task_queue_length);
	system_os_post(bridge_task_id, 0, 0);

	printf("\nbridge task init done\n");
}

/**
  @brief Main serial bridge task
  @param[in] *events: event signal message structure  - not used
  @return void
*/
MEMSPACE
static void bridge_task(os_event_t *events)
{
	uint16_t tcp_data_send_buffer_length;
	uint8_t byte;

	if(!queue_empty(bridge_receive_queue) && !tcp_data_send_buffer_busy)
	{
		// data available and can be sent now
		tcp_data_send_buffer_length = 0;

		while((tcp_data_send_buffer_length < BUFFER_SIZE) && !queue_empty(bridge_receive_queue))
		{
			byte = queue_popc(bridge_receive_queue);
//uart0_putc(byte);
			tcp_data_send_buffer[tcp_data_send_buffer_length++] = byte;
		}

		if(tcp_data_send_buffer_length > 0)
		{
			tcp_data_send_buffer_busy = 1;
			espconn_sent(esp_data_tcp_connection, tcp_data_send_buffer, tcp_data_send_buffer_length);
		}
	}
}

