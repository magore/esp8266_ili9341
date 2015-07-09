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
uint16_t tft_ID;

/// =============================================================
/// SPI 

/// @brief  Obtain SPI bus for TFT display, assert chip select
/// return: void
void tft_spi_init(uint16_t prescale)
{
    hspi_waitReady();
	hspi_init(prescale, 0);
}

/// @brief  Obtain SPI bus for TFT display, assert chip select
/// return: void
void tft_spi_begin()
{
    hspi_waitReady();
    TFT_CS_ACTIVE;
}

/// @brief  Release SPI bus from TFT display, deassert chip select
/// return: void
void tft_spi_end()
{
    hspi_waitReady();
    TFT_CS_DEACTIVE;
}

/// @brief  Transmit 8 bit data array
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// return: void 
void tft_spi_TX(uint8_t *data, int bytes, uint8_t command)
{
	hspi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	hspi_TX(data,bytes);
}

/// @brief  Transmit and read 8 bit data array 
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// return: void 
void tft_spi_TXRX(uint8_t * data, int bytes, uint8_t command)
{
	hspi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	hspi_TXRX(data,bytes);
}


/// @brief  read 8 bit data array 
/// @param[in] *data: data buffer to send 
/// @param[in] bytes: data buffer size
/// return: void 
void tft_spi_RX(uint8_t *data, int bytes, uint8_t command)
{
	hspi_waitReady();
	if(command)
		TFT_COMMAND;
	else
		TFT_DATA;
	hspi_RX(data,bytes);
}

/// @brief  Transmit 8 bit command 
/// @param[in] cmd: display command
/// return: void status is in data array - bytes in size
void tft_Cmd(uint8_t cmd)
{
	hspi_waitReady();
	TFT_COMMAND;
    hspi_TX(&cmd, 1);
}

/// @brief  Transmit 8 bit command and send/receive data buffer
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
/// =============================================================
/// =============================================================
/// =============================================================
/// @brief Initialize TFT
/// @return diplay ID 9341
MEMSPACE
window *tft_init(void)
{

    TFT_CS_INIT;
    TFT_INIT;
    TFT_RST_INIT;
    TFT_RST_ACTIVE;

	// start with slow SPI, no hardware CS
	tft_spi_init(2);

    os_delay_us(10000);
    TFT_RST_DEACTIVE;
    os_delay_us(1000);

	/* Adafruit 9341 TFT Display Initialization */
    tft_configRegister();

	/* Read the TFT ID value */
    tft_ID = tft_readId();

	// fast SPI
	tft_spi_init(1);

	/* Setup the master window */
    tft_window_init(tft, TFT_XOFF, TFT_YOFF, TFT_W, TFT_H);
    tft_setRotation(0);
    tft_fillWin(tft, tft->bg);

    return (tft);
}

/// ====================================
/// @brief window limits
/// ====================================

