
/**
 @file uart.c

 @brief Uart driver for ESP8266
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

#include "uart.h"

#ifdef UART_TASK
	extern queue_t *uart_send_queue;
	extern queue_t *uart_receive_queue;
#endif


// =================================================================
/// @brief uart0 receive queue
queue_t *uart0_gets_receive_queue;
/// @brief uart0 gets line bufer
char *uart0_gets_line;
/// @brief uart0 gets line buffer ready flag
uint8_t uart0_gets_ready = 0;
/// @brief uar0 gets line buffer size
int uart0_gets_size = 0;

//

// =================================================================

/**
 @brief Enable receive interrupts for a uart
 @param[in] uart_no: uart number
 @return void
*/
void uart_rx_enable(uint8 uart_no)
{
    SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_TOUT_INT_ENA);
}

/**
 @brief DIsable receive interrupts for a uart
 @param[in] uart_no: uart number
 @return void
*/
void uart_rx_disable(uint8 uart_no)
{
    CLEAR_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_RXFIFO_FULL_INT_ENA|UART_RXFIFO_TOUT_INT_ENA);
}

/**
 @brief Enable transmit interrupts for a uart
 @param[in] uart_no: uart number
 @return void
*/
void uart_tx_enable(uint8_t uart_no)
{
	SET_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_TXFIFO_EMPTY_INT_ENA);
}

/**
 @brief Disable transmit interrupts for a uart
 @param[in] uart_no: uart number
 @return void
*/
void uart_tx_disable(uint8_t uart_no)
{
	CLEAR_PERI_REG_MASK(UART_INT_ENA(uart_no), UART_TXFIFO_EMPTY_INT_ENA);
}

// =================================================================

/**
 @brief Flush transmit fifo for a uart
 @param[in] uart_no: uart number
 @return void
*/
void tx_fifo_flush(int uart_no)
{
    SET_PERI_REG_MASK(UART_CONF0(uart_no),  UART_TXFIFO_RST);  
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_TXFIFO_RST); 
}

/**
 @brief Flush receive fifo for a uart
 @param[in] uart_no: uart number
 @return void
*/
void rx_fifo_flush(int uart_no)
{
    SET_PERI_REG_MASK(UART_CONF0(uart_no),  UART_RXFIFO_RST);  
    CLEAR_PERI_REG_MASK(UART_CONF0(uart_no), UART_RXFIFO_RST); 
}

// =================================================================
/**
 @brief Get the number of bytes used in transmit fifo
 @param[in] uart_no: uart number
 @return bytes in use
*/
MEMSPACE
int tx_fifo_used(int uart_no)
{
	int used = (READ_PERI_REG(UART_STATUS(uart_no))>>UART_TXFIFO_CNT_S)&UART_TXFIFO_CNT;
	return(used);
}

/**
 @brief Get the number of bytes free in transmit fifo
 @param[in] uart_no: uart number
 @return bytes free
*/
MEMSPACE
int tx_fifo_free(int uart_no)
{
	int used = (READ_PERI_REG(UART_STATUS(uart_no))>>UART_TXFIFO_CNT_S)&UART_TXFIFO_CNT;
    int free = UART_FIFO_LEN - used;
	return(free);
}

/**
 @brief Test if the transmit fifo is empty
 @param[in] uart_no: uart number
 @return 1 if empty, 0 otherwise
*/
MEMSPACE
int tx_fifo_empty(int uart_no)
{
	int used = (READ_PERI_REG(UART_STATUS(uart_no))>>UART_TXFIFO_CNT_S)&UART_TXFIFO_CNT;
	return( used ? 0 : 1);
}

