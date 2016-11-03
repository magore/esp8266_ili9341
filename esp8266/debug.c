/**
 @file debug.c

 @brief Small printf user function

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

#include "mathio.h"
#include "debug.h"

/// @brief _uart0_fn low level function that writes a character with uart0har()
/// @param[in] *p: structure with pointers to track number of bytes written
/// @param[in] ch: character to write
/// @return void
static void _uart0_fn(struct _printf_t *p, char ch)
{
        p->sent++;
//FIXME
		//uart_putc(0, ch);
#ifdef UART_QUEUED_TX
		uart_queue_putc(0, ch);
#else
		uart_putc(0, ch);
#endif
}
   
/// @brief printf function
/// @param[in] format: printf forat string
/// @param[in] ...: list of arguments
/// @return size of printed string
MEMSPACE
int uart0_printf(const char *format, ...)
{
    int len;   
    int i;
    printf_t fn;

	va_list va;

    fn.put = _uart0_fn;
    fn.sent = 0;
   
    va_start(va, format);
    _printf_fn(&fn, format, va);
    va_end(va);
 
    len = fn.sent;

	//uart_tx_flush(0);

    return len;
}
