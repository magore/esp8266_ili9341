/**
 @file web.c 

 @brief Small web server for esp8266

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
#include "web/web.h"


// References: http://www.w3.org/Protocols/rfc2616/rfc2616.html

/// @brief max size of  ERROR/REDIRECT/STATUS Message buffer
#define MAX_MSG 1024 
/// @brief max size of  CGI token
#define CGI_TOKEN_SIZE 128
/// @brief max size of read/write socket buffers
/// Note: reducing this size below 1500 will slow down transfer a great deal
#define BUFFER_SIZE 1000

extern window *winmsg,*wintop;

///@brief socket buffers for this connection
rwbuf_t *web_connections[MAX_CONNECTIONS];

/// @brief Master espconn structure of the web server
espconn_t WebConn;
/// @brief Master network configuration for the web server
static esp_tcp WebTcp;

// =======================================================
// http://www.w3.org/Protocols/rfc2616/rfc2616-sec4.html#sec4.4

///@brief HTTP headers we understand 
header_t msg_headers[] = {
	{ "GET", 				TOKEN_GET },
	{ "PUT", 				TOKEN_PUT },
	{ "POST", 				TOKEN_POST },
	{ "HEAD", 				TOKEN_HEAD },
	{ "Host:", 				TOKEN_HOST },
	{ "User-Agent:", 		TOKEN_USER_AGENT },
	{ "HTTPS:", 			TOKEN_HTTPS},
	{ "DNT:", 				TOKEN_DNT},
	{ "Accept:", 			TOKEN_ACCEPT },
	{ "Accept-Language:", 	TOKEN_ACCEPT_LANGUAGE },
	{ "Accept-Encoding:", 	TOKEN_ACCEPT_ENCODING },
	{ "Connection:", 		TOKEN_CONNECTION },
	{ "Referer:", 			TOKEN_REFERER },
	{ "Content-Length:", 	TOKEN_CONTENT_LENGTH },
	{ "Content-Type:", 		TOKEN_CONTENT_TYPE },
	{ "Cache-Control:", 	TOKEN_CACHE_CONTROL },
	{ NULL,                 -1}
};



// =============================================================

///@brief HTTP status code messages
char *http_status[] = {
	// Successful
	 "200 OK",
	// 200 status code is by far the most common returned. 
	// It means, simply, that the request was received and understood 
	// and is being processed.
	 "201 Created",
	// 201 status code indicates that a request was successful 
	// and as a result, a resource has been created (for example a new page).
	 "202 Accepted",
	// 202 indicates that server has received and understood the request, 
	// and that it has been accepted for processing, although it may not 
	// be processed immediately.
	 "204 No Content",
	// 204 means that the request was received and understood, but that 
	// there is no need to send any data back.

	// Redirection
	 "301 Moved Permanently",
	// 301 tells a client that the resource they asked 
	// for has permanently moved to a new location. The response should 
	// also include this location. It tells the client to use the new URL 
	// the next time it wants to fetch the same resource.
	 "302 Found",
	// 302 tells a client that the resource they asked for has temporarily 
	// moved to a new location. The response should also include this location.
	//  It tells the client that it should carry on using the same URL 
	// to access this resource.
	 "304 Not Modified",
	// 304 status code is sent in response to a request (for a document) 
	// that asked for the document only if it was newer than the one the 
	// client already had. Normally, when a document is cached, the date 
	// it was cached is stored. The next time the document is viewed, the 
	// client asks the server if the document has changed. If not, the 
	// client just reloads the document from the cache.

	// Client Error
	 "400 Bad Request",
	// 400 indicates that the server did not understand the request due 
	// to bad syntax.
	 "401 Unauthorized",
	// 401 status code indicates that before a resource can be accessed, 
	// the client must be authorised by the server.
	 "403 Forbidden",
	// 403 status code indicates that the client cannot access the 
	// requested resource. That might mean that the wrong username 
	// and password were sent in the request, or that the permissions
	// on the server do not allow what was being asked.
	 "404 Not Found",
	// The best known of them all, the 404 status code indicates that 
	// the requested resource was not found at the URL given, and the 
	// server has no idea how long for.

	// Server Error

	 "500 Internal Server Error",
	// 500 status code (all too often seen by Perl programmers) indicates 
	// that the server encountered something it didn't expect and was unable 
	// to complete the request.
	 "501 Not Implemented",
	// The 501 status code indicates that the server does not support all 
	// that is needed for the request to be completed.
	 "502 Bad Gateway",
	// 502 status code indicates that a server, while acting as a proxy, 
	// received a response from a server further upstream that it judged
	//  invalid.
	 "503 Service Unavailable",
	// 503 status code is most often seen on extremely busy servers, and 
	// it indicates that the server was unable to complete the request 
	// due to a server overload.
	NULL
};

///@brief MIME types 
mime_t mimes[] = {
	{ PTYPE_TEXT,	"text/plain", ".text", ".txt" },
	{ PTYPE_HTML,	"text/html", ".htm", ".html" },
	{ PTYPE_PDF, 	"application/pdf", ".pdf",NULL},
	{ PTYPE_CSS, 	"text/css", ".css", NULL },
	{ PTYPE_CGI,	"text/html", ".cgi", NULL },
	{ PTYPE_JS,		"text/plain", ".js", NULL },
	{ PTYPE_XML,	"text/plain", ".xml", NULL },
	{ PTYPE_ICO, 	"mage/vnd.microsoft.icon", ".ico", NULL },
	{ PTYPE_GIF, 	"image/gif", ".gif", NULL },
	{ PTYPE_JPEG, 	"image/jpeg", ".jpg", ".jpeg" },
	{ PTYPE_MPEG, 	"video/mpeg", ".mpg", ".mpeg" },
	{ PTYPE_FLASH,	"application/x-shockwave-flash", ".swf", NULL },
	{ PTYPE_ERR, 	"text/html", NULL , NULL }
};


// =======================================================
MEMSPACE
/**
  @brief Turn on virtual LED
  @param[in] led: led to turn on
  @return void
*/
static _led = 0;
MEMSPACE
void led_on(int led)
{
	printf("LED%d on\n", led);
}
/**
  @brief Turn off virtual LED
  @param[in] led: led to turn off
  @return void
*/
MEMSPACE
void led_off(int led)
{
	printf("LED%d off\n", led);
}

// =======================================================

/**
  @brief Accept an incomming connection, setup connect_callback
  @param[in] *esp_config: ESP8266 network type an mode configuration structure
  @param[in] *esp_tcp_config:  network protocol structure
  @param[in] port:  network port to listen on
  @param[in] connect_callback: connection callback function pointer
  @return void
*/
MEMSPACE
static void tcp_accept(espconn_t *esp_config, esp_tcp *esp_tcp_config,
		uint16_t port, void (*connect_callback)(struct espconn *))
{

	int ret;
#if WEB_DEBUG & 2
	printf("\n=================================\n");
	printf("tcp_accept: %p\n", esp_config);
#endif

	memset(esp_tcp_config, 0, sizeof(esp_tcp));
	esp_tcp_config->local_port = port;
	memset(esp_config, 0, sizeof(espconn_t));
	esp_config->type = ESPCONN_TCP;
	esp_config->state = ESPCONN_NONE;
	esp_config->proto.tcp = esp_tcp_config;
	espconn_regist_connectcb(esp_config, (espconn_connect_callback)connect_callback);
	espconn_accept(esp_config);
	ret = espconn_tcp_set_max_con_allow(esp_config, MAX_CONNECTIONS);
	if(ret)
		printf("espconn_tcp_set_max_con_allow(%d) != (%d) failed\n", 
			MAX_CONNECTIONS, espconn_tcp_get_max_con_allow(esp_config));
#if WEB_DEBUG & 2
	ret = espconn_tcp_get_max_con_allow(esp_config);
	printf("espconn_tcp_get_max_con_allow:(%d)\n", ret);
#endif
}
// =======================================================


