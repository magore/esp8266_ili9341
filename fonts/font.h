/**

 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief par BDF font structure definitions for for BDF to C code converter 

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

/**
* BDF = Glyph Bitmap Distribution Format 
* See: http://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format
*/
#ifndef _FONT_H_
#define _FONT_H_


#include "stdint.h"

#define MEMSPACE /* */
#define MAXFONTS 256    /* Maximum number of fonts to convert at one time */
#define MAXGLYPHS 256 	/* Max Glyphs Per Font */
#define MAXWIDTH 127	/* Max Character Width */
#define MAXHEIGHT 127	/* Max Character Height */

typedef struct 
{
    char *filename;
    char *structname;
} _bdffile;

typedef struct {
    int Offset;              /* Byte offset to Glyph bit array */
    uint8_t Width;         /* Glyph width in bits */
    uint8_t Height;        /* Glyph hight in bits */
    int8_t X;             /* Glyph X offset when rendered */
    int8_t Y;             /* Glyph Y offset when rendering */
} _fontspecs;

typedef struct {
    char *FILE_NAME;         /* FILE NAME */
    char *STRUCT_NAME;       /* STRUCT NAME */
    char *COPYRIGHT;         /* COPYRIGHT */
    char *FONT_NAME;         /* FONT */
    char *FAMILY_NAME;       /* FAMILY NAME */
    char *WEIGHT_NAME;       /* WEIGHT_NAME */
    char *SLANT;             /* SLANT */
    char *SPACING;           /* SPACING */
} _fontinfo;

typedef struct {
    int16_t  Glyphs;             /* Glyphs in this font */
    uint8_t  Fixed;              /* Fixed Format Font, Font info not required  */
    int16_t  First;              /* First Font value in this font set set */
    uint8_t Width;     /* Glyph Width in bits for this font set */
    uint8_t Height;    /* Glyph Hight in bits for this font set */
    int8_t X;           /* Glyph X orgin for this font set */
    int8_t Y;           /* Glyph Y orgin for this font set */
    int8_t Ascent;      /* Glyph Ascent for this font set */
    int8_t Decent;      /* Glyph Decent for this font set */
    /* Working font bitmap buffer */
    int  Bytes;              /* Number of bytes in bitmap */
    uint8_t *bitmap;   /* Font Bitmap */
    _fontspecs *specs;       /* Glyph info */
    _fontinfo *info;         /* Copywrite, etc */
} _font;

typedef struct {
    int8_t index;   // font index
    int8_t fixed;
    int8_t w;       // font width - caries for each character
    int8_t h;       // font hight
    int8_t x;       // font width - caries for each character
    int8_t y;       // font hight
    int8_t Width;   // font max width
    int8_t Height;  // font max width
    int8_t gap;     // font gap
    uint8_t *ptr;
} _fontc;


#endif //_FONT_H_
