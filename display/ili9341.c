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


#include "user_config.h"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "display/font.h"
#include "display/ili9341.h"
#include "3rd_party/ili9341_adafruit.h"

// TFT master window definition
window tftwin;
window *tft = &tftwin;

/// =============================================================
/// =============================================================

/// @brief  Transmit 8 bit command 
/// @param[in] cmd: display command
/// return: void status is in data array - bytes in size
void tft_Cmd(uint8_t cmd)
{
	tft_spi_TX(&cmd, 1, 1);
}

/// @brief  Transmit 8 bit data amd read 8bit data
/// @param[in] data: display command
/// return: read result
uint8_t tft_Data(uint8_t data)
{
	tft_spi_TXRX(&data, 1, 0);
	return(data);
}

/// @brief  Transmit 8 bit command and optionally send data buffer
/// @param[in] cmd: display command
/// @param[in] *data: data buffer to send after command
/// @param[in] bytes: data buffer size
/// return: void status is in data array - bytes in size
void tft_Cmd_Data_TX(uint8_t cmd, uint8_t * data, int bytes)
{
// Do not change Command/Data control until SPI bus clear
	tft_spi_begin();

    tft_Cmd(cmd);

// Read result
    if (bytes > 0)
    {
		tft_spi_TX(data,bytes,0);
    }

	tft_spi_end();
}

/// =============================================================

/// ====================================
/// @brief window limits
/// ====================================

/// @brief Set the ili9341 working window by absolute position and size
/// Note: Function clips x,y,w,y
/// @param[in] x: Starting X offset
/// @param[in] y: Starting Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @return  (w * h) after clipping
int32_t tft_abs_window(window *win, int16_t x, int16_t y, int16_t w, int16_t h)
{
    uint8_t tmp[4];

	int16_t xl,yl;
	int32_t bytes;

	// FIXME do we want to return or use the constrained values ????
	if(tft_window_clip_args(win,&x,&y,&w,&h) )
		return(0);

	// Now We know the result will fit
	xl = x + w - 1;
	yl = y + h - 1;

    tmp[0] = x >> 8;
    tmp[1] = x & 0xff;
    tmp[2] = xl >> 8;
    tmp[3] = xl & 0xff;
	
    tft_Cmd_Data_TX(0x2A, tmp, 4);

    tmp[0] = y >> 8;
    tmp[1] = y & 0xff;
    tmp[2] = yl >> 8;
    tmp[3] = yl & 0xff;

    tft_Cmd_Data_TX(0x2B, tmp, 4);

	bytes = w;
	bytes *= h;
	return( bytes);
}

/// @brief Set the ili9341 working window by relative position and size
/// Note: Function clips x,y,w,y
/// @param[in] win*: window structure
/// @param[in] x: Starting X offset
/// @param[in] y: Starting Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @return  bytes w * h after clipping, 0 on error
int32_t tft_rel_window(window *win, int16_t x, int16_t y, int16_t w, int16_t h)
{
	// function tft_abs_window clips
	return( tft_abs_window(win,x+win->x, y+win->y, w,h) );
}

///  ======================================================================
/// SPI


/// ====================================

/// @brief  Read parameters on SPI bus ILI9341 displays 
/// For those displays that do not have the EXTC pin asserted.
/// This undocumented command overrides the restriction for reading 
/// command parameters.
/// Notes:
/// SPI configurations
/// See M:[0-3] control bits in ILI9341 documenation
///      - Refer to interface I and II modes. 
///        Most ILI9341 displays are using interface I,
///        (Rather the interface II mode)
/// @param[in] command: command whose parameters we want to read 
/// @param[in] parameter: parameter number
/// @return 16bit value
MEMSPACE
uint32_t tft_readRegister(uint8_t command, uint8_t parameter)
{
    uint32_t result;

	tft_spi_begin();

	// Send Undocumented ILI9341 0xd9 as COMMAND
    tft_Cmd(0xd9);

	// We do not know what the 0x10 offset implies, undocumented, but is required.
	// Send ILI9341 parameter as DATA
	tft_Data(0x10 + parameter);

	// The real ILI9341 Command whose parameters we want to read
	// Send ILI9341 command as COMMAND
	tft_Cmd(command);

	// Read Result
	result=tft_Data(0);

	tft_spi_end();


#if ILI9341_DEBUG & 1
ets_uart_printf("cmd:%02x, par:%02x, read: %02x\n",
	0xff & command, 
	0xff & parameter, 
	0xff & result);
#endif

    return (result);
}