/**
 @brief Get the number of bytes used in receive fifo
 @param[in] uart_no: uart number
 @return bytes in use
*/
MEMSPACE
int rx_fifo_used(int uart_no)
{
	// number of characters in fifo
	int used = (READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT;
	return(used);
}

/**
 @brief Get the number of bytes free in receive fifo
 @param[in] uart_no: uart number
 @return bytes free
*/
MEMSPACE
int rx_fifo_free(int uart_no)
{
	// number of characters in fifo
	int used = (READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT;
    int free = UART_FIFO_LEN - used;
	return(used);
}

/**
 @brief Test if the receive fifo is empty
 @param[in] uart_no: uart number
 @return 1 if empty, 0 otherwise
*/
MEMSPACE
int  rx_fifo_empty(int uart_no)
{
	// number of characters in fifo
	int used = (READ_PERI_REG(UART_STATUS(UART0))>>UART_RXFIFO_CNT_S)&UART_RXFIFO_CNT;
	return( used ? 0 : 1);
}

// =================================================================

/**
 @brief Add a byte to the trasmit fifo
 We assume that tx_fifo_free() was called!
 @param[in] uart_no: uart number
 @param[in] c: byte to add
 @return c, (or 0 if full user error)
*/
MEMSPACE
uint8_t tx_fifo_putc(int uart_no, uint8_t c)
{
	int  count = tx_fifo_free(uart_no);
	if(count ==0)
		return(0);
	WRITE_PERI_REG(UART_FIFO(uart_no), c);
	return(c);
}

/**
 @brief Remove a byte from the receive fifo
 We assume that rx_fifo_used() was called!
 @param[in] uart_no: uart number
 @return c, (or 0 if fill user error)
*/
MEMSPACE
uint8_t rx_fifo_getc(int uart_no)
{
	uint8_t c;
	int count = rx_fifo_used(uart_no);
	if(count ==0)
		return(0);
	c = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
	return(c);
}

// =================================================================
/**
	@brief Write a data buffer to the transmit fifo
	Note: This function does not wait/block util there is enough free space 
	to meet the request.
	So you must check that the return value matches the size.
	@param[in] uart_no: uart number
	@param[in] *buf: output buffer
	@param[in] size: size of input buffer
	@return number of bytes sent
*/

MEMSPACE
int tx_fifo_write(int uart_no, uint8_t *buf, int size)
{
	uint8_t tmp;
	int  count = tx_fifo_free(uart_no);
	int  sent = 0;
	while(size)
	{
		count = tx_fifo_free(uart_no);
		if(count > size)
			count = size;
		while(count)
		{
			tmp = *buf++;
			WRITE_PERI_REG(UART_FIFO(uart_no) , tmp);
			++sent;
			--size;
			--count;
		}
	}
	// FIXME
	SET_PERI_REG_MASK(UART_INT_ENA(UART0), UART_TXFIFO_EMPTY_INT_ENA);
	return(sent);
}
		
/**
	@brief Read a data buffer from the receive fifo
	Note: This function does not wait/block util there is enough free space 
	to meet the request.
	So you must check that the return value matches the size.
	@param[in] uart_no: uart number
	@param[in] *buf: output buffer
	@param[in] size: size of output buffer
	@return number of bytes actually read from the fifo - may not be size!
*/
MEMSPACE
int rx_fifo_read(int uart_no, uint8_t *buf, int size)
{
	uint8_t tmp;
	int  count;
	int  read = 0;
	while(size)
	{
		count = rx_fifo_used(uart_no);
		if(count > size)
			count = size;
		while(count)
		{
			tmp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
			*buf++ = tmp;
			++read;
			--size;
			--count;
		}
	}
	return(read);
}

// =================================================================
/// @brief Blocking I/O functions
/**
	@brief Write a byte to a uart
	Note: This function waits/blocks util the write can happen
	@param[in] uart_no: uart number
	@param[in] data: byte to write
	@return void
*/
LOCAL MEMSPACE
void uart_putb(uint8 uart_no, uint8 data)
{
	while( !tx_fifo_free(uart_no) )
		;
	tx_fifo_putc(uart_no, data);
}

/**
	@brief Read a byte from a uart
	Note: This function waits/blocks util the read can happen
	@param[in] uart_no: uart number
	@return byte read
*/
LOCAL MEMSPACE
uint8_t uart_getb(int uart_no)
{
	uint8_t c;
	while( !rx_fifo_empty(uart_no) )
		;
	c = rx_fifo_getc(uart_no);
	return (c);
}

// =================================================================
/**
	@brief Write a byte from a uart with NL to CR/NL conversion
	Note: This function waits/blocks util the write can happen
	@param[in] uart_no: uart number
	@param[in] c: character
	@return void
*/
MEMSPACE
void uart_putc(uint8 uart_no, char c)
{
    if (c == '\n')
    {
        uart_putb(uart_no, '\r');
    }
	uart_putb(uart_no, c);
}

/**
	@brief Read a byte from a uart with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@param[in] uart_no: uart number
	@return byte read
*/
MEMSPACE
uint8_t uart_getc(int uart_no)
{
	uint8_t c;
	while(1)
	{
		c = uart_getb(uart_no);
		if(c == '\r')
			continue;
		break;
	}
	return (c);
}
// =================================================================
/**
	@brief Write a byte to uart0 with NL to CR/NL conversion
	Note: This function waits/blocks util the write can happen
	@param[in] c: byte to write
	@return void
*/
MEMSPACE
void uart0_putc(uint8 c)
{
	uart_putc(0,c);
}

/**
	@brief Read a byte from uart0 with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@return byte read
*/
MEMSPACE
uint8_t uart0_getc()
{
	return( uart_getc(0) );
}

// =================================================================
/**
	@brief Write a byte to uart1 with NL to CR/NL conversion
	Note: This function waits/blocks util the write can happen
	@param[in] c: byte to write
	@return void
*/
MEMSPACE
void uart1_putc(uint8 c)
{
	uart_putc(1,c);
}

/**
	@brief Read a byte from uart1 with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@return byte read
*/
MEMSPACE
uint8_t uart1_getc()
{
	return( uart_getc(1) );
}

// =================================================================

/**
	@brief Uart interrupt callback function
    Process all receive and transmit events here
    @param[in] *p: callback pointer - currently unused
	@return void
*/
void uart_callback(void *p)
{
	uint8_t data;
	uint8_t val;
	
	int size;
	int ind;

	ETS_UART_INTR_DISABLE();

	// receive fifo timeout or full intr
	// the fifo timeout is used here for periodic interrupt polling 
	if(READ_PERI_REG(UART_INT_ST(0)) & 
		(UART_RXFIFO_TOUT_INT_ST | UART_RXFIFO_FULL_INT_ST))
	{
		// If we fail to fetch all FIFO data we will get another 
		// interrupt immediately after we enable it
		while(rx_fifo_used(0) > 0)
		{
			data = READ_PERI_REG(UART_FIFO(0));
			// FIXME data ?
// FIXME add callback pointers instead of hard coding it here
#ifdef TELNET_SERIAL
 			if(queue_space(uart_receive_queue) )
				queue_pushc(uart_receive_queue, data);
#endif
// non blocking line buffered queue for gets
			uart0_gets_add(data);
		}
#ifdef TELNET_SERIAL
		system_os_post(bridge_task_id, 0, 0);
#endif
	}

// FIXME add callback pointers instead of hard coding it here
// the fifo timeout is used here for periodic interrupt polling 
#ifdef TELNET_SERIAL
	if(queue_empty(uart_send_queue) && tx_fifo_empty(0))
		uart_tx_disable(0);
	else
		uart_tx_enable(0);

	while(!queue_empty(uart_send_queue) && tx_fifo_free(0))
	{
		data = queue_popc(uart_send_queue);
		WRITE_PERI_REG(UART_FIFO(0), data);
		uart_tx_enable(0);
	}
#endif

	// process transmit fifo empty interupt
	if(READ_PERI_REG(UART_INT_ST(0)) & UART_TXFIFO_EMPTY_INT_ST)
	{
#ifdef TELNET_SERIAL
		system_os_post(bridge_task_id, 0, 0);
#endif
	}

	// acknowledge all uart interrupts
	WRITE_PERI_REG(UART_INT_CLR(0), 0xffff);
	ETS_UART_INTR_ENABLE();
}

// =================================================================

/**
  @brief transfer data into the uart0_gets_receive_queue
  Provide line buffering for uart0_gets()
  @param[in] data: character to add to queue
  @return void
*/
void uart0_gets_add(uint8_t data)
{
	int ind;
	if(!uart0_gets_size || !uart0_gets_receive_queue || !uart0_gets_line)
		return;

	// note: we buffer only the most receint line received
	if(data == '\r')
	{
		// transfer current queue to uart0_gets_line buffer
		ind = 0;
		while(!queue_empty(uart0_gets_receive_queue) && ind < uart0_gets_size)
		{
			uart0_gets_line[ind++]  = queue_popc(uart0_gets_receive_queue);
		}
		uart0_gets_line[ind++]  = 0;
		uart0_gets_ready = 1;
	}
	else
	{
		// filter input data
		if(data == '\t' || (data >= ' ' && data <= 0x7e))
		{
			if(queue_space(uart0_gets_receive_queue) )
				queue_pushc(uart0_gets_receive_queue, data);
		}
	}
}

/**
  @brief allocate and initialize uart0_gets data structures
  Provide line buffering for uart0_gets()
  @param[in] size: size of line buffer
  @return void
*/
MEMSPACE
void uart0_gets_init(int size)
{
	uart0_gets_size = 0;
	uart0_gets_ready = 0;
	// non blocking line buffered queue for gets
    if(!(uart0_gets_receive_queue = queue_new(size)))
        reset();
    if(!(uart0_gets_line = calloc(size+1,1)))
        reset();
	uart0_gets_line[0] = 0;
	uart0_gets_size = size;
}

/**
  @brief uart0_gets non-blocking line buffered gets 
  @param[in] buf: user buffer to fill
  @param[in] max: miximum size of buf
  @return 0, or number of characters in line
*/
MEMSPACE
int uart0_gets(char *buf, int max)
{
	int ind;
	int data;
	*buf = 0;

	if(!uart0_gets_size || !uart0_gets_receive_queue || !uart0_gets_line )
		return(0);

	if(uart0_gets_ready)
	{
		ind = 0;
		while(max--)
		{
			data = uart0_gets_line[ind++];
			*buf++ = data;
			if(!data)
				break;
		}
		uart0_gets_ready = 0;
		return(ind);
	}
	return 0;
}


// ======================================================================


/**
	@brief Uart configuration, baud rate, data and stop bits, parity
    @param[in] uart_no: uart number
    @param[in] baud: baud
    @param[in] data_bits: number of data bits, 5 .. 8
    @param[in] stop_bits: number of stop bits, ONE_STOP_BIT|TWO_STOP_BIT
    @param[in] parity: parity, NO_PARITY,ODD_PARITY,EVEN_PARITY
	@return void
*/
MEMSPACE
void uart_config(uint8 uart_no, uint32_t baud, uint8_t data_bits, uint8_t stop_bits, uint8_t parity)
{
    if (uart_no == UART0)
    {
        PIN_PULLUP_DIS(PERIPHS_IO_MUX_U0TXD_U);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_U0TXD);
        PIN_PULLUP_DIS (PERIPHS_IO_MUX_U0RXD_U);
        PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_U0RXD);
    }
    else
    {
// GPIO2
		PIN_PULLUP_DIS(PERIPHS_IO_MUX_GPIO2_U);
		PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_U1TXD_BK);
    }

	uart_div_modify(uart_no, UART_CLK_FREQ / baud);

