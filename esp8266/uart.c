
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

#include "user_config.h"

#include "uart.h"
#include "uart_register.h"

#ifdef TELNET_SERIAL
	extern queue_t *bridge_send_queue;
	extern queue_t *bridge_receive_queue;
#endif

#define UARTS 2
#ifdef UART_QUEUED
	#ifdef UART_QUEUED_TX
		queue_t *uart_txq[UARTS]  = { NULL, NULL };
	#endif
	#ifdef UART_QUEUED_RX
		queue_t *uart_rxq[UARTS]  = { NULL, NULL };
	#endif
#endif
int uart_debug_port = 0;

// =================================================================
// @brief low level UART functions
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
 @brief Disable receive interrupts for a uart
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
int tx_fifo_putb(int uart_no, uint8_t c)
{
    if(tx_fifo_free(uart_no) )
	{
        WRITE_PERI_REG(UART_FIFO(uart_no), c);
		uart_tx_enable(uart_no);
		return(c);
	}
	return(-1);
}

/**
 @brief Remove a byte from the receive fifo
 We assume that rx_fifo_used() was called!
 @param[in] uart_no: uart number
 @return c, (or 0 if fill user error)
*/
MEMSPACE
int rx_fifo_getb(int uart_no)
{
    if( rx_fifo_used(uart_no) )
        return (READ_PERI_REG(UART_FIFO(UART0)) & 0xFF);
	return(-1);
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
    int  sent = 0;
    while(size && tx_fifo_free(uart_no))
    {
        tmp = *buf++;
        WRITE_PERI_REG(UART_FIFO(uart_no) , tmp);
        ++sent;
        --size;
    }
    // FIXME
	uart_tx_enable(uart_no);
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
    int  read = 0;
    while(size && rx_fifo_used(uart_no) )
    {
        tmp = READ_PERI_REG(UART_FIFO(UART0)) & 0xFF;
        *buf++ = tmp;
        ++read;
        --size;
    }
    return(read);
}

// =================================================================
/// @brief Polled Blocking I/O functions that poll
/**
	@brief Write a byte to a uart
	Note: This function waits/blocks util the write can happen
	@param[in] uart_no: uart number
	@param[in] data: byte to write
	@return void
*/

LOCAL MEMSPACE
int uart_putb(uint8 uart_no, uint8 data)
{
//FIXME verify free before call to tx_free_putc
	while( !tx_fifo_free(uart_no) )
		optimistic_yield(1000);
	return( tx_fifo_putb(uart_no, data) );
}