/**
  @brief Initialize socket read status and read index
  @param[in] p: rwbuf_t pointer
  @return void
*/
MEMSPACE
void rwbuf_rinit(rwbuf_t *p)
{
	if(!p) 
		return;
	p->received = 0;
	p->rind = 0;
}

/**
  @brief Initialize socket send status and write index
  @param[in] p: rwbuf_t pointer
  @return void
*/
MEMSPACE
void rwbuf_winit(rwbuf_t *p)
{
	if(!p) 
		return;
	p->send = 0;
	p->wind = 0;
}

/**
  @brief Display IPV4 address
  @param[in] msg: user supplied message
  @param[in] ip: IP address as uint8_t [4]
  @param[in] port: Port number
  @return void
*/

MEMSPACE
void display_ipv4(char *msg, uint8_t *ip, int port)
{
	printf("%s: %d.%d.%d.%d:%d\n",
		msg, 
		(int)ip[0],
		(int)ip[1],
		(int)ip[2],
		(int)ip[3],
		port  );
}

/**
  @brief Delete socket read/write buffers
  @param[in] p: rwbuf_t pointer to buffer to delete
  @return void
*/
MEMSPACE
void rwbuf_delete(rwbuf_t *p)
{
	if(!p) 
		return;

	p->delete = 0;
	// Free receive buffer
	p->rsize = 0;
	if(p->rbuf)
		safefree(p->rbuf);
	p->rbuf = NULL;
	rwbuf_rinit(p);

	// Free write buffer
	p->wsize = 0;
	if(p->wbuf)
		safefree(p->wbuf);
	p->wbuf = NULL;
	rwbuf_winit(p);

	if( p->conn != &WebConn)
	{
		p->conn = NULL;
	}
	p->remote_ip[0] = 0; p->remote_ip[1] = 0; p->remote_ip[2] = 0; p->remote_ip[3] = 0;
	p->remote_port = 0;
	p->local_ip[0] = 0; p->local_ip[1] = 0; p->local_ip[2] = 0; p->local_ip[3] = 0;
	p->local_port = 0;
	// FIXME
	safefree(p);
}


/**
  @brief Create socket read/write buffer for a connection
  @return void
*/
MEMSPACE
rwbuf_t *rwbuf_create()
{
	char *buf;

	// connect struction for this connection
	// Always over allocate to allow an extra EOS or TWO
	rwbuf_t *p = safecalloc(sizeof(rwbuf_t)+4,1);
	if(!p) 
	{
#if WEB_DEBUG & 1
		printf("rwbuf_create:calloc failed\n");
#endif
		return(NULL);
	}
	p->conn = NULL;

	// read buffer for this connection
	rwbuf_rinit(p);
	// Always over allocate to allow an extra EOS or TWO
	buf = safecalloc(BUFFER_SIZE+4,1);
	if(!buf) 
	{
#if WEB_DEBUG & 1
		printf("rwbuf_create rbuf: calloc failed\n");
#endif
		rwbuf_delete(p);
		return(NULL);
	}
	p->rbuf = buf;
	p->rsize = BUFFER_SIZE;

	// write buffer for this connection
	rwbuf_winit(p);
	// Always over allocate to allow an extra EOS or TWO
	buf = safecalloc(BUFFER_SIZE+4,1);
	if(!buf) 
	{
#if WEB_DEBUG & 1
		printf("rwbuf_create wbuf: calloc failed\n");
#endif
		rwbuf_delete(p);
		return(NULL);
	}
	p->wbuf = buf;
	p->wsize = BUFFER_SIZE;
	p->delete = 0;
	p->remote_ip[0] = 0; p->remote_ip[1] = 0; p->remote_ip[2] = 0; p->remote_ip[3] = 0;
	p->remote_port = 0;
	p->local_ip[0] = 0; p->local_ip[1] = 0; p->local_ip[2] = 0; p->local_ip[3] = 0;
	p->local_port = 0;
	return(p);
}


// =======================================================