// Data bits 5..8
	data_bits = data_bits - 5;

// STOP BITS 1..2
    if(stop_bits == 2)
        stop_bits = TWO_STOP_BIT;
    else
        stop_bits = ONE_STOP_BIT;

// PARITY NO_PARITY,ODD_PARITY,EVEN_PARITY

	WRITE_PERI_REG(UART_CONF0(uart_no),
            ((data_bits & UART_BIT_NUM) << UART_BIT_NUM_S) |
            ((stop_bits & UART_STOP_BIT_NUM) << UART_STOP_BIT_NUM_S) |
            parity);

//clear rx and tx fifo,not ready
//RESET FIFO
    tx_fifo_flush(uart_no);
    rx_fifo_flush(uart_no);

    if (uart_no == UART0)
    {
	   WRITE_PERI_REG(UART_CONF1(uart_no),
			(( 2 & UART_RX_TOUT_THRHD) << UART_RX_TOUT_THRHD_S) | UART_RX_TOUT_EN |
			((16 & UART_RXFIFO_FULL_THRHD) << UART_RXFIFO_FULL_THRHD_S) |
			((64 & UART_TXFIFO_EMPTY_THRHD) << UART_TXFIFO_EMPTY_THRHD_S) );
		WRITE_PERI_REG(UART_INT_CLR(uart_no), 0xffff);
		WRITE_PERI_REG(UART_INT_ENA(uart_no), UART_RXFIFO_TOUT_INT_ENA | UART_RXFIFO_FULL_INT_ENA);
    }
}

