/**
 @file ili9341.c

 @brief ili9341 driver inspired by Adafruit ili9341 code
        All code in this file has been rewritten by Mike Gore
 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
   please retain a copy of this notice in any code you use it in.

 @par Copyright &copy; 2013 Adafruit Industries.  All rights reserved.
 @see https://github.com/adafruit/Adafruit-GFX-Library

 @par Line drawing function CERTS
 @see https://github.com/CHERTS/esp8266-devkit/tree/master/Espressif/examples/esp8266_ili9341


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

#include <user_config.h>

// TFT master window definition
window tftwin;
window *tft = &tftwin;

/// @brief Initialize TFT
/// @ return diplay ID 9341
MEMSPACE
uint32_t tft_init(void)
{
    uint32_t ID;

    hspi_init();

    tft_window_init(tft, TFT_XOFF, TFT_YOFF, TFT_W, TFT_H);

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

    tft_fillWin(tft, tft->bg);
    return ( ID );
}


/// @brief Set the ili9341 update window
/// @param[in] x: Starting X offset
/// @param[in] y: Starting Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @return  w * h after clipping
uint32_t tft_abs_window(int16_t x, int16_t y, int16_t w, int16_t h)
{
    uint8_t tmp[4];

	uint16_t xl,yl;

	int16_t ww,hh;
	uint32_t bytes;

	// Check for basic out of bounds conditions

	// Width or Height <= 0 ? Bogus range
	if(w <= 0 || h <= 0)
		return(0);

	// X >= Width ??
	if(x >= tft->w)
		return(0);

	// Y >= Height ??
	if(y >= tft->h)
		return(0);

	// Clipping tests for < 0
	// X < 0 ? Clip
	if(x < 0)
		x = 0;
	// Y < 0 ? Clip
	if(y < 0)
		y = 0;

	// Clip check X + W
	ww = (x + w) - tft->w;
	if(ww > 0)
		w -= ww;
	if(w <= 0)
		return(0);

	// Clip check Y + H
	hh = (y + h) - tft->h;
	if(hh > 0)
		h -= hh;
	if(h <= 0)
		return(0);

	// Now We know the result will fit
	xl = x + w - 1;
	yl = y + h - 1;

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

	bytes = w;
	bytes *= h;
	return( bytes);
}


///  ====================================

/// @brief  Transmit 8 bit display command
/// @param[in] cmd: command code
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
/// @param[in] data: data
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
/// @param[in] cmd: display command
/// @param[in] *data: data buffer to send after command
/// @param[in] bytes: data buffer size
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
/// @param[in] val: 16 bit data
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
/// @param[in] color: 16 bit color
/// @param[in] count: repeat count
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
/// @param[in] *color_data: 16 bit data array
/// @param[in] count: count of 16bit values to write
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
/// @param[in] reg: register to read
/// @param[in] parameter: parameter number to read
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
/// @brief Color conversions
/// ====================================

/// @brief  Pass 8-bit (each) R,G,B, get back 16-bit packed color
/// ILI9341 defaults to MSB/LSB data so we have to reverse it
/// @param[in] r: red data
/// @param[in] b: blue data
/// @param[in] g: green data
MEMSPACE
uint16_t tft_color565(uint8_t r, uint8_t g, uint8_t b)
{
    return ((r >> 3) << 11) | ((g >> 2) << 5) | (b >>3);
}


/// @brief  Convert 16bit colr into 8-bit (each) R,G,B
/// ILI9341 defaults to MSB/LSB data so we have to reverse it
/// @param[in] color: 16bit color
/// @param[out] *r: red data
/// @param[out] *b: blue data
/// @param[out] *g: green data
MEMSPACE
void convert565toRGB(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = ((0xf800 & color)>>8);
    *g = ((0x7e0 & color)>>3);
    *b = (0x1f & color) << 3;
}


/// @brief  Invert the display
/// @param[in] flag: true or false
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
/// @param[in] win*: window structure
/// @param[in] *ptr: bit array w * h in size
/// @param[in] x: BLITX offset
/// @param[in] y: BLIT Y offset
/// @param[in] w: BLIT Width
/// @param[in] h: BLIT Height
/// @param[in] fg: BLIT forground color
/// @param[in] bg: BLIT background color
/// @return  void
/// TODO CLIP window - depends on blit array
void tft_bit_blit(window *win, uint8_t *ptr, int x, int y, int w, int h, uint16_t fg, uint16_t bg)
{

    uint16_t color;
    int off;
    int xx,yy;

    if(!w || !h)
        return;

#if 1
// BIT BLIT

// TODO CLIP window 
// Note: Clipping modifies offsets which in turn modifies blit array offsets
// We could use tft_drawPixel, and it clips - but that is slow
// We use hspi_stream to make it FAST

    tft_rel_window(win, x, y, w, h);
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
/// @param[in] win*: window structure
/// @param[in] *ptr: color array w * h in size
/// @param[in] x: BLITX offset
/// @param[in] y: BLIT Y offset
/// @param[in] w: BLIT Width
/// @param[in] h: BLIT Height
/// @return  void
/// TODO CLIP window - depends on blit array
void tft_blit(window *win, uint16_t *ptr, int x, int y, int w, int h)
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
    tft_rel_window(win, x, y, w, h);
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
            tft_drawPixel(win, x+xx,y+yy,*ptr++);
        }
        off += w;
    }
#endif
}



/// ====================================
/// @brief Fill functions
/// ====================================

/// @brief Fill window 
/// @param[in] win*: window structure
/// @param[in] color: Fill color
void tft_fillWin(window *win, uint16_t color)
{
    tft_fillRectWH(win, 0,0, win->w, win->h, color);
}

/// @brief  Partial window Fill with color
/// We clip the window to the current view
/// @param[in] win*: window structure
/// @param[in] x: X Start
/// @param[in] y: Y Start
/// @param[in] w: Width
/// @param[in] h: Height
/// @param[in] color: Fill color
/// @return void
void tft_fillRectWH(window *win, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t color)
{
    uint32_t repeat;

//ets_uart_printf("x:%d,y:%d,w:%d,h:%d\n",x,y,w,h);

//ets_uart_printf("repeat: %d\n",repeat);

	repeat = tft_rel_window(win, x,y,w,h);
    if(repeat)
    {
        tft_writeCmd(0x2c);
// Send one *colors value a total of count times.
        tft_writeColor16Repeat(color, repeat);
    }
}

/// @brief  Fill rectangle with color
/// We clip the window to the current view
/// @param[in] win*: window structure
/// @param[in] x: X Start
/// @param[in] y: Y Start
/// @param[in] xl: X End
/// @param[in] yl: Y End
/// @param[in] color: Fill color
/// @return void
void tft_fillRectXY(window *win, int16_t x, int16_t y, int16_t xl, int16_t yl, uint16_t color)
{
    uint32_t repeat;
	uint16_t w,h;

    if(x > xl)
        SWAP(x,xl);
    if(y > yl)
        SWAP(y,yl);

	w = xl - x + 1;
	h = yl - y + 1;
	tft_fillRectWH(win, x, y, w, h, color);
}

/// ====================================
/// ====================================
/// ====================================
/// @brief Set soft window limits
/// ====================================
/// ====================================
/// ====================================

/// @brief Set the ili9341 update window
/// @param[in] win*: window structure
/// @param[in] x: Starting X offset
/// @param[in] y: Starting Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @return  bytes w * h after clipping, 0 on error
uint32_t tft_rel_window(window *win, int16_t x, int16_t y, int16_t w, int16_t h)
{
	return( tft_abs_window(x+win->xoff, y+win->yoff, w,h) );
}



/// @brief Set Display rotation, applies to the master window only
/// Set hardware display rotation and memory fill option bits
/// Update software display limits
/// FIXME Work in progress
/// - do we want to handle exchanging min/max values - rather then set them ?
/// @return void
MEMSPACE
void tft_setRotation(uint8_t m)
{

    uint8_t data;
    tft->rotation = m & 3; // can't be higher than 3
    data = MADCTL_BGR;
    switch (tft->rotation)
    {
        case 0:
            data 		|= 	MADCTL_MX;
            tft->w 	= 	TFT_W;
            tft->xoff = 	TFT_XOFF;
            tft->h 	= 	TFT_H;
            tft->yoff = 	TFT_YOFF;
            break;
        case 1:
            data 		|= 	MADCTL_MV;
            tft->w 	= 	TFT_H;
            tft->xoff	= 	TFT_YOFF;
            tft->h 	= 	TFT_W;
            tft->yoff	= 	TFT_XOFF;
            break;
        case 2:
            data 		|= 	MADCTL_MY;
            tft->w 	= 	TFT_W;
            tft->xoff	= 	TFT_XOFF;
            tft->h 	= 	TFT_H;
            tft->yoff = 	TFT_YOFF;
            break;
        case 3:
            data = MADCTL_MX | MADCTL_MY | MADCTL_MV;
            tft->w 	= 	TFT_H;
            tft->xoff = 	TFT_YOFF;
            tft->h 	= 	TFT_W;
            tft->yoff = 	TFT_XOFF;
            break;
    }
    tft_writeCmdData(ILI9341_MADCTL, &data, 1);
}


/// @brief Initialize window structure we default values
/// FIXME check x+w, y+h absolute limits against TFT limuts
/// @param[in] win*: window structure
/// @param[in] xoff: X offset to window start - absolute
/// @param[in] yoff: Y offset to window start - absolute
/// @param[in] w: Window width
/// @param[in] h: Window Height
/// @return  void
MEMSPACE
void tft_window_init(window *win, uint16_t xoff, uint16_t yoff, uint16_t w, uint16_t h)
{
    win->x         = 0;                            // current X
    win->y         = 0;                            // current Y
    win->font      = 0;                            // current font size
    win->fixed     = 0;
    win->wrap      = true;
    win->w 		   = w;
    win->xoff      = xoff;
    win->h 		   = h;
    win->yoff      = yoff;
    win->rotation  = 0;
    win->fg = 0xFFFF;
    win->bg = 0;
}


/// ====================================

/// @brief  Set text forground and background color
/// @param[in] win*: window structure
/// @param[in] c: forground color
/// @param[in] b: background color
/// @return void
MEMSPACE
void tft_setTextColor(window *win,uint16_t c, uint16_t b)
{
    win->fg = c;
    win->bg = b;
}

/// @brief  Set current window offset
/// (per current rotation)
/// @param[in] win*: window structure
/// @param[in] x: x offset
/// @param[in] y: y oofset
/// return: void
MEMSPACE
void tft_setpos(window *win, int16_t x, int16_t y)
{
    win->x = x;
    win->y = y;
}

/// @brief  Set current font size
/// (per current rotation)
/// @param[in] win*: window structure
/// @param[in] index: font index (for array of fonts)
/// return: void
MEMSPACE
tft_set_font(window *win, uint16_t index)
{
    win->font = index;
}

/// @brief  Get font height
/// @param[in] win*: window structure
/// return: font Height or zero on error
MEMSPACE
int tft_get_font_height(window *win)
{
	int ret;
    _fontc f;
    ret = font_attr(win, ' ', &f);
    if(ret < 0)
        return 0;
	return(f.Height);
}


/// Only works if font is proportional type
/// @param[in] win*: window structure
/// return: void
MEMSPACE
tft_font_fixed(window *win)
{
    win->fixed = 1;
}


/// @brief  Set current font type to variable
/// @param[in] win*: window structure
/// return: void
MEMSPACE
void tft_font_var(window *win)
{
    win->fixed = 0;
}


/// ====================================
/// @brief Pixel functions
/// ====================================

/// @brief Draw one pixel set to color
/// We clip the window to the current view
/// @param[in] win*: window structure
/// @param[in] x: X Start
/// @param[in] y: Y STart
/// @param[in] color: color to set
/// @return void
void tft_drawPixel(window *win, int16_t x, int16_t y, int16_t color)
{
    uint8_t data[2];
	uint16_t xx, yy;

// Clip pixel
    if(x < 0 || x >= win->w)
        return;
    if(y < 0 || y >= win->h)
        return;

    tft_rel_window(win, x,y,1,1);

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
/// @param[in] win*: window structure
/// @param[in] x0: X Start
/// @param[in] y0: Y STart
/// @param[in] x1: X End
/// @param[in] y1: Y End
/// @param[in] color: color to set
/// @return void
#if 0
void tft_drawLine(window *win, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
{
	int npts, i, delx, dely, xi, yi, diff;

	delx = (x1-x0); xi = SIGN(delx); delx = ABS(delx);
	dely = (y1-y0); yi = SIGN(dely); dely = ABS(dely);
	diff = (delx-dely);
	npts = MAX(delx,dely);

	for(i=0;i<=npts;++i) 
	{
		tft_drawPixel(win, x0, y0, color);
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
/// @param[in] win*: window structure
/// @param[in] x0: X Start
/// @param[in] y0: Y STart
/// @param[in] x1: X End
/// @param[in] y1: Y End
/// @param[in] color: color to set
/// @return void
void tft_drawLine(window *win, int16_t x0, int16_t y0, int16_t x1, int16_t y1, uint16_t color)
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
                tft_fillRectXY(win, startX, startY, x0 - sx, y0 - sy, color);
                startX = x0;
                startY = y0;
            }
#else
            tft_drawPixel(win, x0, y0, color);
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
    tft_fillRectXY(win, startX, startY, x0, y0, color);
#endif
}
#endif

///  ====================================
/// @brief Character and String functions
///  ====================================

/// @brief  Clear display to end of line
/// @param[in] win*: window structure
/// return: void
MEMSPACE
void tft_cleareol(window *win)
{
    int x = win->x;
    int flag = win->wrap;
    win->wrap = 0;
    while(x < win->w)
        tft_putch(win,' ');
    win->x = 0;
    win->wrap = flag;
}


/// @brief  put character in current winoow
/// @param[in] win*: window structure
/// @param[in] c: character
/// return: void
MEMSPACE
void tft_putch(window *win, int c)
{
    _fontc f;
    int ret;
    int width;
    int count;

// control characters
    if(c < ' ')
    {
        if(c == '\n' && win->wrap)
        {
            win->x = 0;
            win->y += f.Height;
        }
        if(win->y >= (win->h))
        {
            win->y = 0;
        }
        if(c == '\t')
        {
            count = win->x - 1;                   // 0 based
            count &= 3;                           // MOD 4
            count = 4 - count;                    // Number of spaces
            while(count--)
                tft_putch(win,' ');
        }
        return;
    }
	else 
	{
		ret = font_attr(win, c, &f);
		if(ret < 0)
			return;
	}

// if the character will not fix then wrap
    if((win->x + f.w) >= win->w)
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

    if(win->y >= win->h)
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
    (void) tft_drawChar(win, c);
    win->x += f.gap;
}


/* tft_prinf removes the need for most of the draw string functions */