/// @brief  Read ILI9341 device ID should be 9341
/// This does not work for really high SPI clock speeds
/// Make sure that when called the clock speed in reduced.
/// @see tft_init
/// @return 32bit value with DIPLAY ID
MEMSPACE
uint32_t tft_readId(void)
{
    uint32_t_bytes id;


    id.all = 0;
	/// Paramter 0 is unused
	/// See Read ID4 Command ( 0xd3 ) for a description of the parameters
    id.bytes.b2 = tft_readRegister(0xd3, 1);	// Parameter 1
    id.bytes.b1 = tft_readRegister(0xd3, 2);	// Parameter 2
    id.bytes.b0 = tft_readRegister(0xd3, 3);	// Parameter 3

    return  id.all;
}


/// @brief BLIT functions
/// ====================================

/// @brief  BLIT a bit array to the display
/// @param[in] win*: window structure
/// @param[in] *ptr: bit array w * h in size
/// @param[in] x: BLITX offset
/// @param[in] y: BLIT Y offset
/// @param[in] w: BLIT Width
/// @param[in] h: BLIT Height
/// @return  void
/// TODO CLIP window - depends on blit array
void tft_bit_blit(window *win, uint8_t *ptr, int16_t x, int16_t y, int16_t w, int16_t h)
{

    uint16_t color;
    int xx,yy;
    int32_t off;
	int32_t pixels;
	int wdcount;
	int ind;
	uint8_t buf[2*64];

	// FIXME - do we just want to constrain the values or ignore the request ???
	if ( tft_window_clip_args(tft,&x,&y,&w,&h) )
		return;

    if(!w || !h)
        return;

#if 1
// BIT BLIT

    pixels = tft_rel_window(win, x, y, w, h);
	if(pixels == 0)
		return;

// FIXME now we should consider clipping the pixel array
// Note: Clipping modifies offsets which in turn modifies blit array offsets
// We could use tft_drawPixel, and it clips - but that the pixel function is very slow
// For now we clip the arguments and use tft_rel_windows which also clips

	
	tft_spi_begin();

    tft_Cmd(0x2c);

    off = 0;
	ind = 0;
	wdcount = 0;

    for (yy=0; yy < h; ++yy)
    {
        for (xx=0;xx < w; ++xx)
        {
            if(bittestv(ptr, xx + off))
                color = win->fg;
            else
                color = win->bg;

			buf[ind++] = color >> 8;
			buf[ind++] = color & 0xff;

			if(ind >= sizeof(buf))
			{
				tft_spi_TX(buf,ind,0);
				ind = 0;
			}
        }
		wdcount += xx;
		if(wdcount > 0x3ff)
		{
			optimistic_yield(1000);
			//ets_wdt_disable();
			wdcount = 0;
		}
        off += w;
    }
	if(ind)
	{
		tft_spi_TX(buf,ind,0);
	}

	tft_spi_end();
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

// =============================================================
// =============================================================



/// ====================================
/// @brief Fill functions
/// ====================================

/// @brief Fill window 
/// @param[in] win*: window structure
/// @param[in] color: Fill color
void tft_fillWin(window *win, uint16_t color)
{
    // tft_fillRectWH() clips
    tft_fillRectWH(win, 0,0, win->w, win->h, color);
}

/// @brief Flood fill
/// @see https://en.wikipedia.org/wiki/Flood_fill
/// This method can eat all your ram with "busy" images and could crash
/// @param[in] win*: window structure
/// @param[in] border: border color
/// @param[in] fill: Fill color
void tft_flood(window *win, int16_t x, int16_t y, uint16_t border, uint16_t fill)
{
	uint16_t current;

	if(x < 0 || x >= win->w)
		return;
	if(y < 0 || y >= win->h)
		return;

	current = tft_readPixel(win,x,y);
	if(current == border || current == fill)
		return;

	tft_drawPixel(win,x,y,fill);
	tft_flood(win,x+1,y,border,fill);
	tft_flood(win,x,y+1,border,fill);
	tft_flood(win,x-1,y,border,fill);
	tft_flood(win,x,y-1,border,fill);
}



/// @brief X,Y point stack
#define XYSTACK 64
static struct {
	int16_t x[XYSTACK+1];
	int16_t y[XYSTACK+1];
	int ind;
} xy;

/// @brief point push
/// @param[in] x: X
/// @param[in] y: Y
/// @ return true on success, 0 of stack overflow
int tft_push_xy(int16_t x, int16_t y)
{
	if(xy.ind >= XYSTACK)
	{
		printf("xy.ind stack >= %d\n",XYSTACK);
		return(0);
	}
	xy.x[xy.ind] = x;
	xy.y[xy.ind] = y;
	xy.ind++;
	return(1);
}

/// @brief point push
/// @param[in] x: X
/// @param[in] y: Y
/// @ return true if data exists, 0 of none
int tft_pop_xy(int16_t *x, int16_t *y)
{
	if(xy.ind <= 0)
		return(0);
	xy.ind--;
	*x = xy.x[xy.ind];
	*y = xy.y[xy.ind];
	return(1);
}

/// @brief Flood using line fill method
/// @see https://en.wikipedia.org/wiki/Flood_fill
/// FIXME: not tested yet
/// @param[in] win*: window structure
/// @param[in] x: X position to fill from
/// @param[in] y: Y position to fill from
/// @param[in] border: border color
/// @param[in] fill: Fill color
/// @ return true one success, 0 of stack overflow
int tft_floodline(window *win, int16_t x, int16_t y, uint16_t border, uint16_t fill)
{
	int lineAbove, lineBelow;
	int16_t  xoff;

	// reset stack
	xy.ind = 0;

	// test for stack full
	if(!tft_push_xy(x, y)) 
		return(0);

	// while we have stacked items
	// FIXME we can bypass readpixel - read any entire line interruped by the color check
	while(tft_pop_xy(&x, &y))
	{
		if(x < 0 || x >= win->w)
			continue;
		if(y < 0 || y >= win->h)
			continue;
		
		xoff = x;
		while(xoff >= 0 && tft_readPixel(win,xoff,y) != border ) 
			xoff--;
		xoff++;
		lineAbove = lineBelow = 0;

		while(xoff < win->w && tft_readPixel(win,xoff,y) != border)
		{
			tft_drawPixel(win,xoff,y,fill);

			if(!lineAbove && y > 0 && tft_readPixel(win,xoff,y-1) != border)
			{
				// test for stack full
				if(!tft_push_xy(xoff, y - 1)) 
					return(0);
				lineAbove = 1;
			}
			else if(lineAbove && y > 0 && tft_readPixel(win,xoff,y-1) == border )
			{
				lineAbove = 0;
			}
			if(!lineBelow && y < (win->h-1) && tft_readPixel(win,xoff,y+1) != border )
			{
				// test for stack full
				if(!tft_push_xy(xoff, y + 1)) 
					return(0);
				lineBelow = 1;
			}
			else if(lineBelow && y < (win->h-1) && tft_readPixel(win,xoff,y+1) == border)
			{
				lineBelow = 0;
			}
			xoff++;
		}
	}
	return(1);
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
    int32_t pixels;
    int32_t colors;
	int wdcount;
    int ind;
    uint8_t buf[2*64];

	// tft_rel_window clips
	pixels = tft_rel_window(win, x,y,w,h);
	if(!pixels)
		return;

	wdcount = 0;

    if(pixels > 0)
    {
		tft_spi_begin();

        tft_Cmd(0x2c);

		ind = 0;
		//We are sending words
		while(pixels > 0)
		{
			colors = pixels;
			if(colors > sizeof(buf)/2)
				colors = sizeof(buf)/2;

			pixels -= colors;
			wdcount += colors;

			if(!ind)
			{
				while(ind < sizeof(buf) && ind < colors*2)
				{
					buf[ind++] = color >> 8;
					buf[ind++] = color & 0xff;
				}
			}
			tft_spi_TX(buf,colors*2,0);
			if(wdcount > 0x3ff)
			{
				wdcount = 0;
				optimistic_yield(1000);
				// ets_wdt_disable();
			}
		}

		tft_spi_end();
    } // if(pixels > 0)

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
	int16_t w,h;

    if(x > xl)
        SWAP(x,xl);
    if(y > yl)
        SWAP(y,yl);

	w = xl - x + 1;
	h = yl - y + 1;
	// tft_fillRectWH() clips
	tft_fillRectWH(win, x, y, w, h, color);
}

/// ====================================
/// @brief Pixel functions
/// ====================================

/// @brief Draw one pixel set to color in 16bit 565 RGB format
/// We clip the window to the current view
/// @param[in] win*: window structure
/// @param[in] x: X Start
/// @param[in] y: Y Start
/// @param[in] color: color to set
/// @return void
void tft_drawPixel(window *win, int16_t x, int16_t y, int16_t color)
{
    uint8_t data[2];

// Clip pixel
    if(x < 0 || x >= win->w)
        return;
    if(y < 0 || y >= win->h)
        return;

	// tft_rel_window() clips
    if(! tft_rel_window(win, x,y,1,1))
		return;

    data[0] = color >>8;
    data[1] = color;
    tft_Cmd_Data_TX(0x2c, data, 2);

}

/// @brief  Write a rectangle pixel array
/// @param[in] win*: window structure
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @param[in] *color: pixel array in 565 format
/// @return  void
void tft_writeRect(window *win, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *color)
{

	int32_t pixels;
	uint16_t pixel;
	int wdcount;
    int xx,yy;
	int ind;
	uint8_t buf[2*64];

    if(!w || !h)
        return;

// TODO CLIP window data - depends on blit array offset also
// We could use tft_drawPixel, and it clips - but that is slow

#if 1
    // tft_rel_window() clips limits
    pixels = tft_rel_window(win, x, y, w, h);
	if(!pixels)
		return;

	tft_spi_begin();

    tft_Cmd(0x2c);

	ind = 0;
	wdcount = 0;

	while(pixels-- > 0)
	{
		pixel = *color++;
		buf[ind++]=pixel >> 8;
		buf[ind++]=pixel & 0xff;
		if(ind >= sizeof(buf))
		{
			tft_spi_TX(buf,ind,0);
			wdcount += ind;
			ind = 0;
			if(wdcount > 0x3ff)
			{
				wdcount = 0;
				optimistic_yield(1000);
				//ets_wdt_disable();
			}
		}
	}

	if(ind)
	{
		tft_spi_TX(buf,ind,0);
	}

	tft_spi_end();

#else
	wdcount = 0;
    for (yy=0; yy < h; ++yy)
    {
        for (xx=0;xx < w; ++xx)
        {
            tft_drawPixel(win, x+xx,y+yy,*ptr++);
        }
		wdcount += w;
		if(wdcount >= 0x3ff)
		{
			wdcount = 0;
			optimistic_yield(1000);
			// ets_wdt_disable();
		}
    }
#endif
}


/// @brief Read Rectangle and return 16bit color array in 565 RGB format
/// We clip the window to the current view
/// Note: TFT Chip Select must be asserted for each block read
///       As soon as chip select goes high the read aborts!
/// We break up the data into chunks with a Memory Read, followed by Memory Read Continue
/// So we use Memory Read and Memory Read Continue
/// @param[in] win*: window structure
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @param[in] *color: pixel array in 565 format

/// @brief Number of pixels FIFO can hold
///  We also include the 2 byte Memory Read / Continue opcodes;
#define HSPI_PIX ((HSPI_FIFO_SIZE-2)/3)

void tft_readRect(window *win, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *color)
{
	int32_t pixels;
	int wdcount;
	int rem;
	uint8_t cmd;
	uint8_t *ptr;
	// Command and Pixel buffer
	uint8_t data[64*3];

	// tft_rel_window() clips
    pixels = tft_rel_window(win, x,y,w,h);
	if(!pixels)
		return;

	cmd = 0x2e;	// Memory Read

	wdcount = 0;
	while(pixels > 0)
	{
		tft_spi_begin();

		tft_Cmd(cmd);
		tft_Cmd(0);	// NOP

		if(pixels > sizeof(data)/3)
			rem = sizeof(data)/3;
		else
			rem = pixels;

		pixels -= rem;

		wdcount += rem;

		// Send/Receive
		// Read 3 byte pixel data
		tft_spi_RX(data, rem*3,1);

		tft_spi_end();

		// Now reconvert 3 byte color data into 2 byte color data
		ptr = data;
		while(rem--)
		{
			// reuse the color buffer
			// overwrite the 3 byte/pixel with 2 byte/pixle only data
			*color++ = tft_RGBto565(ptr[0],ptr[1],ptr[2]);
			ptr += 3;
		}
		cmd = 0x3e; // Memory Read Continue

		if(wdcount >= 0x3ff)
		{
			optimistic_yield(1000);
			// ets_wdt_disable();
			wdcount = 0;
		}
	}
}

/// @brief Scroll window up by dir lines
/// We start at the top of the window and move down
/// @param[in] win*: window structure
/// @param[in] dir: direction and count
/// TODO +/- scroll direction
/// TODO +/- Horizontal scroll functions
void tft_Vscroll(window *win, int dir)
{
	int i;
	int yfrom,yto;
	uint8_t buff[TFT_W*3];

	// FIXME unsupported
	// + == normal scroll - start at the top move down
	// 0 == nothing to do
	// - == reverse scroll - start at the bottom and move up
	if(dir <= 0)
		return;

	if(dir >= win->h)
	{
		// clear area that is vacated
		tft_fillRectWH(win, 0, 0, win->w, win->h, win->bg);
		return;
	}
	for(i=0; i < win->h;++i)
	{
		// source of scroll
		yfrom= i+dir;
		// target of scroll
		yto = i;

		if(yfrom >= (win->h-1))
		{
			// Clear to of window
			tft_fillRectWH(win, 0, yto, win->w, 1, win->bg);
		}
		else
		{
			// TOP
			tft_readRect(win, 0, yfrom, win->w, 1, (uint16_t *) buff);
			// DOWN
			tft_writeRect(win, 0, yto, win->w, 1, (uint16_t *)buff);
		}
	}
}

/// @brief Read one pixel and return color in 1bit 565 RGB format
/// We clip the window to the current view
/// Note: Read Memory must be done in a continious write/read operation
/// Chip select can not be deactivated for the transaction
/// @param[in] win*: window structure
/// @param[in] x: X Start
/// @param[in] y: Y Start
/// @return color in 565 format
uint16_t tft_readPixel(window *win, int16_t x, int16_t y)
{
    uint8_t data[3];
    uint16_t color;

	// set window, also clips
    if(!tft_rel_window(win, x,y,1,1))
		return(0);

	tft_spi_begin();
	
    tft_Cmd(0x2e);
    tft_Cmd(0);		// NOP
    tft_spi_RX(data, 3, 1);

	tft_spi_end();
	
    color = tft_RGBto565(data[0],data[1],data[2]);
    return(color);
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
            tft->x = 	TFT_XOFF;
            tft->h 	= 	TFT_H;
            tft->y = 	TFT_YOFF;
            break;
        case 1:
            data 		|= 	MADCTL_MV;
            tft->w 	= 	TFT_H;
            tft->x	= 	TFT_YOFF;
            tft->h 	= 	TFT_W;
            tft->y	= 	TFT_XOFF;
            break;
        case 2:
            data 		|= 	MADCTL_MY;
            tft->w 	= 	TFT_W;
            tft->x	= 	TFT_XOFF;
            tft->h 	= 	TFT_H;
            tft->y = 	TFT_YOFF;
            break;
        case 3:
            data = MADCTL_MX | MADCTL_MY | MADCTL_MV;
            tft->w 	= 	TFT_H;
            tft->x = 	TFT_YOFF;
            tft->h 	= 	TFT_W;
            tft->y = 	TFT_XOFF;
            break;
    }

    tft_Cmd_Data_TX(ILI9341_MADCTL, &data, 1);
}

