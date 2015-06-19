/**
 @file ili9341_adafruit.c
 @par Copyright (c) 2013 Adafruit Industries.  All rights reserved.
  Minor Revisions by 2015 Mike Gore to add multiple window support
 @see https://github.com/adafruit/Adafruit-GFX-Library
*/

/**
This is the core graphics library for all our displays, providing a common
set of graphics primitives (points, lines, circles, etc.).  It needs to be
paired with a hardware-specific library for each display device we carry
(to handle the lower-level functions).

Adafruit invests time and resources providing this open source code, please
support Adafruit & open-source hardware by purchasing products from Adafruit!

Copyright (c) 2013 Adafruit Industries.  All rights reserved.

Redistribution and use in source and binary forms, with or without
modification, are permitted provided that the following conditions are met:

- Redistributions of source code must retain the above copyright notice,
this list of conditions and the following disclaimer.
- Redistributions in binary form must reproduce the above copyright notice,
this list of conditions and the following disclaimer in the documentation
and/or other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
POSSIBILITY OF SUCH DAMAGE.
*/

#include <user_config.h>
#include "ili9341_adafruit.h"
#include "ili9341.h"

// ============================================================

/// @brief  Initialize ILI9341 display
/// @return  void
MEMSPACE
void tft_configRegister(void)
{
    uint8_t data[15] = {0};

	
// // POWER_ON_SEQ_CONTROL
    data[0] = 0x39; 
    data[1] = 0x2C;
    data[2] = 0x00;
    data[3] = 0x34;
    data[4] = 0x02;
    tft_Cmd_Data_TX(0xCB, data, 5);

    data[0] = 0x00;
    data[1] = 0XC1;
    data[2] = 0X30;
    tft_Cmd_Data_TX(0xCF, data, 3);

    data[0] = 0x85;
    data[1] = 0x00;
    data[2] = 0x78;
    tft_Cmd_Data_TX(0xE8, data, 3);

    data[0] = 0x00;
    data[1] = 0x00;
    tft_Cmd_Data_TX(0xEA, data, 2);

    data[0] = 0x64;
    data[1] = 0x03;
    data[2] = 0X12;
    data[3] = 0X81;
    tft_Cmd_Data_TX(0xED, data, 4);

    data[0] = 0x20;
    tft_Cmd_Data_TX(0xF7, data, 1);

    data[0] = 0x23;                               //VRH[5:0]
    tft_Cmd_Data_TX(0xC0, data, 1);              //Power control

    data[0] = 0x10;                               //SAP[2:0];BT[3:0]
    tft_Cmd_Data_TX(0xC1, data, 1);              //Power control

    data[0] = 0x3e;                               //Contrast
    data[1] = 0x28;
    tft_Cmd_Data_TX(0xC5, data, 2);              //VCM control

    data[0] = 0x86;                               //--
    tft_Cmd_Data_TX(0xC7, data, 1);              //VCM control2

// MG was 0x40
    data[0] = 0x48;                               // column address order
    tft_Cmd_Data_TX(0x36, data, 1);              // Memory Access Control

    data[0] = 0x55;
    tft_Cmd_Data_TX(0x3A, data, 1);


    data[0] = 0x00;
    data[1] = 0x18;
    tft_Cmd_Data_TX(0xB1, data, 2);

    data[0] = 0x08;
    data[1] = 0x82;
    data[2] = 0x27;
    tft_Cmd_Data_TX(0xB6, data, 3);              // Display Function Control

    data[0] = 0x00;
    tft_Cmd_Data_TX(0xF2, data, 1);              // 3Gamma Function Disable

    data[0] = 0x01;
    tft_Cmd_Data_TX(0x26, data, 1);              //Gamma curve selected

    data[0] = 0x0F;
    data[1] = 0x31;
    data[2] = 0x2B;
    data[3] = 0x0C;
    data[4] = 0x0E;
    data[5] = 0x08;
    data[6] = 0x4E;
    data[7] = 0xF1;
    data[8] = 0x37;
    data[9] = 0x07;
    data[10] = 0x10;
    data[11] = 0x03;
    data[12] = 0x0E;
    data[13] = 0x09;
    data[14] = 0x00;
    tft_Cmd_Data_TX(0xE0, data, 15);             //Set Gamma

    data[0] = 0x00;
    data[1] = 0x0E;
    data[2] = 0x14;
    data[3] = 0x03;
    data[4] = 0x11;
    data[5] = 0x07;
    data[6] = 0x31;
    data[7] = 0xC1;
    data[8] = 0x48;
    data[9] = 0x08;
    data[10] = 0x0F;
    data[11] = 0x0C;
    data[12] = 0x31;
    data[13] = 0x36;
    data[14] = 0x0F;
    tft_Cmd_Data_TX(0xE1, data, 15);             //Set Gamma

    tft_Cmd_Data_TX(0x11, 0, 0);                 //Exit Sleep
	// MG was 120000
    os_delay_us(150000);

    tft_Cmd_Data_TX(0x29, 0, 0);                 //Display on
	// MG added
    os_delay_us(150000);
    tft_Cmd_Data_TX(0x2c, 0, 0);
}


