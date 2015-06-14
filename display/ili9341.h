/**
 @file ili9341.h

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

#ifndef _ILI9341_H_
#define _ILI9341_H_

typedef struct
{
    int16_t x;       // x pos
    int16_t y;       // y pos
    uint16_t  font;	// font index
    uint16_t  fixed;// font fixed == 1, var == 0
    uint8_t wrap;   // wrap mode
    int16_t w;
    int16_t xoff;
    int16_t h;
    int16_t yoff;
    uint16_t fg;
    uint16_t bg;
    uint8_t rotation;
} window;

#define TFT_W (MAX_TFT_X-MIN_TFT_X+1)
#define TFT_H (MAX_TFT_Y-MIN_TFT_Y+1)
#define TFT_XOFF (MIN_TFT_X)
#define TFT_YOFF (MIN_TFT_Y)

// ============================================================

/* ili9341.c */
MEMSPACE window *tft_init ( void );
uint32_t tft_abs_window ( int16_t x , int16_t y , int16_t w , int16_t h );
void tft_writeCmd ( uint8_t cmd );
void tft_writeData ( uint8_t data );
void tft_writeCmdData ( uint8_t cmd , uint8_t *data , uint8_t bytes );
void tft_writeData16 ( uint16_t val );
void tft_writeColor16Repeat ( uint16 color , uint32_t count );
void tft_writeDataBuffered ( uint16_t *color_data , uint32_t count );
uint32_t tft_readRegister ( uint8_t reg , uint8_t parameter );
MEMSPACE uint32_t tft_readId ( void );
uint16_t tft_readData16 ( void );
MEMSPACE uint16_t tft_color565 ( uint8_t r , uint8_t g , uint8_t b );
MEMSPACE void convert565toRGB ( uint16_t color , uint8_t *r , uint8_t *g , uint8_t *b );
MEMSPACE void tft_invertDisplay ( int flag );
void tft_bit_blit ( window *win , uint8_t *ptr , int x , int y , int w , int h , uint16_t fg , uint16_t bg );
void tft_blit ( window *win , uint16_t *ptr , int x , int y , int w , int h );
void tft_fillWin ( window *win , uint16_t color );
void tft_fillRectWH ( window *win , int16_t x , int16_t y , int16_t w , int16_t h , uint16_t color );
void tft_fillRectXY ( window *win , int16_t x , int16_t y , int16_t xl , int16_t yl , uint16_t color );
uint32_t tft_rel_window ( window *win , int16_t x , int16_t y , int16_t w , int16_t h );
MEMSPACE void tft_setRotation ( uint8_t m );
MEMSPACE void tft_window_init ( window *win , uint16_t xoff , uint16_t yoff , uint16_t w , uint16_t h );
MEMSPACE void tft_setTextColor ( window *win , uint16_t c , uint16_t b );
MEMSPACE void tft_setpos ( window *win , int16_t x , int16_t y );
MEMSPACE tft_set_font ( window *win , uint16_t index );
MEMSPACE int tft_get_font_height ( window *win );
MEMSPACE tft_font_fixed ( window *win );
MEMSPACE void tft_font_var ( window *win );
void tft_drawPixel ( window *win , int16_t x , int16_t y , int16_t color );
void tft_drawLine ( window *win , int16_t x0 , int16_t y0 , int16_t x1 , int16_t y1 , uint16_t color );
void tft_drawLine ( window *win , int16_t x0 , int16_t y0 , int16_t x1 , int16_t y1 , uint16_t color );
MEMSPACE void tft_cleareol ( window *win );
MEMSPACE void tft_putch ( window *win , int c );


#endif // _ILI9341_SUP_H_