/// ====================================
/// @brief Color conversions
/// ====================================

/// @brief  Convert 16bit colr into 8-bit (each) R,G,B
/// ILI9341 defaults to MSB/LSB data so we have to reverse it
/// @param[in] color: 16bit color
/// @param[out] *r: red data
/// @param[out] *b: blue data
/// @param[out] *g: green data
void tft_565toRGB(uint16_t color, uint8_t *r, uint8_t *g, uint8_t *b)
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

	tft_spi_begin();
    tft_Cmd(cmd);
	tft_spi_end();
}

/// @brief Clip window structure to TFT limits
/// @param[in] win*: window structure
/// @return 0 - or - count of clipped values
/// FIXME check negitive offsets, etc
MEMSPACE
int tft_window_clip(window *win)
{
	int clipped = 0;

	// Clip X
	if(win->x < tft->x)
	{
		win->x = tft->x;
		clipped++;
	}
	if(win->x > (tft->x + tft->w - 1))
	{
		win->x = (tft->x + tft->w - 1);
		clipped++;
	}

	// CLIP X,Y first
	// CLIP Y
	if(win->y < tft->y)
	{
		win->y = tft->y;
		clipped++;
	}
	if(win->y > (tft->y + tft->h - 1))
	{
		win->y = (tft->y + tft->h - 1);
		clipped++;
	}


	// CLIP W,H last
	// CLIP W
	if( (win->x + win->w - 1 ) > (tft->x + tft->w - 1) )
	{
		win->w = (tft->x + tft->w ) - win->x;
		clipped++;
	}

	// CLIP H
	if( (win->y + win->h - 1 ) > (tft->y + tft->h - 1) )
	{
		win->h = (tft->y + tft->h ) - win->y;
		clipped++;
	}
	return(clipped);
}

