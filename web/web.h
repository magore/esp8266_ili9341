/**
 @file web.h

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

#ifndef	__WEB_H__
#define	__WEB_H__

#include <user_config.h>

typedef struct espconn espconn_t;

// WEB CONNECTIONS
#ifndef MAX_CONNECTIONS
	#define MAX_CONNECTIONS 1
#endif

// =======================================================
// HTML HEADER information
typedef struct {
// GET /LEDCTL.CGI?led2=on&led3=on HTTP/1.1
// TOKEN_GET,TOKEN_POST,TOKEN_HEAD
    int type;
    char *filename;
    char *arg_ptr;
    char *args;
    uint16_t args_length;
    char *html_encoding;
// POST msg_pointers
// Content-Type: application/x-www-form-urlencoded
// Content-Length: 165
    char *content_type;
    uint16_t content_length;
// Follows msg headers
    char *msg;
} hinfo_t;

// =======================================================
// Memory buffering for socket reads
typedef struct {
    char *ptr;  // Current line
    char *next; // Next line
    int size;   // memory size
} mem_t;

// =======================================================
// Memory buffering for socket writes
#define IO_MAX 512  // buffered IO

// HTTP headers from the client
enum {
    TOKEN_GET,
    TOKEN_PUT,
    TOKEN_POST,
    TOKEN_HEAD,
    TOKEN_HOST,
    TOKEN_USER_AGENT,
    TOKEN_DNT,
    TOKEN_ACCEPT,
    TOKEN_ACCEPT_LANGUAGE,
    TOKEN_ACCEPT_ENCODING,
    TOKEN_CONNECTION,
    TOKEN_REFERER,
    TOKEN_CONTENT_LENGTH,
    TOKEN_CONTENT_TYPE,
    TOKEN_CACHE_CONTROL,
};


typedef struct {
    char *pattern;
    int type;
} header_t;

//HTTP code descriptions from
//  HTTP Status Codes for Beginners
//  All valid HTTP 1.1 Status Codes simply explained.
//  This article is part of the For Beginners series.
//  http://www.addedbytes.com/for-beginners/"},
// Web Server Status Codes
enum {
    STATUS_OK=200,
    STATUS_CREATED=201,
    STATUS_ACCEPTED=202,
    STATUS_NO_CONTENT=204,
    STATUS_MV_PERM=301,
    STATUS_MV_TEMP=302,
    STATUS_NOT_MODIF=304,
    STATUS_BAD_REQ=400,
    STATUS_UNAUTH=401,
    STATUS_FORBIDDEN=403,
    STATUS_NOT_FOUND=404,
    STATUS_INT_SERR=500,
    STATUS_NOT_IMPL=501,
    STATUS_BAD_GATEWAY=502,
    STATUS_SERV_UNAVAIL=503
};

enum {
    PTYPE_TEXT,
    PTYPE_HTML,
    PTYPE_PDF,
    PTYPE_CSS,
    PTYPE_CGI,
    PTYPE_JS,
    PTYPE_XML,
    PTYPE_ICO,
    PTYPE_GIF,
    PTYPE_JPEG,
    PTYPE_MPEG,
    PTYPE_FLASH,
    PTYPE_ERR
};

typedef struct {
    uint8_t type;
    char *mime;
    char *ext1;
    char *ext2;
} mime_t;


// =======================================================
typedef struct {
    espconn_t *conn;

    char *rbuf;
    int received;   // bytes creived
    int rind;       // index into rbuf
    int rsize;      // bytes allocated

    char *wbuf;
    int send;       // bytes to send
    int wind;       // index into wbuf
    int wsize;      // bytes allocated

	uint8_t remote_ip[4];
	uint8_t local_ip[4];
	int remote_port;
	int local_port;

	int delete;		// close connection
} rwbuf_t;


// ============================================================
/* web.c */
MEMSPACE void led_on ( int led );
MEMSPACE void led_off ( int led );
MEMSPACE void rwbuf_rinit ( rwbuf_t *p );
MEMSPACE void rwbuf_winit ( rwbuf_t *p );
MEMSPACE void rwbuf_delete ( rwbuf_t *p );
MEMSPACE rwbuf_t *rwbuf_create ( void );
MEMSPACE rwbuf_t *create_connection ( espconn_t *conn );
MEMSPACE rwbuf_t *find_connection ( espconn_t *conn , int *index, char *msg );
MEMSPACE void delete_connection ( rwbuf_t *p );
MEMSPACE int write_byte ( rwbuf_t *p , int c );
MEMSPACE void write_flush ( rwbuf_t *p );
MEMSPACE void write_len ( rwbuf_t *p , char *str , int len );
MEMSPACE void write_str ( rwbuf_t *p , char *str );
MEMSPACE void write_len_flush ( rwbuf_t *p , char *buff , int len );
MEMSPACE int vsock_printf ( rwbuf_t *p , const char *fmt , va_list va );
MEMSPACE int sock_printf ( rwbuf_t *p , const char *fmt , ...);
MEMSPACE int html_msg ( rwbuf_t *p , int status , char type , char *fmt , ...);
MEMSPACE char *meminit ( mem_t *p , char *ptr , int size );
MEMSPACE char *memgets ( mem_t *p );
MEMSPACE char *mime_type ( int type );
MEMSPACE int file_type ( char *name );
MEMSPACE char *html_status ( int status );
MEMSPACE hinfo_t *init_hinfo ( hinfo_t *hi );
MEMSPACE int match_headers ( char *str , char **p );
MEMSPACE char *process_args ( hinfo_t *hi );
MEMSPACE char *first_arg ( hinfo_t *hi );
MEMSPACE char *next_arg ( hinfo_t *hi );
MEMSPACE char *arg_name ( hinfo_t *hi );
MEMSPACE char *arg_value ( hinfo_t *hi );
MEMSPACE char *http_value ( hinfo_t *hi , char *str );
MEMSPACE char *is_header ( char *str );
MEMSPACE char *nextbreak ( char *ptr );
MEMSPACE void u5toa ( char *ptr , uint16_t num );
MEMSPACE int is_token ( int c );
MEMSPACE int read_html_token ( FILE *fi , char *str , int len );
MEMSPACE int rewrite_html_token ( char *src , int len );
MEMSPACE void html_head ( rwbuf_t *p , int status , char type , int len );
MEMSPACE int parse_http_request ( rwbuf_t *p , hinfo_t *hi );
MEMSPACE void web_task ( void );
MEMSPACE void web_init_connections ( void );
MEMSPACE void web_init ( int port );

// ============================================================


#endif	/* end of __WEB_H__ */ 
