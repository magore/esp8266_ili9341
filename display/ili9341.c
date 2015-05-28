/**
 @file ili9341.c

 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief Adafruit ili9341 code mostly rewritten by Mike Gore
 @par Copyright (c) 2013 Adafruit Industries.  All rights reserved.
 @see https://github.com/adafruit/Adafruit-GFX-Library

 @par Line drawing function CERTS
 @see https://github.com/CHERTS/esp8266-devkit/tree/master/Espressif/examples/esp8266_ili9341

  Line drawing function from

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option)
any later version.

bdffont2c is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <mem.h>
#include "hspi.h"
#include "font.h"
#include "util.h"
#include "ili9341_adafruit.h"

window win;

/// @brief Initialize TFT
/// @ return diplay ID 9341
MEMSPACE
uint32_t tft_init(void)
{
    uint32_t ID;

    hspi_init();

    window_init();

    TFT_CS_INIT;
    TFT_INIT;
    TFT_RST_INIT;
    TFT_RST_ACTIVE;

    os_delay_us(10000);
    TFT_RST_DEACTIVE;
    os_delay_us(1000);

    ID = tft_readId();

    tft_configRegister();

    tft_setRotation(0);

    tft_fillRectXY(win.min_x,win.min_y,win.max_x,win.max_y, win.textbgcolor);
    return ( ID );
}



/// @brief Constrain window to display window
/// @param[in] *xs: Starting X offset
/// @param[in] *xl: Ending X offset
/// @param[in] *ys: Starting Y offset
/// @param[in] *yl: Ending X offset
/// @return  size of clipped window: height * width
/// FIXME if the entire zoe is outside the display we must return ZERO

uint32_t tft_clip(int16_t *xs, int16_t *xl, int16_t *ys, int16_t *yl)
{
    uint32_t bytes;

// Make sure starting X offset is <= ending offset, else swap
    if(*xs > *xl)
        SWAP(*xs,*xl);
// Is the window totally off screen
    if(*xs > win.max_x || *xl < win.min_x)
        return(0);

// Make sure starting Y offset is <= ending offset, else swap
    if(*ys > *yl)
        SWAP(*ys,*yl);
// Is the window totally off screen
    if(*ys > win.max_y || *yl < win.min_y)
        return(0);

//Constrain xs,xl,ys,yl to current window
    CONSTRAIN(*xs,win.min_x, win.max_x);
    CONSTRAIN(*xl,win.min_x, win.max_x);
    CONSTRAIN(*ys,win.min_y, win.max_y);
    CONSTRAIN(*yl,win.min_y, win.max_y);
    bytes = (*xl - *xs + 1) * (*yl - *ys + 1);
    return(bytes);
}


/// @brief Set the ili9341 update window
/// @param[in] x: Starting X offset
/// @param[in] y: Starting Y offset
/// @param[in] xl: Ending X offset
/// @param[in] yl: Ending Y offset
/// @return  void
void tft_window(int16_t x, int16_t y, int16_t xl, int16_t yl)
{
    uint8_t tmp[4];

    tmp[0] = x >> 8;
    tmp[1] = x & 0xff;
    tmp[2] = xl >> 8;
    tmp[3] = xl & 0xff;
    tft_writeCmdData(0x2A, tmp, 4);
    tmp[0] = y >> 8;
    tmp[1] = y & 0xff;
    tmp[2] = yl >> 8;
    tmp[3] = yl & 0xff;
    tft_writeCmdData(0x2B, tmp, 4);
}


///  ====================================

/// @brief  Transmit 8 bit display command
/// @param [in] cmd: command code
/// return: void
void tft_writeCmd(uint8_t cmd)
{
// Do not change Command/Data control until SPI bus clear
    hspi_waitReady();
    TFT_COMMAND;
    TFT_CS_ACTIVE;
    hspi_Tx(&cmd, 1);
    TFT_CS_DEACTIVE;
}


/// @brief  Transmit 8 bit display data
/// @param [in] data: data
/// return: void
void tft_writeData(uint8_t data)
{
// Do not change Command/Data control until SPI bus clear
    hspi_waitReady();
    TFT_DATA;
    TFT_CS_ACTIVE;
    hspi_Tx(&data, 1);
    TFT_CS_DEACTIVE;
}


/// @brief  Transmit 8 bit command and related data buffer
/// @param [in] cmd: display command
/// @param [in] *data: data buffer to send after command
/// @param [in] bytes: data buffer size
/// return: void status is in data array - bytes in size
void tft_writeCmdData(uint8_t cmd, uint8_t * data, uint8_t bytes)
{
// FIXME can we insert the command into data buffer and do both at once ?

// Do not change Command/Data control until SPI bus clear
    hspi_waitReady();
    TFT_CS_ACTIVE;
    TFT_COMMAND;
    hspi_Tx(&cmd, 1);

// Read result
    if (bytes > 0)
    {
// Do not change Command/Data control until SPI bus clear
        hspi_waitReady();
        TFT_DATA;
        hspi_TxRx(data,bytes);
//hspi_Tx(data, bytes, 1);
    }
    TFT_CS_DEACTIVE;
}


/// @brief  Transmit 16 bit data
/// ILI9341 defaults to MSB/LSB data so we have to reverse it
/// @param [in] val: 16 bit data
/// return void
void tft_writeData16(uint16_t val)
{
    uint8_t data[2];

// Do not change Command/Data control until SPI bus clear
    hspi_waitReady();
    TFT_DATA;
    TFT_CS_ACTIVE;
    data[0] = val >>8;
    data[1] = val;
    hspi_Tx(data, 2);
    TFT_CS_DEACTIVE;
}


/// @brief  Buffered color fill
/// Optimized for 16bit MSB/LSB SPI bus tramission
/// ILI9341 defaults to MSB/LSB color data so we reverse it
/// @param [in] color: 16 bit color
/// @param [in] count: repeat count
/// @return void
void tft_writeColor16Repeat(uint16 color, uint32_t count)
{
    if (!count)
        return;

    hspi_stream_init();
    TFT_DATA;

//We are sending words

    while(count--)
    {
        hspi_stream(color >> 8);
        hspi_stream(color & 0xff);
    }
    hspi_stream_flush();

}


/// @brief  Write 16bit color data buffered
/// Optimized for 16bit MSB/LSB SPI bus tramission
/// ILI9341 defaults to MSB/LSB color data so we reverse it
/// Warning: exceeding the array bounds may crash certain systems
/// @param [in] *color_data: 16 bit data array
/// @param [in] count: count of 16bit values to write
/// @return void
void tft_writeDataBuffered(uint16_t *color_data, uint32_t count)
{
    uint32_t d_ind = 0;                           // data index, size of write_data (bytes)
    uint16_t color;

    if (!count)
        return;                                   // Error parameter

// Do not change Command/Data control until SPI bus clear
    hspi_stream_init();
    TFT_DATA;

//We are sending words

    while(count--)
    {
// MSB/LSB byte swap
        color = color_data[d_ind++];
        hspi_stream(color >> 8);
        hspi_stream(color & 0xff);
    }
    hspi_stream_flush();
}


/// @brief  Read ILI9341 16bit register value
/// Special undocumented way to read registers on 4 wire
/// SPI configurations
/// See M:[0-3] control bits in ILI9341 documenation
///      - refer to interface I and II
///      modes. Most ILI9341 displays are using interface I
///      rather the interface II mode.
/// It appears that direct reading has been disabled on most
/// SPI displays.
/// @param [in] reg: register to read
/// @param [in] parameter: parameter number to read
/// @return 16bit value
uint32_t tft_readRegister(uint8_t reg, uint8_t parameter)
{
    uint8_t data;
	uint8_t tmp[4];

	tmp[0] = 0x10 + parameter;
    tft_writeCmdData(0xd9,tmp,1);
    hspi_waitReady();
    TFT_COMMAND;
	tmp[0] = reg;
	tmp[1] = 0;
    hspi_TxRx(tmp,2);
    ets_uart_printf("data: %02x,%02x,%02x,%02x\r\n", tmp[0],tmp[1],tmp[2],tmp[3]);
	data = tmp[1];
    return data;
}


/// @brief  Read ILI9341 device ID should be 9341
/// @return 32bit value
MEMSPACE
uint32_t tft_readId(void)
{
    uint32_t_bytes id;

    id.all = 0;
    id.bytes.b2 = tft_readRegister(0xd3, 1);
    id.bytes.b1 = tft_readRegister(0xd3, 2);
    id.bytes.b0 = tft_readRegister(0xd3, 3);

    return  id.all;
}


/// ====================================
/// @brief Set soft window limits
/// ====================================

/// @brief Set Display rotation
/// Set hardware display rotation and memory fill option bits
/// Update software display limits
/// FIXME Work in progress
/// - do we want to handle exchanging min/max values - rather then set them ?
/// @return void
MEMSPACE
void tft_setRotation(uint8_t m)
{

    uint8_t data;
    win.rotation = m % 4;                         // can't be higher than 3
    data = MADCTL_BGR;
    switch (win.rotation)
    {
        case 0:
            data |= MADCTL_MX;
            win.min_x  = MIN_TFT_X;
            win.max_x  = MAX_TFT_X;
            win.min_y = MIN_TFT_Y;
            win.max_y = MAX_TFT_Y;
            break;
        case 1:
            data |= MADCTL_MV;
            win.max_x  = MAX_TFT_Y;
            win.min_x  = MIN_TFT_Y;
            win.min_y  = MIN_TFT_X;
            win.max_y  = MAX_TFT_X;
            break;
        case 2:
            data |= MADCTL_MY;
            win.min_x  = MIN_TFT_X;
            win.max_x  = MAX_TFT_X;
            win.min_y = MIN_TFT_Y;
            win.max_y = MAX_TFT_Y;
            break;
        case 3:
            data = MADCTL_MX | MADCTL_MY | MADCTL_MV;
            win.min_x  = MIN_TFT_Y;
            win.max_x  = MAX_TFT_Y;
            win.min_y = MIN_TFT_X;
            win.max_y = MAX_TFT_X;
            break;
    }
    tft_writeCmdData(ILI9341_MADCTL, &data, 1);
}


/// @brief Defines software values for current window size
/// These are not hardware limits
/// @return  void
MEMSPACE
void window_init()
{
    win.x         = 0;                            // current X
    win.y         = 0;                            // current Y
    win.fontindex = 0;                            // current font size
    win.fontfixed     = 0;
    win.min_x     = MIN_TFT_X;
    win.max_x     = MAX_TFT_X;
    win.min_y     = MIN_TFT_Y;
    win.max_y     = MAX_TFT_Y;
    win.rotation  = 0;
    win.wrap      = true;
    win.tabcolor  = 0;
    win.textcolor = 0xFFFF;
    win.textbgcolor = 0;
}


/// ====================================

/// @brief  Set text forground and background color
/// @param [in] c: forground color
/// @param [in] b: background color
/// @return void
MEMSPACE
void tft_setTextColor(uint16_t c, uint16_t b)
{
    win.textcolor   = c;
    win.textbgcolor = b;
}


/// @brief  Set current window offset
/// (per current rotation)
/// @param [in] x: x offset
/// @param [in] y: y oofset
/// return: pointer to current windows
MEMSPACE
window *tft_setpos(int16_t x, int16_t y)
{
    win.x = x;
    win.y = y;
    return (window *) &win;
}


/// @brief  Set current font size
/// (per current rotation)
/// @param [in] index: font index (for array of fonts)
/// return: pointer to current windows
MEMSPACE
window *tft_set_fontsize(uint16_t index)
{
    win.fontindex = index;
    return (window *) &win;
}


/// @brief  Set current font type to fixed
/// Only works if font is proportional type
/// return: pointer to current windows
MEMSPACE
window *tft_font_fixed()
{
    win.fontfixed = 1;
    return (window *) &win;
}


/// @brief  Set current font type to variable
/// return: pointer to current windows
MEMSPACE
window *tft_font_var()
{
    win.fontfixed = 0;
    return (window *) &win;
}


/// @brief  Return current window pointer
/// return: pointer to current windows
MEMSPACE
window *tft_getwin()
{
    return (window *) &win;
}


/// @brief  Return window height
/// return: pointer to current windows
MEMSPACE
int16_t tft_height(void)
{
    return win.max_y;
}


/// @brief  Return window width
/// return: pointer to current windows
MEMSPACE
int16_t tft_width(void)
{
    return win.max_x;
}


/// @brief  Return window rotation
/// return: window rotation
MEMSPACE
uint8_t tft_getRotation(void)
{
    return win.rotation;
}


/// ====================================
/// @brief Color conversions
/// ====================================

/// @brief  Pass 8-bit (each) R,G,B, get back 16-bit packed color
/// ILI9341 defaults to MSB/LSB data so we have to reverse it
/// @param [in] r: red data
/// @param [in] b: blue data
/// @param [in] g: green data
MEMSPACE
uint16_t tft_color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >>3);
}


/// @brief  Convert 16bit colr into 8-bit (each) R,G,B
/// ILI9341 defaults to MSB/LSB data so we have to reverse it
/// @param [in] color: 16bit color
/// @param [out] *r: red data
/// @param [out] *b: blue data
/// @param [out] *g: green data
MEMSPACE
void convert565toRGB(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = ((0xf800 & color)>>8);
    *g = ((0x7e0 & color)>>3);
    *b = (0x1f & color) << 3;
}


/// @brief  Invert the display
/// @param [in] flag: true or false
/// @return void
MEMSPACE
void tft_invertDisplay(int flag)
{
    uint8_t cmd = flag ? ILI9341_INVON : ILI9341_INVOFF;
    tft_writeCmd(cmd);
}


/// ====================================
/// @brief BLIT functions
/// ====================================

/// @brief  BLIT a bit array to the display
/// @param[in] *ptr: bit array w * h in size
/// @param[in] x: BLITX offset
/// @param[in] y: BLIT Y offset
/// @param[in] w: BLIT Width
/// @param[in] h: BLIT Height
/// @param[in] fg: BLIT forground color
/// @param[in] bg: BLIT background color
/// @return  void
/// TODO CLIP window - depends on blit array
void tft_bit_blit(uint8_t *ptr, int x, int y, int w, int h, uint16_t fg, uint16_t bg)
{

    uint16_t color;
    int off;
    int xx,yy;

    if(!w || !h)
        return;

#if 1
// BIT BLIT

// TODO CLIP window - depends on blit array offset also
// We could use tft_drawPixel, and it clips - but that is slow
// We use hspi_stream to make it FAST

    tft_window(x, y, x+w-1, y+h-1);
    tft_writeCmd(0x2c);

    hspi_stream_init();
    TFT_DATA;

    off = 0;
    for (yy=0; yy < h; ++yy)
    {
        for (xx=0;xx < w; ++xx)
        {
            if(bittestv(ptr, xx + off))
                color = fg;
            else
                color = bg;
            hspi_stream(color >> 8);
            hspi_stream(color & 0xff);
        }
        off += w;
    }
    hspi_stream_flush();
#else
    off = 0;
    for (yy=0; yy < h; ++yy)
    {
        for (xx=0;xx < w; ++xx)
        {
            if(bittestv(ptr, xx + off))
                tft_drawPixel(x+xx,y+yy,fg);
            else
                tft_drawPixel(x+xx,y+yy,bg);
        }
        off += w;
    }
#endif
}


/// @brief  BLIT a color array to the display
/// @param[in] *ptr: color array w * h in size
/// @param[in] x: BLITX offset
/// @param[in] y: BLIT Y offset
/// @param[in] w: BLIT Width
/// @param[in] h: BLIT Height
/// @return  void
/// TODO CLIP window - depends on blit array
void tft_blit(uint16_t *ptr, int x, int y, int w, int h)
{

    uint16_t color;
    int off;
    int xx,yy;

    if(!w || !h)
        return;

// TODO CLIP window - depends on blit array offset also
// We could use tft_drawPixel, and it clips - but that is slow
// We use hspi_stream to make it FAST

#if 1
// BLIT
    tft_window(x, y, x+w-1, y+h-1);
    tft_writeCmd(0x2c);

    hspi_stream_init();
    TFT_DATA;

    off = 0;
    for (yy=0; yy < h; ++yy)
    {
        for (xx=0;xx < w; ++xx)
        {
            color = *ptr++;
            hspi_stream(color >> 8);
            hspi_stream(color & 0xff);
        }
        off += w;
    }
    hspi_stream_flush();
#else
    off = 0;
    for (yy=0; yy < h; ++yy)
    {
        for (xx=0;xx < w; ++xx)
        {
            tft_drawPixel(x+xx,y+yy,*ptr++);
        }
        off += w;
    }
#endif
}


/// ====================================
/// @brief Pixel functions
/// ====================================

/// @brief Draw one pixel set to color
/// We clip the window to the current view
/// @param[in] x: X Start
/// @param[in] y: Y STart
/// @param[in] color: color to set
/// @return void
void tft_drawPixel(int16_t x, int16_t y, int16_t color)
{
    uint8_t data[2];
// Clip pixel
    if(x < win.min_x || x > win.max_x)
        return;
    if(y < win.min_y || y > win.max_y)
        return;

    tft_window(x, y, x, y);

#ifdef JUNK
    tft_writeCmd(0x2c);
    tft_writeData16(color);
#else
    data[0] = color >>8;
    data[1] = color;
    tft_writeCmdData(0x2c, data, 2);
#endif

}

/// @brief Draw line
/// From my blit test code testit.c 1984 - 1985 Mike Gore
/// @param[in] x0: X Start
/// @param[in] y0: Y STart
/// @param[in] x1: X End
/// @param[in] y1: Y End
/// @param[in] color: color to set
/// @return void
#if 0
void tft_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	int npts, i, delx, dely, xi, yi, diff;

	delx = (x1-x0); xi = SIGN(delx); delx = ABS(delx);
	dely = (y1-y0); yi = SIGN(dely); dely = ABS(dely);
	diff = (delx-dely);
	npts = MAX(delx,dely);

	for(i=0;i<=npts;++i) 
	{
		tft_drawPixel(x0, y0, color);
		if(diff >= 0) 
		{
			diff -= dely; x0 += xi;
			if(dely >= delx) 
			{
				diff +=delx; y0 +=yi;
			}
		}
		else 
		{
			diff += delx; y0 += yi;
			if(delx >= dely) 
			{
				diff -= dely; x0 += xi;
			}
		}
	}
}

#else
/// @brief Draw line
/// Draw line from CERTS
/// https://github.com/CHERTS/esp8266-devkit/tree/master/Espressif/examples/esp8266_ili9341
/// @param[in] x0: X Start
/// @param[in] y0: Y STart
/// @param[in] x1: X End
/// @param[in] y1: Y End
/// @param[in] color: color to set
/// @return void
void tft_drawLine(int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{

#define USE_OPTIMIZATION_DRAWLINE

    int16_t dx = ABS(x1 - x0);
    int16_t dy = -ABS(y1 - y0);
    int8_t sx = (x0 < x1) ? 1 : -1;
    int8_t sy = (y0 < y1) ? 1 : -1;
    int16_t err = dx + dy;
    int16_t e2 = 0;

#ifdef USE_OPTIMIZATION_DRAWLINE
    uint16_t startX = x0;
    uint16_t startY = y0;
#endif

    for (;;)
    {
#ifdef USE_OPTIMIZATION_DRAWLINE
            // Require correction
            if ((startX != x0) && (startY != y0)) // draw line and not draw point
            {
                tft_fillRectXY(startX, startY, x0 - sx, y0 - sy, color);
                startX = x0;
                startY = y0;
            }
#else
            tft_drawPixel(x0, y0, color);
#endif

        e2 = 2*err;
        if (e2 >= dy)
        {
            if (x0 == x1) break;
            err += dy;
            x0 += sx;
        }
        if (e2 <= dx)
        {
            if (y0 == y1) break;
            err += dx;
            y0 += sy;
        }
    }
#ifdef USE_OPTIMIZATION_DRAWLINE
    tft_fillRectXY(startX, startY, x0, y0, color);
#endif
}
#endif


/// ====================================
/// @brief Fill functions
/// ====================================

/// @brief  Fill rectangle with color
/// We clip the window to the current view
/// @param[in] x: X Start
/// @param[in] y: Y Start
/// @param[in] w: Width
/// @param[in] h: Height
/// @param[in] color: Fill color
/// @return void
void tft_fillRectWH(int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    uint32_t repeat;
    int16_t xl = x + w - 1;
    int16_t yl = y + h - 1;

    repeat = tft_clip(&x,&xl,&y,&yl);
    if(repeat)
    {
        tft_window(x,y,xl,yl);
        tft_writeCmd(0x2c);
// Send one *colors value a total of count times.
        tft_writeColor16Repeat(color, repeat);
    }
}

/// @brief  Fill rectangle with color
/// We clip the window to the current view
/// @param[in] x: X Start
/// @param[in] y: Y Start
/// @param[in] xl: X End
/// @param[in] yl: Y End
/// @param[in] color: Fill color
/// @return void
void tft_fillRectXY(int16_t x, int16_t y, int16_t xl, int16_t yl, uint16_t color)
{
    uint32_t repeat;

    repeat = tft_clip(&x,&xl,&y,&yl);
    if(repeat)
    {
        tft_window(x,y,xl,yl);
        tft_writeCmd(0x2c);
// Send one *colors value a total of count times.
        tft_writeColor16Repeat(color, repeat);
    }
}

/// @brief Fill current display window specified by win
/// @brief Fill current display window specified by win
/// @param[in] color: Collor to fill
/// @return void
void tft_fillWin(uint16_t color)
{
	int16_t w = win.max_x - win.min_x + 1;
	int16_t h = win.max_y - win.min_y + 1;
    tft_fillRectXY(win.min_x,win.min_y,win.max_x,win.max_y, win.textbgcolor);
}

///  ====================================
/// @brief Character and String functions
///  ====================================

/// @brief  Clear display to end of line
/// return: void
MEMSPACE
void tft_cleareol()
{
    int x = win.x;
    int flag = win.wrap;
    win.wrap = 0;
    while(x < win.max_x)
        tft_putch(' ', &win);
    win.x = 0;
    win.wrap = flag;
}


/// @brief  put character in current winoow
/// @param [in] c: character
/// @param [in] *win: window pointer
/// return: void
MEMSPACE
void tft_putch(int c, window *win)
{
    _fontc f;
    int ret;
    int width;
    int count;

    ret = font_attr(c, &f, win->fontindex);
    if(ret < 0)
        return;

// control characters
    if(c < ' ')
    {
        if(c == '\n')
        {
            win->x = 0;
            win->y += f.Height;
        }
        if(win->y > win->max_y)
        {
            win->y = 0;
        }
        if(c == '\t')
        {
            count = win->x - 1;                   // 0 based
            count &= 3;                           // MOD 4
            count = 4 - count;                    // Number of spaces
            while(count)
                tft_putch(' ', win);
        }
        return;
    }

// if the character will not fix then wrap
    if((win->x + f.w) >= win->max_x)
    {
        if(win->wrap)
        {
            win->y += f.h;
            win->x = 0;
        }
        else
        {
            return;                               // no wrap
        }
    }

    if(win->y > win->max_y)
    {
        if(win->wrap)
        {
            win->y = 0;
        }
        else
        {
            return;                               // no wrap
        }
    }
    (void) tft_drawChar(c, win->x, win->y, win->fontindex);
    win->x += f.gap;
}


/// @brief  print long number
/// @param [in] long_num: number to print
/// @param [in] x: X offset
/// @param [in] y: Y offset
/// @param [in] size: font index (index the main array of fonts )
/// return: void
MEMSPACE
int tft_drawNumber(long long_num,int16_t x, int16_t y, uint8_t size)
{
    tft_printf(x, y, size, "%ld", long_num);
}


/// @brief  Draw String at x,y with size bytes
/// @param[in] *string: String
/// @param[in] x: X Start
/// @param[in] y: Y STart
/// @param[in] size: Size of string
/// @return X position of cursor
MEMSPACE
int tft_drawString(const char *string, int16_t x, int16_t y, uint8_t size)
{
    int16_t sumX = 0;
    int16_t xPlus;

    while(*string)
    {
        xPlus = tft_drawChar(*string, x, y, size);
        sumX += xPlus;
        x += xPlus;                               /* Move cursor right            */
        *string++;
    }
    return sumX;
}