/**
  @brief Find a read/write socket buffer for an espconn connection
  @param[in] conn: espconn pointer for this connection
  @param[in] *index: return index into pool of rwbuf_t connections
  @param[in] *msg: user debug message
  @return rwbuf_t pointer to found connection with *index set, or NULL with *index == -1 on failure
*/
MEMSPACE
rwbuf_t *find_connection(espconn_t *conn, int *index, char *msg)
{
	rwbuf_t *p;
	int i;

	*index = -1;
	if(!conn)
	{
#if WEB_DEBUG & 1
	printf("%s: find_connection: conn = NULL\n",msg);
#endif
		return(NULL);
	}

	if(conn == &WebConn)
	{
#if WEB_DEBUG & 2
	printf("%s: find_connection: conn == &WebConn\n",msg);
#endif
	}
	if(!conn->proto.tcp || !conn->proto.tcp->remote_ip || !conn->proto.tcp->remote_port)
	{
#if WEB_DEBUG & 1
	printf("%s: find_connection: conn->proto.tcp NULL\n",msg);
#endif
		return(NULL);
	}

	for(i=0;i<MAX_CONNECTIONS;++i)
	{
		if( (p = web_connections[i]) )
		{
			if(!p->conn)
				continue;

			// skip if the connection protocol doesn't match
			if (conn->type != p->conn->type)
				continue;

			// local ports must match
			if( conn->proto.tcp->local_port != p->local_port )
				continue;

			// remote ports must match
			if( conn->proto.tcp->remote_port != p->remote_port)
				continue;

			// If the structure pointer does not match check
			// 	the IP and ports
			if( memcmp( conn->proto.tcp->remote_ip, p->remote_ip,4) != 0)
				continue;
			// If the structure pointer does not match check
			// 	the IP and ports
			if( memcmp( conn->proto.tcp->local_ip, p->local_ip,4) != 0)
				continue;

			*index = i;
			return (p);
		}
	}
#if WEB_DEBUG & 2
	printf("%s: find_connection: mismatch conn=%p\n", msg, conn );
	display_ipv4("local  ", conn->proto.tcp->local_ip, conn->proto.tcp->local_port);
	display_ipv4("remote ", conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
#endif

	return(NULL);
}

/**
  @brief Allocate read/write socket buffers and add it to the working pool
  @param[in] conn: espconn pointer for this connection
  @return rwbuf_t pointer to created structure
*/
MEMSPACE
rwbuf_t *create_connection(espconn_t *conn)
{
	rwbuf_t *p;
	int index;
	int i;

	for(i=0;i<MAX_CONNECTIONS;++i)
	{
		if(web_connections[i] == NULL)
		{
			p = rwbuf_create();
			if(!p)
			{
#if WEB_DEBUG & 1
				printf("create_connection: NO MEMORY conn=%p\n",conn);
#endif
				break;
			}
			// FIXME no no no COPY conn structure, etc
			web_connections[i] = p;
			p->conn = conn;
            // local ports must match
            p->remote_port = conn->proto.tcp->remote_port;
			memcpy(p->remote_ip, conn->proto.tcp->remote_ip, 4);
            p->local_port = conn->proto.tcp->local_port;
			memcpy(p->local_ip, conn->proto.tcp->local_ip, 4);
#if WEB_DEBUG & 2
			printf("create_connection: conn=%p\n",conn);
#endif
			return(p);
			break;
		}
	}
#if WEB_DEBUG & 1
	printf("create_connection: NO FREE connections for conn=%p\n",conn);
#endif
	return(NULL);
}



/**
  @brief Delete a rwbuf_t entry using its index
  @param[in] p: rwbuf_t pointer
*/
MEMSPACE
void delete_connection(rwbuf_t *p)
{
	int i;

	for(i=0;i<MAX_CONNECTIONS;++i)
	{
		if( (p == web_connections[i]) )
		{
#if WEB_DEBUG & 2
			printf("delete_connection: conn=%p\n",p->conn);
			display_ipv4("local  ", p->local_ip, p->local_port);
			display_ipv4("remote ", p->remote_ip, p->remote_port);
#endif
			rwbuf_delete(p);
			web_connections[i] = NULL;
		}
	}
}


// =======================================================

/**
  @brief Write a byte (buffered) using the rwbuf_t socket buffers for this connection
  If the buffers are full the socket is written using espconn_send
  @param[in] *p: rwbuf_t pointer for this socket buffer
  @param[in] c: character to write
*/
MEMSPACE
MEMSPACE
int write_byte(rwbuf_t *p, int c)
{
	int i;
	int ret;

 	if(!p || !p->wbuf || !p->conn)
	{
#if WEB_DEBUG & 1
		//printf("\nwrite_byte: send Unexpected NULL\n");
#endif
		return(0);
	}

	// send socket is busy with the last send request
	// sent callback resets this
	if(p->send)
	{
		write_flush(p);
		if(!p || !p->wbuf || !p->conn)
		{
#if WEB_DEBUG & 1
			//printf("\nwrite_byte: send Unexpected NULL after wait\n");
#endif
			return(0);
		}
	}

	p->wbuf[p->wind++] = c;
 	if(p->wind >= p->wsize)
	{
		// sent callback resets send and wind for us
		ret = espconn_send(p->conn, (uint8_t *)p->wbuf, p->wind );
		if( ret )
		{
#if WEB_DEBUG & 1
			printf("write_byte: espconn_send(%d bytes) error:%d\n", p->wind, ret);
#endif
		}
		// flag that send socket is busy with the last send request
		p->send = p->wind;
#if WEB_DEBUG & 16
		for(i=0;i<p->wind;++i)
			putchar(p->wbuf[i]);
#endif
	}
	return(c);
}

/**
  @brief Wait for buffer to send for this connection
  If the buffers have any data in them it is written using espconn_send
  @param[in] *p: rwbuf_t pointer for this socket buffer
  @return 1 on success, 0 on error
*/
MEMSPACE
int wait_send(rwbuf_t *p)
{
	int i;
	int ret;

 	if(!p || !p->wbuf)
	{
#if WEB_DEBUG & 1
		printf("wait_buff: no sendbuffer\n");
#endif
		return 0;
	}

	// send socket is busy with the last send request
	if(p && p->send)
	{
#if WEB_DEBUG & 4
		printf("waiting for send\n");
#endif
		// send socket is busy with the last send request
		while(p && p->send)
		{
			optimistic_yield(1000);
		}
#if WEB_DEBUG & 4
		printf("wait_buff: ...sent\n");
#endif
	}
	return(1);
}

/**
  @brief Flush the the socket write buffers for this connection
  If the buffers have any data in them it is written using espconn_send
  @param[in] *p: rwbuf_t pointer for this socket buffer
  @return void
*/
MEMSPACE
void write_flush(rwbuf_t *p)
{
	int i;
	int ret;
	int wait;

 	if(!p || !p->wbuf)
		return;

	if(!wait_send(p))
	{
		return;
	}
 	if(p && p->wind)
	{
#if WEB_DEBUG & 16
// FIXME DEBUG
		for(i=0;i<p->wind;++i)
			putchar(p->wbuf[i]);
#endif
		// sent callback resets send and wind for us
		ret = espconn_send(p->conn, (uint8_t *)p->wbuf, p->wind );
		if( ret )
		{
#if WEB_DEBUG & 1
			printf("write_flush: espconn_send(%d bytes) error:%d\n", p->wind, ret);
			return;
#endif
		}
		// flag that send socket is busy with the last send request
		p->send = p->wind;
	}
	
	wait_send(p);
}

/**
  @brief Write data using buffered write_byte function
  @param[in] *p: rwbuf_t pointer for this socket buffer
  @param[in] *str: data buffer to write
  @param[in] len: number of bytes to write
  @return void
*/
MEMSPACE
void write_len(rwbuf_t *p, char *str, int len)
{
	while(len--) 
		write_byte(p,*str++);
}

/**
  @brief Write string using buffered write_byte function
  @param[in] *p: rwbuf_t pointer for this socket buffer
  @param[in] *str: 0 terminated string to write
  @return void
*/
MEMSPACE
void write_str(rwbuf_t *p, char *str)
{
	while(*str) 
		write_byte(p,*str++);
}

/**
   @brief low level vsock_printf function that calls socket write_byte 
   @param[in] *pr: printf structure and user buffer for this socket
   @param[in] c: character to write
   @return void
*/
MEMSPACE
static void _write_byte_fn(struct _printf_t *pr, char c)
{
		rwbuf_t *p = (rwbuf_t *) pr->buffer;

        write_byte(p,c);
        pr->sent++;
}


/** 
	@brief vsock_printf function
	@param[in] p: socket buffer structure
	@param[in] fmt: printf format string
	@param[in] va: va_list of arguments
	@return bytes written
*/
MEMSPACE
int vsock_printf(rwbuf_t *p, const char *fmt, va_list va)
{
	printf_t fn;

    fn.put = _write_byte_fn;
    fn.sent= 0;
	fn.buffer = (void *) p;

    _printf_fn(&fn, fmt, va);

	return(fn.sent);
}


/** 
	@brief sock_printf function
	@param[in] p: socket buffer structure
	@param[in] fmt: printf format string
	@param[in] ...: list of arguments
	@return bytes written
*/
MEMSPACE
int sock_printf(rwbuf_t *p, const char *fmt, ...)
{
    int len;
    va_list va;

    va_start(va, fmt);
    len = vsock_printf(p, fmt, va);
    va_end(va);

    return len;
}

// =================================================================
/** 
	@brief Send an HTML status message to socket
	@param[in] p: socket buffer structure
	@param[in] status: index into http_status table
	@param[in] type: mime type index
	@param[in] fmt: printf format string
	@param[in] ...: list of arguments
	@return bytes written, 0 on wrror
*/
MEMSPACE
int html_msg(rwbuf_t *p, int status, char type, char *fmt, ...)

{
    int len,ret;
	char *header;
	char *body;
	char *ptr;

	char *statp = html_status(status);
	char *mimep = mime_type( type );

	va_list args;

    va_start(args, fmt);

	
	// Always over allocate to allow an extra EOS or TWO
	header = safecalloc(MAX_MSG+4,1);
	if(!header) {
#if WEB_DEBUG & 1
		printf("html_msg: calloc failed\n");
		printf("\tstatus: %s\n",statp);
		printf("\tmime: %s\n",mimep);
		printf(fmt, args);
#endif
		return(0);
	}
	// Make header for room for 5 byte size starting at #

	// HTTP/1.1 200 OK\n
	snprintf(header,MAX_MSG,
		"HTTP/1.1 %s\nContent-Type: %s\nContent-Length: #    \n\n",
		statp, mimep);

	// Make body point to message after header
	body = header;
	while(*body)
		body++;


	// Start of body
	strncat(header,"<html><body>",MAX_MSG);
	ptr = header;
	while(*ptr)
		ptr++;

	len = strlen(body);
	// We allocated MAX_MSG to header so we subtract len
	vsnprintf(ptr,MAX_MSG - len - 1, fmt, args);

	strncat(header,"</html></body>",MAX_MSG);
	len = strlen(body);
	
	// Patch in the message size
	ptr = header;
	while(*ptr && *ptr != '#')
		++ptr;
	u5toa(ptr,(uint16_t) len);

	// send buffer
	len = strlen(header);
	write_len(p,header,len);

	safefree(header);
	return(len);
}



// =======================================================================
// MEMORY gets FUNCTIONS

/** 
	@brief in memory memory gets function
	@param[in] p: structure with size and offset used by mem_gets()
	@param[in] ptr: start of memory area
	@param[in] size: size of memory area
	@return pointer to start of memory area (ptr)
*/
MEMSPACE
char *meminit(mem_t *p, char *ptr, int size)
{
	if(!ptr)
		return(ptr);
    p->next = ptr;
    p->ptr = ptr;
	p->size = size;
	// We always allocate 4 bytes more then we need
    return(p->next);
}


/** 
	@brief Memory gets function
    We ASSUME we can replace any \n with a \0
	@param[in] p: structure with size and offset used by mem_gets()
	@return pointer to string
*/
MEMSPACE
char *memgets(mem_t *p)
{
	char *base = p->next;
	char *ptr = p->next;

    if(!p->ptr || p->size <= 0)
		return(NULL);

    // Mark end of this string and point to start of next string
    while(*ptr && *ptr != '\n' && p->size) {
        ++ptr;
		p->size--;
    }
	// replace newline with EOS
	*ptr = 0;
	// only advance pointer if size > 0
	if(p->size)
		++ptr;
	p->next = ptr;
    return(base);
}


// ==============================================================

/** 
	@brief return strung pointer for mime type index
	@param[in] type: index into mim_type table
	@return pointer to string, if out of range uses index PTYPE_ERR
*/
MEMSPACE
char *mime_type(int type)
{
	if(type >= PTYPE_ERR)
		type = PTYPE_ERR;
	return(mimes[type].mime);
}

/** 
	@brief Determin mimetype using file name extension
	@param[in] *name: name to test
	@return index into mimetype table
*/
MEMSPACE
int file_type(char *name)
{
	char *ptr;
	int i;

	ptr = name;
	while(*ptr)
	{
		if(*ptr == '.')
			name = ptr;
		++ptr;
	}

	for(i=0;i<PTYPE_ERR;++i) 
	{
		if(mimes[i].ext1 == NULL)
			break;
		if(strcasecmp(name, mimes[i].ext1) == 0)
			return(i);
		if(mimes[i].ext2 == NULL)
			continue;
		if(strcasecmp(name, mimes[i].ext2) == 0)
			return(i);
	}
	return(i);
}


/** 
	@brief Convert html status into string using http_status table
	@param[in] status: html status number
	@return http_status entry mathing status, or 500 Internal Server Error
*/
MEMSPACE
char *html_status(int status)
{
	int i;
	for(i=0;http_status[i];++i) {
		if(status == atoi(http_status[i]))
			return(http_status[i]);
	}
	// Not found ? Then find 500 error and return that
	for(i=0;http_status[i];++i) {
		if(500 == atoi(http_status[i]))
			return(http_status[i]);
	}
#if WEB_DEBUG & 1
	printf("html_status: %d not found\n",status);
#endif
	return("500 Internal Server Error");
}


/** 
	Initilize hinfo_t structure
	@param[in] *hi: hinfo_t structure pointer to initialize
	@return hinfo_t structure pointer to initialize
*/
MEMSPACE
void init_hinfo(hinfo_t *hi)
{
	hi->type = -1;	// GET,PUT,HEADER
	hi->filename = NULL;
	hi->connection = NULL;
	hi->args = NULL;
	hi->arg_ptr = NULL;
	hi->args_length = 0;
	hi->html_encoding= NULL;
	hi->content_type = NULL;
	hi->content_length = 0;
	hi->msg = NULL;
}


/** 
	@brief Match GET/POST message headers
	@param[in] *str: string to patch
	@param[in] **p: points past matched string on sucess
	@return header index or -1 on no match
*/
MEMSPACE
int match_headers(char *str, char **p)
{
	int len, i,ind;
	char *ptr;

	if(!str)
		return(-1);
	
	str = skipspaces(str);
	trim_tail(str);

	len = strlen(str);
	if(!len)
		return(-1);
	
	for(i=0;msg_headers[i].type != -1; ++i) {
		if( (ind = MATCHI_LEN(str, msg_headers[i].pattern)) ) {
			ptr = str + ind;
			ptr = skipspaces(ptr);
			// Instead of reallocating we now assume we can replace the EOL with EOS in ram
			// msg_headers[i].value = ptr; // stralloc(ptr);
			*p = ptr;
			return(i);
		}
	}
	return(-1);
}

/** 
	@brief Process GET argments or POST message name/value data.
	HTML encoding is done in place often reducting the size of the result.
	Convert name=value pairs into null terminated strings.
 	Names and values are each terminated with an EOS by replaceing
    '?', '&', '=' characters seen while scaning.
	@param[in] *hi: hinfo_t structure to fill
	@param[in] *ptr: GET arguments or PUT message body.
	@return *ptr pointer that points to just past the header area.
*/
MEMSPACE
char *process_args(hinfo_t *hi, char *ptr)
{
	char	*out;
	int h,l,num;
	char *end;
	int len;
	int size = 0;
	
	len = strlen(ptr);
#if WEB_DEBUG & 8
	printf("process_args:(%d)[%s]\n",len,ptr);
#endif

	// find next space or EOS - after arguments
	// HTTP/Versions follows space
	end = nextspace(ptr);
	if(*end)
		*end++ = 0;	// EOS after arguments

	hi->args = ptr;
	hi->arg_ptr = ptr;
	hi->args_length = len; // initial undecoded length

#if WEB_DEBUG & 8
	printf("process_args:[%s]\n",hi->args);
	printf("process_args_length:[%d]\n",hi->args_length);
#endif

	if(!hi->args_length)
	{
#if WEB_DEBUG & 8
		// this is not an error - just a notice
		printf("process_args: zero length\n");
#endif
		return(end);
	}
	len = hi->args_length;

	// We are overwriting the input buffer
	// The output will always be <= in size - so this works
	out = ptr;
	while(len > 0) {
		// Name = Value
		if(*ptr == '=') {	// Name =
			ptr++;
			--len;
			*out++ = 0;		// EOS break
			++size;
			continue;
		}
		if( *ptr == '&' ) {	// New Argument
			ptr++;
			--len;
			*out++ = 0;		// EOS break
			++size;
			continue;
		}
		if(*ptr == '+') {	// Space
			ptr++;
			--len;
			*out++ = ' ';
			++size;
			continue;
		}
		if ( *ptr == '%') {
			// skip %
			++ptr;
			--len;
			// Make sure we do not go past the end of the string
			if(!ptr[0] || !ptr[1])
			{
#if WEB_DEBUG & 1+8
	printf("process_args: HTML %HEX decode short string\n");
#endif
				break;
			}

			// Is the data HEX ?
			h = atodigit( *ptr++, 16);
			l = atodigit( *ptr++, 16);
			len -=2;

			if(h < 0 || l < 0) 	// The data was not HEX
			{
				num = ' ';
#if WEB_DEBUG & 1+8
	printf("process_args: HTML %HEX bad value\n");
#endif
			}
			else
			{	
				// The data was HEX
				num = (h << 4) | l;
			}

			*out++ = num;
			++size;
			continue;
		} // '%'

		// Default is to copy
		*out++ = *ptr++;								// Character
		++size;
		--len;
	}
	// EOS at very end of buffer
	*out = 0;
	// Compute new length of arguments string space after decoding
	hi->args_length = size;

	return(end);
}

/** 
	@brief Find first POST/GET argument
	@param[in] *hi: hinfo_t structure with arguments
	@return argument value pair
*/
MEMSPACE
char *first_arg(hinfo_t *hi)
{
	return(hi->arg_ptr = hi->args);
}

/** 
	@brief Find next POST/GET argument
     We have to skip a name and a value
	@param[in] *hi: hinfo_t structure with arguments
	@return argument value pair
*/
MEMSPACE
char *next_arg(hinfo_t *hi)
{
	uint8_t len;
	char *ptr = hi->arg_ptr;

	if(!ptr)
		return(NULL);

	// At or past the end of arguments
	if(ptr >= (hi->args + hi->args_length) )
		return(hi->arg_ptr = NULL);
	
	// Name
	len = strlen(ptr);
	// Empty name will also terminate arguments
	if(!len)	// Done ??
		return(hi->arg_ptr = NULL);

#if WEB_DEBUG & 8
	printf("next_arg:%s=",ptr);
#endif

	ptr += len+1;		// skip past name

	// Are we past the argument list ? if so we have an error
	if(ptr > (hi->args + hi->args_length) )
		return(hi->arg_ptr = NULL);

	// VALUE
#if WEB_DEBUG & 8
	printf("next_arg:%s\n",ptr);
#endif

	// Value can be empty
	len = strlen(ptr);
	// point past value
	ptr += len+1;		// skip past name

	// Are we at the end - or past the argument list ? 
	if(ptr >= (hi->args + hi->args_length) )
		return(hi->arg_ptr = NULL);

	return(hi->arg_ptr = ptr);
}

/** 
	@brief Return the argument name for current argument
	@param[in] *hi: hinfo_t structure with arguments
	@return argument name
*/
MEMSPACE
char *arg_name(hinfo_t *hi)
{
	char *ptr = hi->arg_ptr;
	int len;

	// NAME 
	// Are we at the end - or past the argument list ? 
	if(!ptr || ptr >= (hi->args + hi->args_length) )
	{
#if WEB_DEBUG & 8
	printf("arg_name: END OF ARGS\n");
#endif
		return(hi->arg_ptr = NULL);
	}

	len = strlen(ptr);

	if(!len)			// End of list
	{
#if WEB_DEBUG & 8
	printf("arg_name: END OF ARGS\n");
#endif
		return(hi->arg_ptr = NULL);
	}

#if WEB_DEBUG & 8
	printf("arg_name: name(%d)=%s\n", len,ptr);
#endif

	// Does name extend past the list ?
	if((ptr + len) > (hi->args + hi->args_length) )
	{
#if WEB_DEBUG & 1+8
        printf("arg_name: name(%d) size is > args length by (%d)\n",
            len, ((ptr + len) - (hi->args + hi->args_length)) );
#endif
		return(hi->arg_ptr = NULL);
	}

	return(ptr);
}

/** 
	@brief Return the argument value for current argument
	@param[in] *hi: hinfo_t structure with arguments
	@return argument value
*/
MEMSPACE
char *arg_value(hinfo_t *hi)
{
	char *ptr = hi->arg_ptr;
	int len;

	// NAME 
	// Are we at the end - or past the argument list ? 
	if(!ptr || ptr >= (hi->args + hi->args_length) )
	{
#if WEB_DEBUG & 8
	printf("arg_value: name END OF ARGS\n");
#endif
		return(hi->arg_ptr = NULL);
	}

	len = strlen(ptr);

	if(!len)			// End of list
	{
#if WEB_DEBUG & 8
	printf("arg_value: name END OF ARGS\n");
#endif
		return(hi->arg_ptr = NULL);
	}

#if WEB_DEBUG & 8
	printf("arg_value: name(%d)=%s\n", len,ptr);
#endif

	// Does name extend past the list ?
	if((ptr + len) > (hi->args + hi->args_length) )
	{
#if WEB_DEBUG & 1+8
        printf("arg_value: name(%d) size is > args length by (%d)\n",
            len, ((ptr + len) - (hi->args + hi->args_length)) );
#endif
		return(hi->arg_ptr = NULL);
	}
	ptr += len; 

	// VALUE
	if((ptr + 1) > (hi->args + hi->args_length) )
	{
#if WEB_DEBUG & 1+8
		printf("arg_value: value start is > args length by (%d)\n",
			((ptr + 1) - (hi->args + hi->args_length)) );
#endif
		hi->arg_ptr = NULL;
		return(ptr);
	}
	++ptr;

	// VALUE can be empty but must not be past the argument list
	len = strlen(ptr);

	// Does value extend past the list ?
	if((ptr + len) > (hi->args + hi->args_length) )
	{
#if WEB_DEBUG & 1+8
        printf("arg_value: value size is > args length by (%d)\n",
            ((ptr + len) - (hi->args + hi->args_length)) );
#endif

		return(hi->arg_ptr = NULL);
	}

	return(ptr);		// Value
}

/** 
	@brief Lookup and argument name and return its value
	@param[in] *hi: hinfo_t structure with arguments
	@param[in] *str: string to lookup
	@return argument value
*/
MEMSPACE
char *http_value(hinfo_t *hi, char *str)
{
	char *name,*value;

	first_arg(hi);
	while( (name = arg_name(hi)) ) {
		if(strcasecmp(name,str) == 0) {
			value = arg_value(hi);
			if(!value)
				return(NULL);
#if WEB_DEBUG & 8
		printf("http_value:%s=%s\n",str,value);
#endif
			return(value);
		}
		if(!next_arg(hi))
			break;
	}
	return(NULL);
}


/** 
	@brief Does the string look like a header token with a ':' ?
	@param[in] *str: string to test
	@param[in] **p: string pointer to set on match
	@return 1 if it looks like a header, otherwise 0
*/
MEMSPACE
int is_header(char *str, char **p)
{
	if(!str)
	{
		return (0);
	}
	while(*str && *str != ':') {
		if(*str <= ' ' || *str > 'z')
			break;
		++str;
	}
	if(*str == ':')
	{
		*p = str+1;
		return(1);
	}
	return(0);
}

/** 
	@brief Find next space or ? character
	@param[in] *ptr: string to search
	@return first space or ? character
*/
MEMSPACE
char *nextbreak(char *ptr)
{
   if(!ptr)
        return(ptr);

    while(*ptr) {
		if(*ptr == ' ' || *ptr == '\t' || *ptr == '?')
			break;
        ++ptr;
	}
    return(ptr);
}


/** 
	@brief Print a decimal number into a string without an EOS
	@param[in] *ptr: buffer to write number to
	@param[in] num: number to convet
	@return void
*/
MEMSPACE
void u5toa(char *ptr, uint16_t num)
{
	char buf[10];
	snprintf(buf,5,"%5u",num);
	memcpy(ptr,buf,5);
}



// =================================================================


/**
    @brief Write HTTP Contenet-Type/Content-Length header
    @param[in] *p: rwbuf_t pointer to socket buffer
    @param[in] status: html status message index
    @param[in] type: mimetype index
    @param[in] len: length of message
    @return void
*/
MEMSPACE
void html_head(rwbuf_t *p, int status, char type, int len	)
{
	sock_printf(p,"HTTP/1.1 %s\nContent-Type: %s\nContent-Length: %lu\n\n", 
		html_status(status),
		mime_type(type), 
		len );
}

                                                      
// ==============================================================================
/**
    @brief Get arguments for a GET or POST request
    @param[in] *p: rwbuf_t pointer to socket buffer
    @param[in] *hi: header structure of parsed result
    @return 0 on error, 1 on success
*/
MEMSPACE
int parse_http_request(rwbuf_t *p, hinfo_t *hi)
{
	int ret,len;
	int c,type,header;
	
	char *ptr;
	char *value;
	char *name;
	char *save;
	mem_t memp;

    init_hinfo(hi);        // Init Headers

	type = -1;

#if WEB_DEBUG & 8
	printf("\nparse_http_request\n");
#endif
	if(!p || !p->rbuf || !p->received)
	{
#if WEB_DEBUG & 1
		printf("EMPTY\n");
#endif
		return(0);
	}

	ptr = meminit(&memp, p->rbuf, p->received);

	// memgets converts '\n' to EOS, then points to next string
	while( (ptr = memgets(&memp)) ) 
	{
		save = ptr;
		// check against known headers
		header = match_headers(save,&ptr);

		if(header == -1)
		{
			// Does it at least look like a header ?
			if( !is_header(save, &ptr))
			{
#if WEB_DEBUG & 8
		printf("Header break ==== \n");
#endif
				break;
			}
#if WEB_DEBUG & 8
		printf("header skip:[%s]\n",save);
#endif
			// skip the unknown header
			continue;
		}

#if WEB_DEBUG & 8
		printf("header: %s %s\n",msg_headers[header].pattern,ptr);
#endif
		// ptr now points just after the matched header  pattern

		type = msg_headers[header].type;

		if(type == TOKEN_CONTENT_LENGTH)
		{
			hi->content_length = atoi(ptr);
			continue;
		}

		if(type == TOKEN_CONTENT_TYPE )
		{
			hi->content_type = ptr;
			continue;
		}

		// Process CONNECTION directive
		if(type == TOKEN_CONNECTION)
		{
			hi->connection = (char *)ptr;
			ptr = nextbreak(ptr);
			if(*ptr)
				*ptr++ = 0;	// Filename EOS
#if WEB_DEBUG & 8
			printf("connection: %s\n", hi->connection);
#endif
		}

		// Process GET,POST or HEAD directive
		// These methods put EOS characters withing the current string
		if(type == TOKEN_GET)
		{
			hi->type = type;
			hi->filename = ptr;
			ptr = nextbreak(ptr);
			c = *ptr;
			if(*ptr)
				*ptr++ = 0;	// Filename EOS
#if WEB_DEBUG & 8
			printf("filename: [%s]\n",hi->filename);
#endif
			// ? implies Arguments
			if(c == '?') 
				ptr = process_args(hi,ptr);

			// HTTP Encoding
			ptr = skipspaces(ptr);
			hi->html_encoding = ptr;
		} // GET

		else if(type == TOKEN_POST || type == TOKEN_HEAD )
		{
			hi->type = type;
			hi->filename = ptr;
			// find next space - after arguments
			ptr = nextbreak(ptr);
			c = *ptr;
			if(*ptr)
				*ptr++ = 0;	// Filename EOS
#if WEB_DEBUG & 8
			printf("filename: [%s]\n",hi->filename);
#endif
			// ? implies Arguments
			if(c == '?') {	
#if WEB_DEBUG & 8
				printf("Warning arguments not expected for %d\n", type);
#endif
				ptr = process_args(hi,ptr);
			}
			// HTTP Encoding
			ptr = skipspaces(ptr);
			hi->html_encoding = ptr;
		} // POST or HEAD
		else if( type == -1) {
#if WEB_DEBUG & 8
			printf("Unknown type: %d\n", hi->type);
#endif
			return(0);
		}
	}

	// POST has arguments after all headers
	if(hi->type == TOKEN_POST ) {
		// Get first non blank line
		while( (ptr = memgets(&memp)) ) {
#if WEB_DEBUG & 8
		printf("process_args:[%s]\n",ptr);
#endif
			if(strlen(ptr) > 1)
				break;
		}
		if(strlen(ptr) != hi->content_length)
		{
#if WEB_DEBUG & 8
		printf("strlen:[%d] != hi->content_length:[%d]\n",
			strlen(ptr) , hi->content_length);
#endif

		}
		if(ptr)
			ptr = process_args(hi,ptr);
	}

#if WEB_DEBUG & 8
	if(hi->type == TOKEN_POST && hi->content_length)
	{
		int len;
		printf("ARGS\n");
		first_arg(hi);
		while( (name = arg_name(hi)) ) {
			len = strlen(name);
			value = arg_value(hi);
			printf("\t%s=%s\n",name,value);
			if(!next_arg(hi))
				break;;
		}
		printf("END ARGS\n");
	}

	if( hi->type != -1) 
		printf("type: %s\n",msg_headers[hi->type].pattern);
	if(hi->filename)
		printf("Filename: %s\n",hi->filename);
	if(hi->html_encoding)
		printf("Html_encoding: %s\n",hi->html_encoding);
	if(hi->content_type ) 
		printf("Content-Type: %s\n",hi->content_type);
	if(hi->content_length) 
		printf("Content-Length: %d\n",hi->content_length);
#endif
	return(1);
}



// =================================================================
/**
  @brief Network receive callback function
  @param[in] *arg: connection pointer
  @param[in] *data: Data received
  @param[in] length: Length of data received
  @return void
*/
MEMSPACE
static void web_data_receive_callback(void *arg, char *data, unsigned short length)
{
	espconn_t *conn = (espconn_t *) arg;
    int index;
    rwbuf_t *p = find_connection(conn, &index, "web_data_receive_callback");


	if(!p)
	{
#if WEB_DEBUG & 1
		printf("web_data_receive_callback: mismatch conn=%p\n",conn);
#endif
		// delete the bad connection
		espconn_disconnect(conn);
		// esp_schedule();
		return;
	}

	// The main task has not yet read the data - we must discard it
	if(p->received)
	{
#if WEB_DEBUG & 1
		printf("web_data_receive_callback: receive buffer busy rejecting:%d\n",length);
#endif
		// FIXME flag overrun
		esp_schedule();
		return;
	}

	// TRIM buffer size if needed
	if(length > p->rsize)
	{
		// FIXME we may want to permit multi packet receive
		// FIXME flag overrun
#if WEB_DEBUG & 1
		printf("web_data_receive_callback: receive too big:%d, trim to:%d\n",length,p->rsize);
#endif
		length = p->rsize;
	}

	// reset read pointers
	rwbuf_rinit(p);
	if(p->rbuf)
	{
		memset(p->rbuf,0, p->rsize);
		memcpy(p->rbuf,data,length);
		p->received = length;
#if WEB_DEBUG & 2
		printf("web_data_receive_callback: received:%d\n",length);
#endif
	}
	else
	{
#if WEB_DEBUG & 1
		printf("web_data_receive_callback: buffer NULL\n");
#endif
	}
#if WEB_DEBUG & 2
    printf("web_data_receive_callback: conn=%p\n", conn);
    display_ipv4("local  ", conn->proto.tcp->local_ip, conn->proto.tcp->local_port);
    display_ipv4("remote ", conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
#endif

	esp_schedule();
}

/**
  @brief Network sent callback function
  @param[in] *arg: connection pointer
  @return void
*/
MEMSPACE
static void web_data_sent_callback(void *arg)
{
	espconn_t *conn = (espconn_t *) arg;

    int index;
    rwbuf_t *p = find_connection(conn, &index,"web_data_sent_callback");

	if(!p)
	{
#if WEB_DEBUG & 1
		printf("web_data_sent_callback: conn mismatch\n");
#endif
		return;
	}

	if(!p->send)
	{
#if WEB_DEBUG & 1
		printf("web_data_send_callback: NO send value\n");
#endif
		return;
	}

	if(p->delete)
	{
#if WEB_DEBUG & 1
		printf("web_data_send_callback: Delete\n");
#endif
#if WEB_DEBUG & 2
		printf("web_data_sent_callback: delete_connection: conn=%p\n",p->conn);
		display_ipv4("local  ", p->local_ip, p->local_port);
		display_ipv4("remote ", p->remote_ip, p->remote_port);
#endif
		espconn_disconnect(conn);
		rwbuf_delete(p);
		esp_schedule();
		return;
	}

#if WEB_DEBUG & 2
	printf("web_data_send_callback: sent:%d\n",p->send);
    display_ipv4("local  ", conn->proto.tcp->local_ip, conn->proto.tcp->local_port);
    display_ipv4("remote ", conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);

#endif
	rwbuf_winit(p);
	esp_schedule();
}

// FIXME we still have issues with deleting connection
/**
  @brief Network disconnect callback function
  @param[in] *arg: connection pointer
*/
MEMSPACE
static void web_data_disconnect_callback(void *arg)
{
	espconn_t *conn = (espconn_t *) arg;
    int i, index;
    rwbuf_t *p;

	if(!conn)
		return;

    p = find_connection(conn, &index, "web_data_disconnect_callback");

	if(p)
	{
		p->delete = 1;
		delete_connection(p);
	}
	else
	{
#if WEB_DEBUG & 2
		printf("web_data_disconnect_callback: disconnect mismatch %p\n", conn);
		display_ipv4("local  ", conn->proto.tcp->local_ip, conn->proto.tcp->local_port);
		display_ipv4("remote ", conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
#endif
	}

	esp_schedule();
}



/**
  @brief Network disconnect callback function
  @param[in] *arg: connection pointer
  FIXME TODO
*/
MEMSPACE
static void web_data_reconnect_callback(void *arg, int8_t err)
{
    struct espconn *conn = arg;
	int index;
	rwbuf_t *p = find_connection(conn,&index, "web_data_reconnect_callback");
  // FIXME TODO
#if WEB_DEBUG & 2
    printf("web_data_reconnect_callback: conn=%p\n", conn);
    display_ipv4("local  ", conn->proto.tcp->local_ip, conn->proto.tcp->local_port);
    display_ipv4("remote ", conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
#endif

}

/**
  @brief incomming connection setup callbacks
  @param[in] *conn: espconn structure pointer
  @return void
*/
MEMSPACE
static void web_data_connect_callback(espconn_t *conn)
{
	// FIXME *** really broken *** 
	// we need to malloc conn (we just have the one) and copy it (see tcp_accept!)
	// MUST BE MAX_CONNECTIONS - attach rwbuf_t stuff to that

	rwbuf_t *p = create_connection(conn);
    if(!p)
	{
#if WEB_DEBUG & 1
		printf("Can not create connection\n");
#endif
		esp_schedule();
		return;
	}
#if WEB_DEBUG & 2
    printf("web_data_connect_callback: conn=%p\n", conn);
    display_ipv4("local  ", conn->proto.tcp->local_ip, conn->proto.tcp->local_port);
    display_ipv4("remote ", conn->proto.tcp->remote_ip, conn->proto.tcp->remote_port);
#endif

	espconn_regist_recvcb(conn, web_data_receive_callback);
	espconn_regist_sentcb(conn, web_data_sent_callback);
	// disconnect will get the index into the connection pool
	espconn_regist_disconcb(conn, web_data_disconnect_callback);
	espconn_regist_reconcb(conn, web_data_reconnect_callback);

	// FIXME we should REUSE!!!!!!!
	// espconn_set_opt(conn, ESPCONN_REUSEADDR);
	espconn_regist_time(conn, 10, 0);
	// FIXME
	esp_schedule();
}


// ==========================================================================

/**
	@brief test to see if a character is a valid member of the CGI token character set
	CGI tokens have the following syntax @_example123_@
	They start with "@_" and end with "_@"
    "@_" must be first two characters of string
	May have upper and lower case letters, numbers and '-'
	@param[in] c: character to test
	@return 1 if the character is a CGI token or 0 if not
*/
MEMSPACE
int is_cgitoken_char(int c)
{
	if(c >= 'A' && c <= 'Z')
		return(1);
	else if(c >= 'a' && c <= 'z')
		return(1);
	else if(c >= '0' && c <= '9')
		return(1);
	else if( c == '-')
		return(1);
	else if(c == '@' || c == '_' )
		return(1);
	return(0);
}

/**
    @brief Find start of CGI token in a string
	CGI tokens have the following syntax @_example123_@
	They start with "@_" and end with "_@"
    "@_" must be first two characters of string
	May have upper and lower case letters, numbers and '-'
	param[in] *str: string to search
    @return offset of start of token, or -1 if not found
*/
MEMSPACE
int find_cgitoken_start(char *str)
{
	int offset = 0;

    while(*str) {
		// End of Token
		if( str[0] == '@' && str[1] == '_')
		{
			return(offset);
		}
		++str;
		++offset;
    }
	return(-1);
}

/** 

/**
    @brief Does the string have a CGI TOKEN at the beginning ?
	CGI tokens have the following syntax @_example123_@
	They start with "@_" and end with "_@"
    "@_" must be first two characters of string
	May have upper and lower case letters, numbers and '-'
    @return size of token or -1 on fail
*/
MEMSPACE
int is_cgitoken(char *str)
{
	int len = 0;

	if( str[0] == '@' && str[1] == '_' )
	{
		// token body is after @_, 2 bytes long
		len += 2;
		str += 2;

		while(*str) 
		{
			// End of Token
			if( str[0] == '_' && str[1] == '@')
			{
				len += 2;
#if WEB_DEBUG & 8
				printf("is_cgi_token:%d\n",len);
#endif
				return(len);
			}
			if(is_cgitoken_char(0xff & *str) == 0)
				break;
			++str;
			++len;
		}
	}
	return(-1);
}


/**
    @brief Replace CGI token with CGI result
	CGI tokens have the following syntax @_example123_@
	They start with "@_" and end with "_@"
    "@_" must be first two characters of string
	May have upper and lower case letters, numbers and '-'
    @param[in] *p: socket stream
    @param[in] *str: string with token, example @_A_@
    @return length of replaced text or 0 if no CGI handler was matched
*/
MEMSPACE
int rewrite_cgi_token(rwbuf_t *p, char *src)
{
	int len = 0;
	int ind;

	if((ind = MATCH_LEN(src,"@_TIMER_@")))
	{
		tz_t tz;
		tv_t tv;
		time_t secs;
		char *utc;

		gettimeofday( &tv, &tz );

		secs = tv.tv_sec;
		if( is_dst(secs) )
			tz.tz_dsttime = 1;

		utc = ctime(&secs);
	
		len = sock_printf(p, "Time: %s seconds: %lu.%06lu, minuteswest:%d, dsttime:%d",
			utc, 
			(uint32_t) tv.tv_sec,
			(uint32_t) tv.tv_usec,
			(int)tz.tz_minuteswest,
			(int)tz.tz_dsttime);
	}
	if((ind = MATCH_LEN(src,"@_DATE_@")))
	{
		time_t sec;
		time(&sec);
		len = sock_printf(p, "Date: %s", ctime(&sec));
	}

	return ( len );
}


/**
    @brief Process an incoming HTTP request
    @param[in] *p: rwbuf_t pointer to socket buffer
    @return void
*/
MEMSPACE
static void process_requests(rwbuf_t *p)
{
	int len,ind,size;
	long pos;
    uint8_t byte;
	int8_t type;
	char *name;
	char *param;
	char *value,*ptr;
	FILE *fi;
	hinfo_t hibuff;
	hinfo_t *hi;

	hi = &hibuff;
	// a token like; $i_am_a_token_name$, must be less then this in length
#define READBUFFSIZE 512
	char buff[READBUFFSIZE+1];

	if(!p->conn )
	{
#if WEB_DEBUG & 1
		printf("Process Requests: NULL conn\n");
#endif
		return;
	}
	if(!p->received)
	{
#if WEB_DEBUG & 1
		printf("Process Requests: NULL ARG\n");
#endif
		return;
	}
#if WEB_DEBUG & 2+8
	printf("\n==========================================\n");
	printf("Process Requests\n");
	printf("%d\n", system_get_free_heap_size());
	printf("length:%d\n", p->received);
	printf("[%s]\n", p->rbuf);
	printf("conn:%p\n", p->conn);
#endif

	if(!parse_http_request(p,hi))
	{
		html_msg(p, STATUS_BAD_REQ, PTYPE_HTML, "Not Understood Type:%d",type );
		write_flush(p);
		p->delete = 1;
		espconn_disconnect(p->conn);
		return;
	}

	type = hi->type;
	name = hi->filename;

	if(!name ) 
		name = "index.html";

	if (MATCH(name, "/")) 
		hi->filename = name = "index.html";	

	type = file_type(name);

#if WEB_DEBUG & 8
	printf("Filename: %s, type:%d\n", name,type);
#endif

// CGI
	if(type == PTYPE_CGI)
	{
		if(strstr(name,"timer.cgi")) 
		{
			name = hi->filename = "time.htm";
		}
		else if(strstr(name,"led.cgi"))
		{			
			name = hi->filename = "dout.htm";
			if( (param = http_value(hi,"led0")) )
			{
				if(!strcmp(param,"on")) led_on(0);
				else			led_off(0);
			}
			else led_off(0);
		}
		else if(strstr(name,"msg.cgi"))
		{			
			name = hi->filename = "msg.cgi";
			int away = 0;

			// send output to display
#if WEB_DEBUG & 8
			printf("found msg.cgi\n");
#endif

#ifdef DEBUG_STATS
			tft_fillWin(winmsg, winmsg->bg);
			tft_set_textpos(winmsg, 0,0);
			tft_set_font(winmsg,1);
#else
			tft_fillWin(wintop, wintop->bg);
			tft_set_textpos(wintop, 0,0);
			tft_set_font(wintop,2);

			tft_fillWin(winmsg, winmsg->bg);
			tft_set_textpos(winmsg, 0,0);
			tft_set_font(winmsg,2);
#endif

#if WEB_DEBUG & 8
			printf("msg.cgi: winmsg(%d,%d)\n",winmsg->h,winmsg->w);
#endif

			// TOP
			if( (param = http_value(hi,"title")) && strlen(param))
			{
#if WEB_DEBUG & 8
				printf("msg.cgi: %s\n",param);
#endif
#ifdef DEBUG_STATS
				tft_printf(winmsg, "%s\n", param);
#else
				tft_set_textpos(wintop,1,0);
				tft_printf(wintop, "%s", param);
#endif
			}
			if( (param = http_value(hi,"contact")) && strlen(param))
			{
#if WEB_DEBUG & 8
				printf("msg.cgi: %s\n",param);
#endif
#ifdef DEBUG_STATS
				tft_printf(winmsg, "%s\n", param);
#else
				tft_set_textpos(wintop,1,1);
				tft_printf(wintop, "%s", param);
#endif
			}
			// MESSAGE
			if( (param = http_value(hi,"location")) && strlen(param))
			{
#if WEB_DEBUG & 8
				printf("msg.cgi: %s\n",param);
#endif
				tft_printf(winmsg, "-> %s\n", param);
				++away;
			}
			else if( (param = http_value(hi,"location_other")) && strlen(param) )
			{
#if WEB_DEBUG & 8
				printf("msg.cgi: %s\n",param);
#endif
				tft_printf(winmsg, "-> %s\n", param);
				++away;
			}

			if( (param = http_value(hi,"return")) && strlen(param))
			{
#if WEB_DEBUG & 8
				printf("msg.cgi: %s\n",param);
#endif
				tft_printf(winmsg, "Return by\n");
				tft_printf(winmsg, "-> %s", param);
				++away;
			}
			else if( (param = http_value(hi,"return_other")) && strlen(param) )
			{
#if WEB_DEBUG & 8
				printf("msg.cgi: %s\n",param);
#endif
				tft_printf(winmsg, "Return by\n");
				tft_printf(winmsg, "-> %s", param);
				++away;
			}
			if(!away)
				tft_printf(winmsg, "-> Is Here");
		}
	}
	// END OF CGI
	
	type = file_type(name);
#if WEB_DEBUG & 32
	printf("name: %s, type:%d\n",name,type);
#endif

	fi = fopen(name,"r");
	/* Search the specified file in stored binaray html image */
	if(!fi)
	{
		html_msg(p, STATUS_NOT_FOUND, PTYPE_HTML, "File: %s not found\n", name);
		write_flush(p);
		p->delete = 1;
		espconn_disconnect(p->conn);
		return;
	}

#if WEB_DEBUG & 8
	printf("Found name: %s, type:%d\n",name,type);
#endif
	if(type == PTYPE_HTML || type == PTYPE_CGI || type == PTYPE_TEXT)
	{
		// socket write buffering
		while( 1 )
		{
			// keep track of file position
			pos = ftell(fi);
			memset(buff,0,READBUFFSIZE);
			len = fread(buff, 1, READBUFFSIZE,fi);
			if(len == 0)
				break;

			optimistic_yield(1000);

			// make sure that string operations stop at end of read data
			buff[len] = 0;

			// Seach for a CGI token

			ind = find_cgitoken_start(buff);

			if(ind < 0) // NOT FOUND
			{
				// Write all data in buffer
				write_len(p, buff, len);
				continue;
			}

			if(ind > 0)	// FOUND a token start header ahead of this position
			{
				// Write all data up to token start header
				write_len(p, buff, ind);
				// Reposition CGI token to start of buffer and reread
				fseek(fi, pos + ind, 0L);
				continue;
			}

			// FOUND CGI token at this position
			size = is_cgitoken(buff);

			if(size > 0)
			{
				buff[size] = 0;
#if WEB_DEBUG & 8
				printf("CGI: ind:%d, len:%d, size:%d [%s]\n", ind, len, size, buff);
#endif
				// TODO CGI actions go here
				rewrite_cgi_token(p, buff);	
				// Skip over token
				fseek(fi, pos + size, 0L);
				continue;
			}

			fseek(fi, pos + len, 0L);
#if WEB_DEBUG & 8
			printf("CGI BOGUS: ind:%d, len:%d, size:%d [%s]\n", ind, len, size, buff);
#endif
			// Write bogus CGI header and skip over it
			write_len(p, buff, 2);
		}
	}
	else 
	{	// NON CGI read and echo

		while(1)
		{
			// FIXME
			len = fread(buff, 1, READBUFFSIZE,fi);
			if(!len)
				break;
			optimistic_yield(1000);
			write_len(p, buff, len);
		}
	}
	fclose(fi);
	write_flush(p);

///FIXME if we want to support keep-alive we have to change this
	p->delete = 1;
	espconn_disconnect(p->conn);

#if WEB_DEBUG & 2+8
	printf("\nDone: ftell:%ld, len:%ld, feof%d\n",ftell(fi),len,feof(fi));
	printf("\n==========================================\n");
#endif
}

// =======================================================

int connections;

/**
    @brief Process ALL incoming HTTP requests
	@see process_requests()
    @return void
*/
MEMSPACE
void web_task()
{
	int i;
	rwbuf_t *p;
	// loop through all connections and process read actions

	connections = 0;
	for(i=0;i< MAX_CONNECTIONS;++i)
	{
		p = web_connections[i];

		if(!p)
			continue;

		++connections;
		// FIXME do we need to process the reqquest before delete ????
		if(p->delete)
		{
			delete_connection(p);
			continue;
		}

		if(p->received)
		{
#if WEB_DEBUG & 2
			printf("web_task: received:%d\n",p->received);
#endif
			process_requests(p);
			p->received = 0;
			optimistic_yield(1000);
		}
	}
	esp_schedule();
}

// only called at main initialization time
MEMSPACE
void web_init_connections()
{
	int i;
	for(i=0;i<MAX_CONNECTIONS;++i)
	{
		web_connections[i] = NULL;
	}
}

/**
    @brief Setup WEB server and accept connections
	@param[in] port: port number to run web server on
    @return void
*/
MEMSPACE
void web_init(int port)
{
	web_init_connections();
    wifi_set_sleep_type(NONE_SLEEP_T);
    tcp_accept(&WebConn, &WebTcp, port, web_data_connect_callback);
    //espconn_regist_time(&WebConn, 10, 0);
	espconn_set_opt(&WebConn, ESPCONN_REUSEADDR);
#if WEB_DEBUG & 2
    printf("\nWeb Server task init done\n");
#endif
}