// =======================================================================

/// @brief  Draw bitmap
/// Replaced by tft_writeRect()
/// @param[in] *win: window structure
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] *bitmap: bitmap
/// @param[in] w: Width of bitmap
/// @param[in] h: Height of bitmap
/// @return  void
MEMSPACE
void tft_drawBitmap(window *win, int16_t x, int16_t y,
const uint16_t *bitmap, int16_t w, int16_t h)
{
	tft_writeRect(win, x, y, w, h, (uint16_t *)bitmap);
}


// ====================================================

/// @brief  Draw a circle outline
/// @param[in] *win: window structure
/// @param[in] x0 X offset
/// @param[in] y0: Y offset
/// @param[in] r: Radius of circle
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_drawCircle(window *win, int16_t x0, int16_t y0, int16_t r,
uint16_t color)
{
    int16_t f = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x = 0;
    int16_t y = r;

    tft_drawPixel(win,x0  , y0+r, color);
    tft_drawPixel(win,x0  , y0-r, color);
    tft_drawPixel(win,x0+r, y0  , color);
    tft_drawPixel(win,x0-r, y0  , color);

    while (x<y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f += ddF_y;
        }
        x++;
        ddF_x += 2;
        f += ddF_x;

        tft_drawPixel(win,x0 + x, y0 + y, color);
        tft_drawPixel(win,x0 - x, y0 + y, color);
        tft_drawPixel(win,x0 + x, y0 - y, color);
        tft_drawPixel(win,x0 - x, y0 - y, color);
        tft_drawPixel(win,x0 + y, y0 + x, color);
        tft_drawPixel(win,x0 - y, y0 + x, color);
        tft_drawPixel(win,x0 + y, y0 - x, color);
        tft_drawPixel(win,x0 - y, y0 - x, color);
    }
}


/// @brief  Draw a circle helper
/// @param[in] *win: window structure
/// @param[in] x0: X offset
/// @param[in] y0: Y offset
/// @param[in] r: Radius of circle
/// @param[in] cornername: Corner to draw
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_drawCircleHelper(window *win, int16_t x0, int16_t y0,
int16_t r, uint8_t cornername, uint16_t color)
{
    int16_t f     = 1 - r;
    int16_t ddF_x = 1;
    int16_t ddF_y = -2 * r;
    int16_t x     = 0;
    int16_t y     = r;

    while (x<y)
    {
        if (f >= 0)
        {
            y--;
            ddF_y += 2;
            f     += ddF_y;
        }
        x++;
        ddF_x += 2;
        f     += ddF_x;
        if (cornername & 0x4)
        {
            tft_drawPixel(win,x0 + x, y0 + y, color);
            tft_drawPixel(win,x0 + y, y0 + x, color);
        }
        if (cornername & 0x2)
        {
            tft_drawPixel(win,x0 + x, y0 - y, color);
            tft_drawPixel(win,x0 + y, y0 - x, color);
        }
        if (cornername & 0x8)
        {
            tft_drawPixel(win,x0 - y, y0 + x, color);
            tft_drawPixel(win,x0 - x, y0 + y, color);
        }
        if (cornername & 0x1)
        {
            tft_drawPixel(win,x0 - y, y0 - x, color);
            tft_drawPixel(win,x0 - x, y0 - y, color);
        }
    }
}