/**
	@brief Read a byte from a uart
	Note: This function waits/blocks util the read can happen
	@param[in] uart_no: uart number
	@return byte read
*/
LOCAL MEMSPACE
int uart_getb(int uart_no)
{
	uint8_t c;
//FIXME verify empty before call to rx_free_getc
	while( rx_fifo_empty(uart_no) )
		optimistic_yield(1000);
	c = rx_fifo_getb(uart_no);
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
int uart_putc(uint8 uart_no, char c)
{
    if (c == '\r')
        uart_putb(uart_no, '\n');
	return( uart_putb(uart_no, c) );
}

/**
	@brief Read a byte from a uart with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@param[in] uart_no: uart number
	@return byte read
*/
MEMSPACE
int uart_getc(int uart_no)
{
	uint8_t c;
	while(1)
	{
		optimistic_yield(1000);
		c = uart_getb(uart_no);
		if(c == '\n')
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
int uart0_putc(uint8 c)
{
	return( uart_putc(0,c) );
}

/**
	@brief Read a byte from uart0 with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@return byte read
*/
MEMSPACE
int  uart0_getc()
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
int uart1_putc(uint8 c)
{
	return( uart_putc(1,c) );
}

/**
	@brief Read a byte from uart1 with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@return byte read
*/
MEMSPACE
int uart1_getc()
{
	return( uart_getc(1) );
}

// =================================================================
// =================================================================
#ifdef UART_QUEUED

/// @brief uart queue functions 


#ifdef UART_QUEUED_RX
/**
	@brief Read a byte from a uart
	Note: This function waits/blocks util the read can happen
	@param[in] uart_no: uart number
	@return byte read
*/
LOCAL MEMSPACE
int uart_queue_getb(uint8_t uart_no)
{
	uint8_t c;
	while(queue_empty(uart_rxq[uart_no]))
		optimistic_yield(1000);
	c = queue_popc(uart_rxq[uart_no]);
	return (c);
}
/**
	@brief Read a byte from a uart with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@param[in] uart_no: uart number
	@return byte read
*/
MEMSPACE
uint8_t uart_queue_getc(uint8_t uart_no)
{
	uint8_t c;
	while(1)
	{
		c = uart_queue_getb(uart_no);
		if(c == '\r')
			continue;
		break;
	}
	return (c);
}

/**
	@brief Read a byte from uart 0 with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@param[in] uart_no: uart number
	@return byte read
*/
MEMSPACE
uint8_t uart0_queue_getc()
{
	uint8_t data = uart_queue_getc(0);
}


/**
	@brief Read a byte from uart 1 with NL to CR/NL conversion
	Note: This function waits/blocks util the read can happen
	@param[in] uart_no: uart number
	@return byte read
*/
MEMSPACE
uint8_t uart1_queue_getc()
{
	return(uart_queue_getc(1) );
}
#endif

#ifdef UART_QUEUED_TX
#ifdef UART_TASK
/**
  @brief Keep transmit process running if we get new data
*/
void uart_task(void)
{
	uint8_t data;
	if(tx_fifo_empty(0) )
	{ 
		if(!queue_empty(uart_txq[0]) )
		{
			data = queue_popc(uart_txq[0]);
			WRITE_PERI_REG(UART_FIFO(0), data);
			uart_tx_enable(0);
		}
	}
}
#endif
/**
	@brief Write a byte to a uart queue
	Note: This function waits/blocks util the queue has space
	@param[in] uart_no: uart number
	@param[in] data: byte to write
	@return void
*/
LOCAL MEMSPACE
void uart_queue_putb(uint8 uart_no, uint8 data)
{
	// enable transmit queue to empty existing data
	// uart_tx_enable(uart_no);
	while(queue_full(uart_txq[uart_no]))
	  optimistic_yield(1000);
	queue_pushc(uart_txq[uart_no], data);
	// enable transmit queue to empty new data
	uart_tx_enable(uart_no);
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
void uart_queue_putc(uint8_t uart_no, char c)
{
    if (c == '\r')
        uart_queue_putb(uart_no, '\n');
	uart_queue_putb(uart_no, c);
}

/**
	@brief Write a byte from uart 0 with NL to CR/NL conversion
	Note: This function waits/blocks util the write can happen
	@param[in] c: character
	@return void
*/
MEMSPACE
void uart0_queue_putc(char c)
{
    uart_queue_putc(0,c);
}

/**
	@brief Write a byte from uart 1 with NL to CR/NL conversion
	Note: This function waits/blocks util the write can happen
	@param[in] c: character
	@return void
*/
MEMSPACE
void uart1_queue_putc(char c)
{
    uart_queue_putc(1,c);
}
#endif

#endif

/**
	@brief high level function to see if we have input data available
	@param[in] uart_no: uart number
	@return byte read
*/
int kbhit(int uart_no, int eol)
{
	if(eol && uart_rxq[uart_no]->flags & QUEUE_EOL )
			return(1);
	if(!eol && !queue_empty(uart_rxq[uart_no]) )
		return(1);
	return(0);
}


// =================================================================
/**

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
	
	ETS_UART_INTR_DISABLE();

	// process transmit fifo empty interupt
	if(READ_PERI_REG(UART_INT_ST(0)) & UART_TXFIFO_EMPTY_INT_ST)
	{
// FIXME - we need a task to wake up normal serial send if we are diabled
#ifdef TELNET_SERIAL
		system_os_post(bridge_task_id, 0, 0);
#endif
		uart_tx_disable(0);
	}

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

// FIXME add callback pointers instead of hard coding it here
#ifdef TELNET_SERIAL
 			if(queue_space(bridge_receive_queue) )
				queue_pushc(bridge_receive_queue, data);
#endif

#ifdef UART_QUEUED_RX
//FIXME this really must be defined so we might want to remove the UART_QUEUED options
			if(queue_space(uart_rxq[0]) )
				queue_pushc(uart_rxq[0], data);
#endif
		}
#ifdef TELNET_SERIAL
//wakeup bridge task
		system_os_post(bridge_task_id, 0, 0);
#endif
	}

// FIXME add callback pointers instead of hard coding it here
// the fifo timeout is used here for periodic interrupt polling 

// TELNET queue
#ifdef TELNET_SERIAL
	while(!queue_empty(bridge_send_queue) && tx_fifo_free(0))
	{
		data = queue_popc(bridge_send_queue);
		WRITE_PERI_REG(UART_FIFO(0), data);
		uart_tx_enable(0);
	}
#endif

#ifdef UART_QUEUED_TX
	while(!queue_empty(uart_txq[0]) && tx_fifo_free(0))
	{
		uart_tx_enable(0);
		data = queue_popc(uart_txq[0]);
		WRITE_PERI_REG(UART_FIFO(0), data);
	}
#endif

	// acknowledge all uart interrupts
	WRITE_PERI_REG(UART_INT_CLR(0), 0xffff);
	ETS_UART_INTR_ENABLE();
}

// =================================================================
/**
	@brief Install debug uart 
	@return void
*/
MEMSPACE
void UART_SetPrintPort(uint8 uart_no)
{
	uart_debug_port = uart_no;
    if(uart_no==1)
    {
#ifdef UART_QUEUED_TX
        os_install_putc1(uart1_queue_putc);
#else
        os_install_putc1(uart1_putc);
#endif
    }
    else
    {
#ifdef UART_QUEUED_TX
        os_install_putc1(uart0_queue_putc);
#else
        os_install_putc1(uart0_putc);
#endif
    }
}
// =================================================================

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
	int i;
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
	int i;
    ETS_UART_INTR_DISABLE();
    ETS_UART_INTR_ATTACH(uart_callback,  0);

    uart_config(UART0, uart0_br, 8, 1, NO_PARITY);
    uart_config(UART1, uart1_br, 8, 1, NO_PARITY);

	for(i=0;i<UARTS;++i)
	{
#ifdef UART_QUEUED_TX
		if(!(uart_txq[i] = queue_new(256)))
			reset();
#endif
#ifdef UART_QUEUED_RX
		if(!(uart_rxq[i] = queue_new(256)))
			reset();
#endif
	}

	fdevopen( 
#ifdef UART_QUEUED_TX
	(void *)uart0_queue_putc, 
#else
	(void *)uart0_putc, 
#endif
#ifdef UART_QUEUED_RX
	(void *)uart0_queue_getc
#else
	(void *)uart0_getc
#endif
);

#ifdef UART_TASK
    if(set_timers(uart_task,1) == -1)
		printf("Uart task init failed\n");
#endif
	UART_SetPrintPort(0);

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