/**
	@brief initialize uart0 and uart1
    Defaults: 8 = data bits, 1 = stop bits, no parity
    @param[in] uart0_br: baud rate for uart 0
    @param[in] uart1_br: baud rate for uart 1
	@return void
*/
MEMSPACE
void uart_init(UartBaudRate uart0_br, UartBaudRate uart1_br)
{
    ETS_UART_INTR_DISABLE();
    ETS_UART_INTR_ATTACH(uart_callback,  0);

    uart_config(UART0, uart0_br, 8, 1, NO_PARITY);
    uart_config(UART1, uart1_br, 8, 1, NO_PARITY);

	uart0_gets_init(256);

#ifdef POSIX
	fdevopen((void *)uart0_putc, (void *)uart0_getc);
#endif

	// Install the debug port callback on uart 0
    os_install_putc1((void *)uart0_putc);   //print output at UART0
    ETS_UART_INTR_ENABLE();
}

/**
	@brief Reinitialize uart0 and uart1
	calls uart_init()
	@return void
*/
MEMSPACE
void uart_reattach()
{
    uart_init(BIT_RATE_115200, BIT_RATE_115200);
}
//========================================================



/**
	@brief Install debug uart 
	@return void
*/
MEMSPACE
void UART_SetPrintPort(uint8 uart_no)
{
    if(uart_no==1)
    {
        os_install_putc1(uart1_putc);
    }
    else
    {
        os_install_putc1(uart0_putc);
    }
}