/// @brief Set the ili9341 working window by absolute position and size
/// @param[in] x: Starting X offset
/// @param[in] y: Starting Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @return  w * h after clipping
int32_t tft_abs_window(int16_t x, int16_t y, int16_t w, int16_t h)
{
    uint8_t tmp[4];

	uint16_t xl,yl;

	int16_t ww,hh;
	int32_t bytes;

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
/// @param[in] win*: window structure
/// @param[in] x: Starting X offset
/// @param[in] y: Starting Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @return  bytes w * h after clipping, 0 on error
int32_t tft_rel_window(window *win, int16_t x, int16_t y, int16_t w, int16_t h)
{
	return( tft_abs_window(x+win->xoff, y+win->yoff, w,h) );
}

///  ======================================================================
/// SPI




/// ====================================

/// @brief  Read parameters on SPI bus ILI9341 displays when EXTC is not asserted
/// However; This undocumented command overrides the restriction for reading command parameters.
/// SPI configurations
/// See M:[0-3] control bits in ILI9341 documenation
///      - refer to interface I and II
///      modes. Most ILI9341 displays are using interface I
///      rather the interface II mode.
/// @param[in] command: command whose parameters we want to read 
/// @param[in] parameter: parameter number
/// @return 16bit value
MEMSPACE
uint32_t tft_readRegister(uint8_t command, uint8_t parameter)
{
    uint32_t result;
	uint8_t tmp[4];


	tft_spi_begin();

	// We do not know what the 0x10 offset implies - but it is required.
	// ILI9341 parameter
    tft_Cmd(0xd9);

	// DATA
	tmp[0] = 0x10 + parameter;
	tmp[1] = 0;
	tmp[2] = 0;
	tmp[3] = 0;
	// Undocumented 0xd9 command
    tft_spi_TX(tmp,4,0);

	// The real ILI9341 Command whose parameters we want to read
	// COMMAND
	tmp[0] = command;	
	tmp[1] = 0;
	tmp[2] = 0;
	tmp[3] = 0;
    tft_spi_TXRX(tmp,4,1);

	tft_spi_end();

	result = tmp[1];

#if ILI9341_DEBUG & 1
ets_uart_printf("command:%02x, data: %02x,%02x,%02x,%02x\r\n", 
	0xff & command, 
	0xff & tmp[0],
	0xff & tmp[1],
	0xff & tmp[2],
	0xff & tmp[3]);
#endif

    return (result);
}


/// @brief  Read ILI9341 device ID should be 9341
/// This does not work for really high SPI clock speeds
/// @return 32bit value
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
void tft_bit_blit(window *win, uint8_t *ptr, int x, int y, int w, int h)
{

    uint16_t color;
    int xx,yy;
    int32_t off;
	int32_t pixels;
	int wdcount;
	int ind;
	uint8_t buf[2*64];

    if(!w || !h)
        return;

#if 1
// BIT BLIT

// TODO CLIP window 
// Note: Clipping modifies offsets which in turn modifies blit array offsets
// We could use tft_drawPixel, and it clips - but that is slow

    pixels = tft_rel_window(win, x, y, w, h);

	
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
			ets_wdt_disable();
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
    int32_t pixels;
    int32_t colors;
	int wdcount;
    int ind;
    uint8_t buf[2*64];

	pixels = tft_rel_window(win, x,y,w,h);

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
				ets_wdt_disable();
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
	uint16_t xx, yy;

// Clip pixel
    if(x < 0 || x >= win->w)
        return;
    if(y < 0 || y >= win->h)
        return;

    tft_rel_window(win, x,y,1,1);

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
/// TODO CLIP window - depends on blit array also
void tft_writeRect(window *win, int16_t x, int16_t y, int16_t w, int16_t h, uint16_t *color)
{

	int32_t pixels;
	uint16 pixel;
	int wdcount;
    int xx,yy;
	int ind;
	uint8_t buf[2*64];

    if(!w || !h)
        return;

// TODO CLIP window - depends on blit array offset also
// We could use tft_drawPixel, and it clips - but that is slow

#if 1
    pixels = tft_rel_window(win, x, y, w, h);

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
				ets_wdt_disable();
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
			ets_wdt_disable();
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

	// set window
    pixels = tft_rel_window(win, x,y,w,h);

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
			ets_wdt_disable();
			wdcount = 0;
		}
	}
}


/// @brief Scroll window up by dir lines
/// @param[in] win*: window structure
/// @param[in] dir: direction and count
/// TODO +/- scroll direction
void tft_Vscroll(window *win, int dir)
{
	int i;
	uint8_t buff[TFT_W*3];
	if(dir >= win->h)
		dir = win->h-1;
	if(dir <= 0)
		return;

	for(i=0;i<win->h-dir;++i)
	{
		tft_readRect(win, 0, i+dir, win->w, 1, (uint16_t *) buff);
		tft_writeRect(win, 0, i, win->w, 1, (uint16_t *)buff);

	}
	tft_fillRectWH(win, 0, win->h-1-dir, win->w, dir, win->bg);
	win->y -= dir;
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
    uint8_t data[5];
    uint16_t color;
	// set window

    tft_rel_window(win, x,y,1,1);

	tft_spi_begin();
	
    tft_Cmd(0x2e);
    tft_Cmd(0);		// NOP
    tft_spi_TXRX(data, 5, 0);

	tft_spi_end();

    color = tft_RGBto565(data[2],data[3],data[4]);
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
    win->flags     = WRAP_H;
    win->w 		   = w;
    win->xoff      = xoff;
    win->h 		   = h;
    win->yoff      = yoff;
    win->rotation  = 0;
    win->tabstop   = w/4;
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
	int ret, rem;
    _fontc f;
	ret = font_attr(win,' ', &f);
	if(ret < 0)
		return;
	rem = (win->w - 1 - win->x);
	if(rem > 0)
	{
		tft_fillRectWH(win, win->x, win->y, rem, f.Height, win->bg);
	}
	win->x = win->w;	// one past end
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
		win->x = 0;
		win->y += f.Height;
	}
	if(c == '\f')
	{
		tft_fillWin(win,win->bg);
		win->x = 0;
		win->y = 0;
	}
	if(c == '\t')
	{
		count = win->tabstop - (win->x % win->tabstop);// skip size to next tabstop
		// Will we overflow ?
		if(win->x + count > win->w)
		{
			count = (win->x + count) - win->w -1;
			tft_cleareol(win);
			if(win->flags & WRAP_H)
			{
				tft_fillRectWH(win, 0, win->y, count, f.Height, win->bg);
				win->x = count;
				win->y += f.Height;
			}
		}
		else
		{
			tft_fillRectWH(win, win->x, win->y, count, f.Height, win->bg);
			win->x += count;
		}
	}
}


/* tft_printf removes the need for most of the draw string functions */