/**
  @brief  Clip X,Y to fix inside specifiied window
  @param[in] *win: window structure
  @param[in] *X: X position in window
  @param[in] *Y: Y position is window
  return: void
*/
MEMSPACE
void tft_clip_xy(window *win, int16_t *X, int16_t *Y)
{

    int16_t X1 = *X;
    int16_t Y1 = *Y;

    if(X1 < win->x)
        X1 = win->x;
    if(X1 > (win->x + win->w - 1))
        X1 = (win->x + win->w - 1);

    if(Y1 < win->y)
        Y1 = win->y;
    if(Y1 > (win->y + win->w - 1))
        Y1 = (win->y + win->w - 1);
    *X = X1;
    *Y = Y1;
}



/// @brief clip arguments to window limits
/// Arguments position x,y width w and height h to be clipped
/// @param[in] *win: window structure we will use to clip arguments to.
/// @param[in] *x: X argument offset
/// @param[in] *y: Y argument offset
/// @param[in] *w: W argument width
/// @param[in] *h: H argument height
/// @return  void
/// FIXME check negitive offsets, etc
MEMSPACE
int tft_window_clip_args(window *win, int16_t *x, int16_t *y, int16_t *w, int16_t *h)
{
	int clipped = 0;

	// Clip X
	if(*x < win->x)
	{
		*x = win->x;
		clipped++;
	}
	if(*x > (win->x + win->w - 1))
	{
		*x = (win->x + win->w - 1);
		clipped++;
	}

	// CLIP Y
	if(*y < win->y)
	{
		*y = win->y;
		clipped++;
	}
	if(*y > (win->y + win->h - 1))
	{
		*y = (win->y + win->h - 1);
		clipped++;
	}

	// CLIP W
	if( (*x + *w - 1 ) > (win->x + win->w - 1) )
	{
		*w = (win->x + win->w ) - *x;
		clipped++;
	}

	// CLIP H
	if( (*y + *h - 1 ) > (win->y + win->h - 1) )
	{
		*h = (win->y + win->h ) - *y;
		clipped++;
	}
	return(clipped);
}