/// @brief  Draw Centered String at x,y with size bytes
/// @param[in] *string: String
/// @param[in] x: X Start
/// @param[in] y: Y STart
/// @param[in] size: Size of string
/// @return X position of cursor
MEMSPACE
int tft_drawCentreString(const char *string, int16_t x, int16_t y, uint8_t size)
{
    int16_t sumX = 0;
    int16_t len = 0;
    int16_t xPlus;
    const char *pointer = string;
    char ascii;
    _fontc f;
    int ret;

    while(*pointer)
    {
        ret = font_attr(*pointer, &f, size);
        if(ret < 0)
            len += 0;
        len += f.gap;
        pointer++;
    }

    int xs = x - len/2;

    if (xs < 0) xs = 0;

    while(*string)
    {
        uint16_t xPlus = tft_drawChar(*string, xs, y, size);
        sumX += xPlus;
        *string++;

// Advance cursor if it does not overflow the line
        if(xs <= win.max_x)
        {
            xs += xPlus;                           /* Move cursor right            */
        }
    }

    return sumX;
}


/// @brief  Draw String Right Justified at x,y with size bytes
/// @param[in] *string: String
/// @param[in] x: X Start
/// @param[in] y: Y STart
/// @param[in] size: Size of string
/// @return X position of cursor
MEMSPACE
int tft_drawRightString(const char *string, int16_t x, int16_t y, uint8_t size)
{
    int sumX = 0;
    int len = 0;
    const char *pointer = string;
    char ascii;
    _fontc f;
    int ret;

    while(*pointer)
    {
        ascii = *pointer;
        ret = font_attr(ascii, &f, size);
        if(ret < 0)
            len += 0;
        len += f.gap;
        pointer++;
    }

    int xs = x - len;

    if (xs < 0) xs = 0;

    while(*string)
    {

        int16_t xPlus = tft_drawChar(*string, xs, y, size);
        sumX += xPlus;
        *string++;

        if(xs <= win.max_x)
        {
            xs += xPlus;                           /* Move cursor right            */
        }
    }

    return sumX;
}
