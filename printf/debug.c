/**
 @file pr.c

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
#include "printf.h"

/// @brief debug_printf function
/// @param[in] fmt: printf forat string
/// @param[in] ...: vararg list or arguments
/// @return size of string
MEMSPACE
int uart0_printf(const char *fmt, ... )
{
	_fontc f;
	int ret;
	char *s;
    char buff[512];

    va_list va;
    va_start(va, fmt);
    ret = t_vsnprintf(buff, 510, fmt, va);
    va_end(va);

	s = buff;
	while(*s)
	{
		uart_putc(0, *s++);
	}
}