/// @brief Initialize window structure we default values
/// FIXME check x+w, y+h absolute limits against TFT limuts
/// @param[in] win*: window structure
/// @param[in] x: X offset to window start - absolute
/// @param[in] y: Y offset to window start - absolute
/// @param[in] w: Window width
/// @param[in] h: Window Height
/// @return  void
MEMSPACE
void tft_window_init(window *win, int16_t x, int16_t y, int16_t w, int16_t h)
{
	// Do Clipping checks for bogus values


	(void) tft_window_clip_args(tft,&x,&y,&w,&h);

    win->xpos		= 0;                            // current X position
    win->ypos		= 0;                            // current Y position
    win->font		= 0;                            // current font size
    win->flags     	= WRAP_H;
    win->w 		   	= w;
    win->x		   	= x;
    win->h 		   	= h;
    win->y      	= y;
    win->rotation  	= 0;
    win->tabstop   	= w/4;
    win->fg = 0xFFFF;
    win->bg = 0;
}


/// ====================================

/// @brief  Set text forground and background color
/// @param[in] win*: window structure
/// @param[in] fg: forground color
/// @param[in] bg: background color
/// @return void
MEMSPACE
void tft_setTextColor(window *win,uint16_t fg, uint16_t bg)
{
    win->fg = fg;
    win->bg = bg;
}