/// @brief  Fill circle helper
/// @param[in] *win: window structure
/// @param[in] x0: X offset
/// @param[in] y0: Y offset
/// @param[in] r: Radius of circle
/// @param[in] cornername: Corner to draw
/// @param[in] delta: X or X offset
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_fillCircleHelper(window *win, int16_t x0, int16_t y0, int16_t r,
    uint8_t cornername, int16_t delta, uint16_t color) {

  int16_t f     = 1 - r;
  int16_t ddF_x = 1;
  int16_t ddF_y = -2 * r;
  int16_t x     = 0;
  int16_t y     = r;

  while (x<y) {
    if (f >= 0) {
      y--;
      ddF_y += 2;
      f     += ddF_y;
    }
    x++;
    ddF_x += 2;
    f     += ddF_x;

    if (cornername & 0x1) {
      tft_drawFastVLine(win,x0+x, y0-y, 2*y+1+delta, color);
      tft_drawFastVLine(win,x0+y, y0-x, 2*x+1+delta, color);
    }
    if (cornername & 0x2) {
      tft_drawFastVLine(win,x0-x, y0-y, 2*y+1+delta, color);
      tft_drawFastVLine(win,x0-y, y0-x, 2*x+1+delta, color);
    }
  }
}

/// @brief  Fill circle 
/// @param[in] *win: window structure
/// @param[in] x0: X offset
/// @param[in] y0: Y offset
/// @param[in] r: Radius
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_fillCircle(window *win, int16_t x0, int16_t y0, int16_t r,
uint16_t color)
{
    tft_drawFastVLine(win,x0, y0-r, 2*r+1, color);
    tft_fillCircleHelper(win, x0, y0, r, 3, 0, color);
}


/// @brief  Draw a rectangle
/// @param[in] *win: window structure
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_drawRect(window *win, int16_t x, int16_t y,
int16_t w, int16_t h,
uint16_t color)
{
    tft_drawFastHLine(win,x, y, w, color);
    tft_drawFastHLine(win,x, y+h-1, w, color);
    tft_drawFastVLine(win,x, y, h, color);
    tft_drawFastVLine(win,x+w-1, y, h, color);
}


/// @brief  Draw a rounded rectangle
/// @param[in] *win: window structure
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @param[in] r: Radius 
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_drawRoundRect(window *win, int16_t x, int16_t y, int16_t w,
int16_t h, int16_t r, uint16_t color)
{
// smarter version
    tft_drawFastHLine(win,x+r  , y    , w-2*r, color);// Top
    tft_drawFastHLine(win,x+r  , y+h-1, w-2*r, color);// Bottom
    tft_drawFastVLine(win,x    , y+r  , h-2*r, color);// Left
    tft_drawFastVLine(win,x+w-1, y+r  , h-2*r, color);// Right
// draw four corners
    tft_drawCircleHelper(win,x+r    , y+r    , r, 1, color);
    tft_drawCircleHelper(win,x+w-r-1, y+r    , r, 2, color);
    tft_drawCircleHelper(win,x+w-r-1, y+h-r-1, r, 4, color);
    tft_drawCircleHelper(win,x+r    , y+h-r-1, r, 8, color);
}


/// @brief  Fill a rounded rectangle
/// @param[in] *win: window structure
/// @param[in] x: X offset
/// @param[in] y: Y offset
/// @param[in] w: Width
/// @param[in] h: Height
/// @param[in] r: Radius 
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_fillRoundRect(window *win, int16_t x, int16_t y, int16_t w,
int16_t h, int16_t r, uint16_t color)
{
// smarter version
    tft_fillRectWH(win,x+r, y, w-2*r, h, color);

// draw four corners
    tft_fillCircleHelper(win,x+w-r-1, y+r, r, 1, h-2*r-1, color);
    tft_fillCircleHelper(win,x+r    , y+r, r, 2, h-2*r-1, color);
}



