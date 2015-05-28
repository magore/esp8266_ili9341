/**
 @file ili9341.h

 @par Copyright &copy; 2015 Mike Gore, Inc. All rights reserved.
 @par Edit History
      - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option)
 any later version.

ili9431_sup.h is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _ILI9341_H_
#define _ILI9341_H_

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <gpio.h>
#include <mem.h>
#include "util.h"
#include "hspi.h"
#include "font.h"

typedef struct
{
    int16_t x;                                    // x pos
    int16_t y;                                    // y pos
    uint16_t  fontindex;                          // font index
    uint16_t  fontfixed;                          // font fixed == 1, var == 0
    uint8_t wrap;                                 // wrap mode
    int16_t min_x;
    int16_t max_x;
    int16_t min_y;
    int16_t max_y;
    uint8_t tabcolor;
    uint16_t textcolor;
    uint16_t textbgcolor;
    uint8_t rotation;
} window;
// ============================================================

/* ili9341_sup.c */
MEMSPACE uint32_t tft_init ( void );
uint32_t tft_clip ( int16_t *xs , int16_t *xl , int16_t *ys , int16_t *yl );
void tft_window ( int16_t x , int16_t y , int16_t xl , int16_t yl );
void tft_writeCmd ( uint8_t cmd );
void tft_writeData ( uint8_t data );
void tft_writeCmdData ( uint8_t cmd , uint8_t *data , uint8_t bytes );
void tft_writeData16 ( uint16_t val );
void tft_writeColor16Repeat ( uint16 color , uint32_t count );
void tft_writeDataBuffered ( uint16_t *color_data , uint32_t count );
uint32_t tft_readRegister ( uint8_t reg , uint8_t parameter );
MEMSPACE uint32_t tft_readId ( void );
MEMSPACE void tft_setRotation ( uint8_t m );
MEMSPACE void window_init ( void );
MEMSPACE void tft_setTextColor ( uint16_t c , uint16_t b );
MEMSPACE window *tft_setpos ( int16_t x , int16_t y );
MEMSPACE window *tft_set_fontsize ( uint16_t index );
MEMSPACE window *tft_font_fixed ( void );
MEMSPACE window *tft_font_var ( void );
MEMSPACE window *tft_getwin ( void );
MEMSPACE int16_t tft_height ( void );
MEMSPACE int16_t tft_width ( void );
MEMSPACE uint8_t tft_getRotation ( void );
MEMSPACE uint16_t tft_color565 ( uint8_t r , uint8_t g , uint8_t b );
MEMSPACE void convert565toRGB ( uint16_t color , uint8_t *r , uint8_t *g , uint8_t *b );
MEMSPACE void tft_invertDisplay ( int flag );
void tft_bit_blit ( uint8_t *ptr , int x , int y , int w , int h , uint16_t fg , uint16_t bg );
void tft_blit ( uint16_t *ptr , int x , int y , int w , int h );
void tft_drawPixel ( int16_t x , int16_t y , int16_t color );
void tft_drawLine ( int16_t x0 , int16_t y0 , int16_t x1 , int16_t y1 , uint16_t color );
void tft_drawLine ( int16_t x0 , int16_t y0 , int16_t x1 , int16_t y1 , uint16_t color );
void tft_fillRectWH ( int16_t x , int16_t y , int16_t w , int16_t h , uint16_t color );
void tft_fillRectXY ( int16_t x , int16_t y , int16_t xl , int16_t yl , uint16_t color );
void tft_fillWin ( uint16_t color );
MEMSPACE void tft_cleareol ( void );
MEMSPACE void tft_putch ( int c , window *win );
MEMSPACE int tft_drawNumber ( long long_num , int16_t x , int16_t y , uint8_t size );
MEMSPACE int tft_drawString ( const char *string , int16_t x , int16_t y , uint8_t size );
MEMSPACE int tft_drawCentreString ( const char *string , int16_t xs , int16_t y , uint8_t index );
MEMSPACE int tft_drawRightString ( const char *string , int16_t xs , int16_t y , uint8_t index );


#endif                                            // _ILI9341_SUP_H_