/// @brief  Set current window text pointer in pixels
/// (per current rotation)
/// @param[in] win*: window structure
/// @param[in] x: x offset
/// @param[in] y: y oofset
/// return: void
MEMSPACE
void tft_setpos(window *win, int16_t x, int16_t y)
{
    win->xpos = x;
    win->ypos = y;
}

/// @brief  Set current window text pointer in characters
/// (per current rotation) - overall font bounding box
/// @param[in] win*: window structure
/// @param[in] x: x offset
/// @param[in] y: y oofset
/// return: void
MEMSPACE
void tft_set_textpos(window *win, int16_t x, int16_t y)
{
    win->xpos = x * font_W(win->font);
    win->ypos = y * font_H(win->font);
}


/// @brief  Set current font size
/// (per current rotation)
/// @param[in] win*: window structure
/// @param[in] index: font index (for array of fonts)
/// return: void
MEMSPACE
void tft_set_font(window *win, uint16_t index)
{
	if(win->font > font_max())
		win->font = font_max();
    win->font = index;
}

/// Only works if font is proportional type
/// @param[in] win*: window structure
/// return: void
MEMSPACE
void tft_font_fixed(window *win)
{
    win->flags &= ~FONT_VAR;
}


/// @brief  Set current font type to variable
/// @param[in] win*: window structure
/// return: void
MEMSPACE
void tft_font_var(window *win)
{
    win->flags |= FONT_VAR;
}

/// @brief  Get font height
/// @param[in] win*: window structure
/// return: font Height or zero on error
int tft_get_font_height(window *win)
{
	int ret;
    _fontc f;
    ret = font_attr(win, ' ', &f);
    if(ret < 0)
        return 0;
	return(f.Height);
}


/// @brief  Fast virtical line drawing
/// @param[in] *win: window structure
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] h: Height of line
/// @param[in] color: Color
/// @return  void
void tft_drawFastVLine(window *win,int16_t x, int16_t y,
int16_t h, uint16_t color)
{
	// function clips
	tft_fillRectWH(win, x,y ,1, h, color);
}


/// @brief  Fast virtical line drawing
/// @param[in] *win: window structure
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] w: Width of line
/// @param[in] color: Color
/// @return  void
void tft_drawFastHLine(window *win, int16_t x, int16_t y,
int16_t w, uint16_t color)
{
	// function clips
	tft_fillRectWH(win, x,y ,w, 1, color);
}