/// @brief Draw a triangle
/// @param[in] *win: window structure
/// @param[in] x0: X0 offset
/// @param[in] y0: Y0 offset
/// @param[in] x1: X1 offset
/// @param[in] y1: Y1 offset
/// @param[in] x2: X2 offset
/// @param[in] y2: Y2 offset
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_drawTriangle(window *win, int16_t x0, int16_t y0,
int16_t x1, int16_t y1,
int16_t x2, int16_t y2, uint16_t color)
{
    tft_drawLine(win,x0, y0, x1, y1, color);
    tft_drawLine(win,x1, y1, x2, y2, color);
    tft_drawLine(win,x2, y2, x0, y0, color);
}


/// @brief Fill a triangle
/// @param[in] *win: window structure
/// @param[in] x0: X0 offset
/// @param[in] y0: Y0 offset
/// @param[in] x1: X1 offset
/// @param[in] y1: Y1 offset
/// @param[in] x2: X2 offset
/// @param[in] y2: Y2 offset
/// @param[in] color: Color
/// @return  void
MEMSPACE
void tft_fillTriangle ( window *win, int16_t x0, int16_t y0,
int16_t x1, int16_t y1,
int16_t x2, int16_t y2, uint16_t color)
{

    int16_t a, b, y, last;

// Sort coordinates by Y order (y2 >= y1 >= y0)
    if (y0 > y1)
    {
        SWAP(y0, y1); SWAP(x0, x1);
    }
    if (y1 > y2)
    {
        SWAP(y2, y1); SWAP(x2, x1);
    }
    if (y0 > y1)
    {
        SWAP(y0, y1); SWAP(x0, x1);
    }

    if(y0 == y2)                                  // Handle awkward all-on-same-line case as its own thing
    {
        a = b = x0;
        if(x1 < a)      a = x1;
        else if(x1 > b) b = x1;
        if(x2 < a)      a = x2;
        else if(x2 > b) b = x2;
        tft_drawFastHLine(win,a, y0, b-a+1, color);
        return;
    }

    int16_t
        dx01 = x1 - x0,
        dy01 = y1 - y0,
        dx02 = x2 - x0,
        dy02 = y2 - y0,
        dx12 = x2 - x1,
        dy12 = y2 - y1,
        sa   = 0,
        sb   = 0;

// For upper part of triangle, find scanline crossings for segments
// 0-1 and 0-2.  If y1=y2 (flat-bottomed triangle), the scanline y1
// is included here (and second loop will be skipped, avoiding a /0
// error there), otherwise scanline y1 is skipped here and handled
// in the second loop...which also avoids a /0 error here if y0=y1
// (flat-topped triangle).
    if(y1 == y2) last = y1;                       // Include y1 scanline
    else         last = y1-1;                     // Skip it

    for(y=y0; y<=last; y++)
    {
        a   = x0 + sa / dy01;
        b   = x0 + sb / dy02;
        sa += dx01;
        sb += dx02;
/* longhand:
a = x0 + (x1 - x0) * (y - y0) / (y1 - y0);
b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
 */
        if(a > b) SWAP(a,b);
        tft_drawFastHLine(win,a, y, b-a+1, color);
    }

// For lower part of triangle, find scanline crossings for segments
// 0-2 and 1-2.  This loop is skipped if y1=y2.
    sa = dx12 * (y - y1);
    sb = dx02 * (y - y0);
    for(; y<=y2; y++)
    {
        a   = x1 + sa / dy12;
        b   = x0 + sb / dy02;
        sa += dx12;
        sb += dx02;
/* longhand:
a = x1 + (x2 - x1) * (y - y1) / (y2 - y1);
b = x0 + (x2 - x0) * (y - y0) / (y2 - y0);
 */
        if(a > b) SWAP(a,b);
        tft_drawFastHLine(win,a, y, b-a+1, color);
    }
}
