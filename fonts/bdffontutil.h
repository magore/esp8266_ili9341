#ifndef _UTIL_H_
#define _UTIL_H_
/**
 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief par BDF font utils for BDF to C code converter 

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

#define MATCH(a,b)  ( ( strcmp(a,b) == 0 ) ? 1 : 0 )
#define MAXLINE 1024

/* bdffontutil.c */
void *db_calloc ( size_t size );
void *db_free ( void *p );
char *stralloc ( char *str );
char *remove_quotes ( char *str );
void line_wrap ( char *str , int max );
void trim_tail ( char *str );
char *skip_spaces ( char *str );
char *get_token ( char *str , char *token , int max );
int ishex ( int c );
int ishexstr ( char *str );
char *match_token ( char *str , char *pat );
void FontHeaderInfo ( FILE *out , _font *font , char *prog );
void Convert_Font2c ( FILE *out , _font *font );
void emit_number ( FILE *out , char *name , int num );
void emit_data ( FILE *out , char *name , unsigned char *data , int size );
void emit_str ( FILE *out , char *name , unsigned char *data );
void InitNames ( void );
void InitFonts ( _font *font );
void FreeFont ( _font *font );
void AddFontName ( _font *font );
int FindFontName ( char *str );
void WriteFontBits ( FILE *out , _font *font );
void WriteFontInfo ( FILE *out , _font *font );
void WriteFontTable ( FILE *out , _font *font );
void WriteCharacterBits ( FILE *out , _font *font , int num );
int ReadBdf ( char *name , _font *font , int lower , int upper );
void AdjustFontTable ( _font *font );
void FontAdjustFull ( _font *font );
void FontAdjustSmall ( _font *font );
void bsetv ( unsigned char *ptr , int addr );
void bclrv ( unsigned char *ptr , int addr );
int btestv ( unsigned char *ptr , int addr );
int bittestxy ( unsigned char *ptr , int x , int y , int width , int height );
void bitsetxy ( unsigned char *ptr , int x , int y , int width , int height );
void bitclrxy ( unsigned char *ptr , int x , int y , int width , int height );
void FontPreview ( FILE *out , _font *font , int num );
void FontPreviewFull ( FILE *out , _font *font , int num );
void FontPreviewProportional ( FILE *out , _font *font , int num );
void WriteFontBitsPreview ( FILE *out , _font *font , int preview );

#endif // _UTIL_H_

extern _font font;
extern _bdffile BDFnames[MAXFONTS];
extern char *fnames[MAXFONTS];