/// @brief Draw line
/// From my blit test code testit.c 1984 - 1985 Mike Gore
/// @param[in] win*: window structure
/// @param[in] x0: X Start
/// @param[in] y0: Y Start
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
/// @param[in] y0: Y Start
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
	// function clips
    tft_fillRectXY(win, startX, startY, x0, y0, color);
#endif
}
#endif
///  ====================================
/// Interpolation functions
///  ====================================

/**
   @brief Draw lines between points along Quadratic Bézier curve
   Quadratic Bézier with respect to t, see https://en.wikipedia.org/wiki/Bézier_curve
   		B(t) = (1-t)*(1-t)*S + 2*(1-t)*t*C + t*t*T, 0 <= t <= 1
  
   The path traced by the function B(t), given points S, C, and T,
   		S = initial points
   		C = control points
   		T = target points
  
   Derivative of the Bézier curve with respect to t 
        B'(t) = 2*(1-t)*(C-S) + 2*t*(T-C)
  
   Second derivative of the Bézier curve with respect to t is
   		B"(t) = 2*(T-2*C+S)

   As t increases from 0 to 1, the curve departs from S in the direction of C, 
      then bends to arrive at T from the direction of C.
   The tangents to the curve at S and T intersect at C. 

   @param[in] *win: Window Structure of active window
   @param[in] SX: Start X
   @param[in] SY: Start Y
   @param[in] CX: Control X
   @param[in] CY: Control Y
   @param[in] TX: Target X
   @param[in] TY: Target Y
   @param[in] steps: line segments along curve (1..N) 
   @param[in] color: Line color
   @return  void
*/

void tft_Bezier2(window *win, int16_t SX, int16_t SY, int16_t CX, int16_t CY, int16_t TX, int16_t TY, int steps, uint16_t color)
{
	float t, tinc, t1,p1,p2,p3;
	int16_t LX = SX;
	int16_t LY = SY;
	int16_t X,Y;
	int i;

	LX = SX;
	LY = SY;
	t = 0;

	if(steps < 1)
		steps = 1;

	// FXIME we should compute a step size based on the start to end point distances
	tinc = 1.0 / (float) steps;	// steps = 1 will just draw one line

	// Quadratic Bezier http://en.wikipedia.org/wiki/Bézier_curve
	// B(t) = (1-t)(1-t)S + 2(1-t)tC + t*tT, 0 <= t <= 1
	for (i = 0; i < steps; ++i)
	{
		t += tinc;

		t1 = (1.0 - t);
		p1 = t1 * t1;		/* (1.0 - t) (1.0 - t) */
		p2 = 2.0 * t1 * t;  /* 2(1.0 - t) * t */
		p3 = t * t;			/* t * t */

		X = (int16_t) (p1 * (float)SX + p2 * (float)CX + p3 * (float)TX);
		Y = (int16_t) (p1 * (float)SY + p2 * (float)CY + p3 * (float)TY);
		// Do not plot a line until we actually move
		if(LX == X && LY == Y)
			continue;
		tft_drawLine(win, LX, LY, X, Y, color);
		LX = X;
		LY = Y;
	}
}

/**
   @brief Draw lines between points along Cubic Bézier curve
   Quadratic Bézier with respect to t, see https://en.wikipedia.org/wiki/Bézier_curve
   		B(t) = (1-t)(1-t)*(1-t)*S + 3*(1-t)(1-t)*t*C1  + 3(1-t)*t*t*C2 + t*t*t*T, 0 <= t <= 1
  
   The path traced by the function B(t), given points S, C, and T,
   		S = initial points
   		C1 = control point 1
   		C2 = control point 2
   		T = target points
  
   Derivative of the Bézier curve with respect to t 
        B'(t) = 3(1-t)*(1-t)*(C1-S) + 6*(1-t)*t*(C2-C1) + 3*t*t*(T-C2)
  
   Second derivative of the Bézier curve with respect to t is
   		B"(t) = 6*(1-t)*(C2-2*C1+S) + 6*t*(T-2*C2+C1)

  The curve starts at S and moves toward C1 and on to T from the direction of C2. 
  Points C1 and C2 provide directional information and the distance between them determines 
  "how far" and "how fast" the curve moves towards C1 before turning towards C2.


   @param[in] *win: Window Structure of active window
   @param[in] SX: Start X
   @param[in] SY: Start Y
   @param[in] C1X: Control 1 X
   @param[in] C1Y: Control 1 Y
   @param[in] C2X: Control 2 X
   @param[in] C1Y: Control 2 Y
   @param[in] TX: Target X
   @param[in] TY: Target Y
   @param[in] steps: line segments along curve (1..N) 
   @param[in] color: Line color
   @return  void
*/

