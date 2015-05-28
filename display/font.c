/**
 @file font.c

 @par bdffont2c BDF to C code converter
 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief Font display for ili9341 driver
 BDF = Glyph Bitmap Distribution Format
 The code handles fixed, proportional and bounding box format fonts
 @see http://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format
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
#include "util.h"
#include "ili9341.h"

/// @brief save fonts in flash
#define MEMSPACE_FONT ICACHE_FLASH_ATTR

/// @brief we need to use addition width,height and offset sepcifications
/// This is NOT required for full fonts with fixed atributes
#define FONTSPECS

// Font Structure defintions
#include "font.h"

/// @brief Generated Font table
//// Note the generated tables always include
//// Font specifications: widt, height, offsets, font type, etc
//// Font information: name, copyright, style information
//// FONSPECS and FONTINFO defines controls actual usage.
#include "fonts.h"

#include "ili9341.h"

// All the fonts - defined in fonts.h
extern _font *allfonts[];

/// @brief  Get font attributes for a font
/// @param[in] c: character
/// @param[in] *f: font structure
/// @param[in] index: font index (which font table we want)
/// @return  void
int font_attr(int c, _fontc *f, int index)
{
    int offset;
    int num;
    unsigned char *ptr;
    extern window win;

// check index
    _font *z = allfonts[index];
    _fontspecs s;

    num = c - z->First;
    if(num < 0 || num >= z->Glyphs)
        return(-1);

    f->ptr = z->bitmap;

// If we have font specifications use them
    if(z->specs)
    {
        f->Width = z->Width;
        f->Height = z->Height;

// Copy the full font specification into ram for easy access
// This does not use much memory as it does not include the bitmap
        cpy_flash((uint8_t *)&(z->specs[num]), (uint8_t *)&s,sizeof(_fontspecs));

        f->w = s.Width;
        f->h = s.Height;
        f->x = s.X;
        f->y = s.Y;

        offset = s.Offset;
        f->ptr += offset;

// Override only if we have specs
        f->fixed = win.fontfixed;

        if(f->fixed)
            f->gap = f->Width;
        else
            f->gap = f->w + (f->Width+3)/4;
    }

    else                                          // Create the font specificatio for the main font spec
    {
        f->Width = z->Width;
        f->Height = z->Height;
        f->gap = f->w + (f->Width+3)/4;

        f->w = z->Width;
        f->h = z->Height;
        f->x = 0;
        f->y = 0;

        offset = ((z->Width * z->Height)+7)/8;
        f->ptr += (offset * num);

    }

#ifdef ILI9341_DEBUG
    ets_uart_printf("c: %02x ind:%d w:%d h:%d x:%d y:%d gap:%d, W:%d, H:%d\r\n",
        0xff & c, 0xff & index, f->w, f->h, f->x, f->y, f->gap, f->Width, f->Height);
#endif
    return(index);
}


/// @brief  Display a character using font tables
/// @param[in] c: character
/// @param[in] x: X offset in pixels
/// @param[in] y: Y offset in pixels
/// @param[in] index: Font index (with font set to use )
/// @return  void
int tft_drawChar(uint8_t c, uint16_t x, uint16_t y, uint8_t index)
{
    _fontc f;
    int ret;
    int yskip;
    extern window win;

    ret = font_attr(c, &f, index);
    if(ret < 0)
        return (0);

    if(!f.h || !f.w)
        return (0);

// Alternate clear - all pixels inside the font bounding box
    if(f.h != f.Height ||  f.w != f.Width || f.x != 0 || f.y != 0)
        tft_fillRectWH(x, y, f.Width, f.Height, win.textbgcolor);

// top of bit bounding box ( first row with a 1 bit in it)
    yskip = f.Height - (f.y+f.h);

    tft_bit_blit(f.ptr, x+f.x, y+yskip, f.w, f.h, win.textcolor, win.textbgcolor);

    return (f.gap);
}
