#ifndef _UART_H_
#define _UART_H_

/**
 @file uart.c

 @brief Uart driver for ESP8266 based on Esprissif documents
 @see http://bbs.espressif.com/viewtopic.php?f=21&t=414

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

#ifdef USER_CONFIG
#include "user_config.h"
#endif

// Named address space
#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

#include "uart_register.h"

#define UART0   0
#define UART1   1

typedef enum
{
    ONE_STOP_BIT             = 0x1,
    TWO_STOP_BIT             = 0x3
} UartStopBitsNum;

typedef enum {
	NO_PARITY = 0,
	EVEN_PARITY = UART_PARITY_EN,
    ODD_PARITY = UART_PARITY_EN | UART_PARITY
} UartParityMode;

typedef enum
{
    BIT_RATE_300 = 300,
    BIT_RATE_600 = 600,
    BIT_RATE_1200 = 1200,
    BIT_RATE_2400 = 2400,
    BIT_RATE_4800 = 4800,
    BIT_RATE_9600   = 9600,
    BIT_RATE_19200  = 19200,
    BIT_RATE_38400  = 38400,
    BIT_RATE_57600  = 57600,
    BIT_RATE_74880  = 74880,
    BIT_RATE_115200 = 115200,
    BIT_RATE_230400 = 230400,
    BIT_RATE_460800 = 460800,
    BIT_RATE_921600 = 921600,
    BIT_RATE_1843200 = 1843200,
    BIT_RATE_3686400 = 3686400,
} UartBaudRate;

#ifndef UART_FIFO_LEN
	#define UART_FIFO_LEN  128  /* define the tx fifo length */
#endif

#ifndef FUNC_U0TXD
	#define FUNC_U0TXD    0
#endif

#ifndef FUNC_U0RXD
	#define FUNC_U0RXD    0
#endif

/* uart.c */
void uart_rx_enable ( uint8 uart_no );
void uart_rx_disable ( uint8 uart_no );
void uart_tx_enable ( uint8_t uart_no );
void uart_tx_disable ( uint8_t uart_no );
void tx_fifo_flush ( int uart_no );
void rx_fifo_flush ( int uart_no );
MEMSPACE int tx_fifo_used ( int uart_no );
MEMSPACE int tx_fifo_free ( int uart_no );
MEMSPACE int tx_fifo_empty ( int uart_no );
MEMSPACE int rx_fifo_used ( int uart_no );
MEMSPACE int rx_fifo_free ( int uart_no );
MEMSPACE int rx_fifo_empty ( int uart_no );
MEMSPACE uint8_t tx_fifo_putc ( int uart_no , uint8_t c );
MEMSPACE uint8_t rx_fifo_getc ( int uart_no );
MEMSPACE int tx_fifo_write ( int uart_no , uint8_t *buf , int size );
MEMSPACE int rx_fifo_read ( int uart_no , uint8_t *buf , int size );
LOCAL MEMSPACE void uart_putb ( uint8 uart_no , uint8 data );
LOCAL MEMSPACE uint8_t uart_getb ( int uart_no );
MEMSPACE void uart_putc ( uint8 uart_no , char c );
MEMSPACE uint8_t uart_getc ( int uart_no );
MEMSPACE void uart0_putc ( uint8 c );
MEMSPACE uint8_t uart0_getc ( void );
MEMSPACE void uart1_putc ( uint8 c );
MEMSPACE uint8_t uart1_getc ( void );
void uart_callback ( void *p );
void uart0_gets_add ( uint8_t data );
MEMSPACE void uart0_gets_init ( int size );
MEMSPACE int uart0_gets ( char *buf , int max );
MEMSPACE void uart_config ( uint8 uart_no , uint32_t baud , uint8_t data_bits , uint8_t stop_bits , uint8_t parity );
MEMSPACE void uart_init ( UartBaudRate uart0_br , UartBaudRate uart1_br );
MEMSPACE void uart_reattach ( void );
MEMSPACE void UART_SetPrintPort ( uint8 uart_no );

#endif