void tft_Bezier3(window *win, int16_t SX, int16_t SY, int16_t C1X, int16_t C1Y, int16_t C2X, int16_t C2Y, int16_t TX, int16_t TY, int steps, uint16_t color)
{
	float t, tinc, c0, t1,t2,p1,p2,p3,p4;
	int16_t LX = SX;
	int16_t LY = SY;
	int16_t X,Y;
	int i;

	LX = SX;
	LY = SY;
	t = 0;

	if(steps < 1)
		steps = 1;

	// FXIME we should compute a step size based on the start to end point distances
	tinc = 1.0 / (float) steps;	// steps = 1 will just draw one line

	// Quadratic Bezier http://en.wikipedia.org/wiki/Bézier_curve
	// B(t) = (1-t)(1-t)*(1-t)*S + 3*(1-t)(1-t)*t*C1  + 3(1-t)*t*t*C2 + t*t*t*T, 0 <= t <= 1
	for (i = 0; i < steps; ++i)
	{
		t += tinc;
		t1 = (1.0 - t);
		t2 = t1 * t1;		/* (1-t)(1-t) */

		c0 = 3.0 * t1 * t;	/* 3(1-t) * t */

		p1 = t2 * t1;		/* (1-t)(1-t)(1-t) */
		p2 = c0 * t1;       /* 3(1-t)(1-t) * t */
		p3 = c0 * t;		/* 3(1-t) * t * t */
		p4 = t * t * t;		/* t * t * t */

		X = (int16_t) (p1 * (float)SX + p2 * (float)C1X + p3 * (float) C2X + p4 * (float)TX);
		Y = (int16_t) (p3 * (float)SY + p2 * (float)C1Y + p3 * (float) C2Y + p4 * (float)TY);

		// Do not plot a line until we actually move
		if(LX == X && LY == Y)
			continue;
		tft_drawLine(win, LX, LY, X, Y, color);
		LX = X;
		LY = Y;
	}
}


///  ====================================
/// @brief Character and String functions
///  ====================================

/// @brief  Clear display to end of line
/// @param[in] win*: window structure
/// return: void, win->x = win->w
MEMSPACE
void tft_cleareol(window *win)
{
	int ret, rem;
    _fontc f;
	ret = font_attr(win,' ', &f);
	if(ret < 0)
		return;
	rem = (win->w - 1 - win->xpos);
	if(rem > 0)
	{
		// function clips
		tft_fillRectWH(win, win->xpos, win->ypos, rem, f.Height, win->bg);
	}
	win->xpos = win->w;	// one past end
}

/// @brief  Clear display text line 
/// @param[in] win*: window structure
/// return: void, win->x = 0
MEMSPACE
void tft_clearline(window *win)
{
	int ret, rem;
    _fontc f;
	ret = font_attr(win,' ', &f);
	if(ret < 0)
		return;
	rem = (win->w - 1 - win->xpos);
	if(rem > 0)
	{
		// function clips
		tft_fillRectWH(win, 0, win->ypos, rem, f.Height, win->bg);
	}
	win->xpos = 0;
}



/// @brief  put character in current winoow
/// @param[in] win*: window structure
/// @param[in] c: character
/// return: void
void tft_putch(window *win, int c)
{
    _fontc f;
    int ret;
    int width;
    int count;
	int rem;

	if(c < 0 || c > 0x7e)
		return;

	if(c >= ' ')
	{
		ret = font_attr(win, c, &f);
		if(ret < 0)
			return;
	}
	else 
	{
		// use space to get font attributes 
		ret = font_attr(win,' ', &f);
		if(ret < 0)
			return;
	}

	// Normal visible characters
	if(c >= ' ')
	{
		(void) tft_drawChar(win, c);
		return;
	}

	// Control characters
	if(c == '\n')
	{
		tft_cleareol(win);
		win->xpos = 0;
		win->ypos += f.Height;
	}
	if(c == '\f')
	{
		tft_fillWin(win,win->bg);
		win->xpos = 0;
		win->ypos = 0;
	}
	if(c == '\t')
	{
		count = win->tabstop - (win->xpos % win->tabstop);// skip size to next tabstop
		// Will we overflow ?
		if(win->xpos + count > win->w)
		{
			count = (win->xpos + count) - win->w -1;
			tft_cleareol(win);
			if(win->flags & WRAP_H)
			{
				tft_fillRectWH(win, 0, win->ypos, count, f.Height, win->bg);
				win->xpos = count;
				win->ypos += f.Height;
			}
		}
		else
		{
			tft_fillRectWH(win, win->xpos, win->ypos, count, f.Height, win->bg);
			win->xpos += count;
		}
	}
}


/* tft_printf removes the need for most of the draw string functions */
