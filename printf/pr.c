/**
 @file pr.c

 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief Small printf with floating point support

 @see: http://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option)
 any later version.

 printf is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#include "user_config.h"
#include "printf.h"

MEMSPACE window *tft_setpos(int16_t x, int16_t y);
MEMSPACE window *tft_setsize(uint16_t size);
MEMSPACE void tft_putch(int c, window *win);

/// @brief tft_printf function
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] size: font index (index into font array)
/// @param[in] fmt: printf forat string
/// @param[in] ...: vararg list or arguments
/// @return size of string
MEMSPACE
int tft_printf( int x, int y, int size, const char *fmt, ... )
{
	_fontc f;
	int ret;
	char *s;
	window *win;
    char buff[512];

    va_list va;
    va_start(va, fmt);
    ret = t_vsnprintf(buff, 510, fmt, va);
    va_end(va);

	win = tft_setpos(x,y);
	win = tft_set_fontsize(size);
	s = buff;
	while(*s)
	{
		tft_putch(*s++, win);
	}
}
