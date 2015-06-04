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
* @see http://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format
*/

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "font.h"
#include "bdffontutil.h"

// Maximum number of fonts we can convert
_bdffile BDFnames[MAXFONTS];
// File names to convert
char *fnames[MAXFONTS];

char *EMPTY = "";

/**
 @brief calloc memory or error exit
 @param[in] size: memory size to calloc
 @return: void * pointer
*/
void *db_calloc(size_t size)
{
	void *ptr = calloc( size + 2, 1);
	if( ptr == NULL )
	{
			fprintf(stderr,"Can not allocate memory\n");
			exit(1);
	}
	return(ptr);
}

/**
 @brief free memory 
 @param[in] *p: memory pointer to free
 @return: void * pointer
*/
void *db_free(void *p)
{
	if( p == NULL  || p == EMPTY)
	{
		return EMPTY;
	}
	free(p);
}


/**
 @brief Allocate memory and copy string into it
 @param[in] *str: string to allocate and copy memory for
 @return: char * pointer to allocated string
*/
char *stralloc(char *str)
{
	char *ptr;
	int len = strlen(str);
	
	ptr = db_calloc(len+1);
	strcpy(ptr,str);
	return(ptr);
}


/**
 @brief STrip quotes from string and leading spaces
 @param[in] *str: string to remove quotes from
 @return: char * pointer to first non-space character
*/
char *remove_quotes(char *str)
{
	if(!str)
		return(NULL);

	char *base;
	while(*str)
	{
		if(*str == '\"' || *str == ' ' || *str == '\t')
			++str;
		else
			break;
	}
	base = str;
	while(*str)
	{
	 	if(*str == '\"')
			*str = ' ';
		++str;
	}
	trim_tail(base);
	return(base);
}


/**
 @brief Line wrap function
  ASSUMES we write back into the string (replace space or tab with newlines 
 @param[in] *str: string to remove quotes from
 @param[in] max: Maximum string length
 @return: void
*/
void line_wrap(char *str, int max)
{
	int len = 0;
	int lastsp = 0;
	char *sp = NULL;
	while(*str)
	{
		if(*str == '\n')
		{
			sp = NULL;
			len = 0;
		}
		if(len >= max)
		{
			if(sp)
			{
				*sp = '\n';
				sp = NULL;
			}
			len = 0;
		}
		if(*str == ' ' || *str == '\t')
		{
			sp = str;
		}
		++str;
		++len;
	}
}


/**
 @brief Remove characters less then or equal to space from end of string
 @param[in] *str: string to remove quotes from
 @return: void
*/
void trim_tail(char *str)
{
    int len = strlen(str);
    while(len--)
    {
        if(str[len] > ' ')
            break;
        str[len] = 0;
    }
}

/**
 @brief Skip spaces at start of string
 @param[in] *str: string to remove quotes from
 @return: first non space character
*/
char *skip_spaces(char *str)
{
	// Skip not displayable characters first
	while(*str && ( *str <= ' ' || *str >= 0x7f) )
		++str;
	return(str);
}

/**
 @brief get first non space containing string
  Skip spaces at start of string
 @param[in] *str: string 
 @param[out] *token: token characters greter then space less then 0x7f
 @param[in] max: maximum length of token
 @return: string pointer to character after token 
*/
char *get_token(char *str, char *token, int max)
{
    int len;
	char *save;

	*token = 0;

	// NULL ?
	if(!str)
		return(NULL);

	str = skip_spaces(str);
	save = str;

	// Find size of token
    len = 0;
    while(*str > ' ' && *str <= 0x7e && len < max)
    {
		// clip token to max length
		if(len < max)
		{
			*token++ = *str++;
			++len;
		}
    }   
	*token = 0;
	// str points past the token
	if(!len)
		return(NULL);
	return(str);
}


/**
 @brief is a character hex ASCII character
 @param[in] c: character to test
 @return: 0 .. 15 or -1 on error
*/
int ishex(int c)
{
	if( (c >= '0' && c <= '9') )
		return( c - '0');
	else if (c >= 'A' && c <= 'F')
		return( c - 'A' + 10);
	else if (c >= 'a' || c <= 'f')
		return(c - 'a'+ 10);
	return(-1);
}

/**
 @brief Does a string only contain hex characters ?
 @return: 1 if hex , 0 if not
*/
int ishexstr(char *str)
{
	while(*str && ishex(*str) >= 0)
		++str;
	if(!*str)
		return(1);
	return(0);
}

/**
 @brief Match next token against pattern
 @param[in] str: string to search
 @param[in] pat: pattern
  Skips spaces at start of string
  A token is any string with caracters greter then space less then 0x7f
 @return: length of match on sucess
*/
char *match_token(char *str, char *pat)
{
    int patlen;
    int len;
    char *ptr;
	char *save = str;
    
	// Skip not displayable characters
	while(*str <= ' '  || *str >= 0x7f)
		++str;

	// Find size of token
    ptr = str;
    len = 0;
    while(*ptr > ' ' && *ptr <= 0x7e )
    {
        ++len;
        ++ptr;
    }   
	// ptr now points past the token in str
    
	// Empty string
    if(!len)
        return(NULL);
        
    patlen = strlen(pat);
    
	// Size mismatch ?
    if(len != patlen)
        return(NULL);
        
    if(strncmp(str,pat,patlen) == 0)
        return(ptr);	
    return(NULL);
}   
/**
 @brief Write Font Header Information
 Copyright, font family, etc
 @param[in] *out: File handle
 @param[in] *font: Font handle
 @param[in] *prog: program name
 @return: void
*/
void FontHeaderInfo(FILE *out, _font *font, char *prog, char *target)
{
	if(target == NULL)
	{
		if(font->info)
			target = basename(font->info->FILE_NAME);
		else
			target = "";
	}
	fprintf(out,"/**\n");
	fprintf(out," @file %s\n", target);
	fprintf(out,"  Created using %s (c) 2015 by Mike Gore\n"
		"  License GPL3\n"
		"\n", basename(prog));
	if(font->info)
	{
		fprintf(out,"  BDF File:   [%s]\n", font->info->FILE_NAME);
		fprintf(out,"  C structure:[%s]\n", font->info->STRUCT_NAME);
		fprintf(out,"\n");
		fprintf(out,"  FONT COPYRIGHT: %s\n", font->info->COPYRIGHT);
		fprintf(out,"  FONT NAME:      %s\n", font->info->FONT_NAME);
		fprintf(out,"  FAMILY_NAME:    %s\n", font->info->FAMILY_NAME);
		fprintf(out,"  WEIGHT_NAME:    %s\n", font->info->WEIGHT_NAME);
		fprintf(out,"  SLANT:          %s\n", font->info->SLANT);
		fprintf(out,"\n");
	}
	else
	{
		fprintf(out,"Font info empty\n");

	}
	fprintf(out,"*/\n");
}


/**
 @brief Convert font to C structure
  Writes header information, font specification, font bitmap
 @param[in] *out: File handle
 @param[in] *font: Font handle
 @return: void
*/
void Convert_Font2c(FILE *out, _font *font)
{
	fprintf(out,"\n");
	fprintf(out,"#ifndef MEMSPACE_FONT\n");
	fprintf(out, "  #define MEMSPACE_FONT /* */\n");
	fprintf(out, "#endif\n");
	fprintf(out, "\n");

	if(font->info != NULL)
	{
		/* Forware Reference to Font table and Font BIT data */
		fprintf(out,"extern unsigned char %s_bitmap[];\n", 
			font->info->STRUCT_NAME);
		fprintf(out, "\n");

		fprintf(out,"#ifdef FONTSPECS\n");
		fprintf(out,"   extern _fontspecs %s_specs[];\n", 
			font->info->STRUCT_NAME);
		fprintf(out,"#endif\n");
		fprintf(out, "\n");

		fprintf(out,"#ifdef FONTINFO\n");
		fprintf(out,"   extern _fontinfo %s_info[];\n", 
			font->info->STRUCT_NAME);
		fprintf(out,"#endif\n");
		fprintf(out, "\n");

		fprintf(out,"/* Font %s */\n\n", 
			font->info->STRUCT_NAME);
		fprintf(out,"_font %s = {\n", 
			font->info->STRUCT_NAME);
		fprintf(out, "\n");
	}

	/* Write Font Structure */
	emit_number(out,"Glyphs", font->Glyphs);
    emit_number(out,"Fixed Width Flag", font->Fixed);
    emit_number(out,"First Glyph", font->First);
    emit_number(out,"Font Bounding Box Width", font->Width);
    emit_number(out,"Font Bounding Box Height", font->Height);
    emit_number(out,"Font Orgin X", font->X);
    emit_number(out,"Font Orgin Y", font->Y);
    emit_number(out,"Font Ascent", font->Ascent);
    emit_number(out,"Font Decent", font->Decent);
	emit_number(out,"Font Bytes for entire Bitmap", font->Bytes);


	if(font->info != NULL)
	{
		fprintf(out,"\t%s_bitmap,\n", font->info->STRUCT_NAME);
		fprintf(out,"#ifdef FONTSPECS\n");
		fprintf(out,"\t%s_specs,\n", font->info->STRUCT_NAME);
		fprintf(out,"#else\n");
		fprintf(out,"\tNULL, /* font->specs */\n");
		fprintf(out,"#endif\n");
		fprintf(out,"#ifdef FONTINFO\n");
		fprintf(out,"\t%s_info\n", font->info->STRUCT_NAME);
		fprintf(out,"#else\n");
		fprintf(out,"\tNULL /* font->info */\n");
		fprintf(out,"#endif\n");
	}

	fprintf(out,"};\n\n");

	WriteFontInfo( out , font);
	WriteFontTable ( out , font);
	WriteFontBits ( out , font);
}

/**
 @brief Write Font Structure element as a number
 @param[in] *out: File output handle
 @param[in] *name: number structure name 
 @param[in] num: number to display
 @return: void
*/
void emit_number(FILE *out, char *name, int num)
{
	fprintf(out,"\t/* %s = %d */\n", name, num);
	fprintf(out,"\t\t\%d,\n", num);
}


/**
 @brief Write Font Structure element as a uint8_t value
 @param[in] *out: File handle
 @param[in] *name: structure name 
 @param[in] *data: data
 @param[in] size: size of data to write
 @return: void
*/
void emit_data(FILE *out, char *name, unsigned char *data, int size)
{


	fprintf(out,"\t/* %s */\n", name);
	fprintf(out,"\t\t{\n");
	while(size--)
	{
		fprintf(out,"0x%02x,", *data);
		data++;
	}
	fprintf(out,"\t\t},\n");
}

/**
 @brief Write Font Structure string 
 @param[in] *out: File handle
 @param[in] *name: structure name
 @param[in] *data: string
 @return: void
*/
void emit_str(FILE *out, char *name, unsigned char *data)
{
	fprintf(out,"\t/* %s */\n", name);
	if(!data)
		fprintf(out,"\t\tNULL,\n");
	else
		fprintf(out,"\t\t\"%s\",\n", data);
}

/**
 @brief Reset File and Structure names
 @return: void
*/
void InitNames()
{
	int i;

	for(i=0;i<MAXFONTS;++i)
	{
		fnames[i] = NULL;
	}

	for(i=0;i<MAXFONTS;++i)
	{
		BDFnames[i].filename = NULL;
		BDFnames[i].structname = NULL;
	}
}

/**
 @brief Initialize all font structures to reset states
 @param[in] *font: Font pointer
 @return: void
*/
void InitFonts(_font *font)
{
	int i;

	_fontinfo *info = db_calloc(sizeof(_fontinfo));
	info->FILE_NAME = EMPTY;
	info->STRUCT_NAME = EMPTY;
	info->COPYRIGHT = EMPTY;
	info->FONT_NAME = EMPTY;
	info->FAMILY_NAME = EMPTY;
	info->WEIGHT_NAME = EMPTY;
	info->SLANT = EMPTY;
	info->SPACING = EMPTY;

	font->info = info;
	font->Width =  0;
	font->Height = 0;
	font->Ascent = 0;
	font->Decent = 0;
	font->First = 0;
	font->Fixed = 0;
	font->Bytes = 0;
	font->bitmap = NULL;
	font->specs = db_calloc(sizeof(_fontspecs) * MAXGLYPHS);
	for(i=0;i<MAXGLYPHS;++i)
	{
		font->specs[i].Offset = -1;
		font->specs[i].Width = 0;
		font->specs[i].Height = 0;
		font->specs[i].X = 0;
		font->specs[i].Y = 0;
	}
}

/**
 @brief Free font data
 @param[in] *font: Font pointer
 @return: void
*/
void FreeFont(_font *font)
{
	_fontinfo *info = font->info;
	if(info != NULL)
	{
		db_free(info->FILE_NAME);
		db_free(info->STRUCT_NAME);
		db_free(info->COPYRIGHT);
		db_free(info->FONT_NAME);
		db_free(info->FAMILY_NAME);
		db_free(info->WEIGHT_NAME);
		db_free(info->SLANT);
		db_free(info->SPACING);
		db_free(info);
		font->info = NULL;
	}

	db_free(font->bitmap);
	db_free(font->specs);
}


/**
 @brief Generate the font name for this font
 This is based on the BDF FAMILY_NAME,WEIGHT_NAME,SLANT keywords
 @param[in] *font: Font pointer
 @return: void
*/
void AddFontName(_font *font)
{
	char name[MAXLINE];
	char *save = name;
	/* We define the font by its size - not its bounding box */
	snprintf(name,MAXLINE-1, "%s_%s_%s_X%d_Y%d",
		font->info->FAMILY_NAME,
		font->info->WEIGHT_NAME, 
		font->info->SLANT, 
		font->Width,
		font->Height
	);
	char *ptr = name;
	while(*ptr)
	{
		if(!isalnum(*ptr))
		{
			*ptr = '_';
		}
		++ptr;
	}
	font->info->STRUCT_NAME = stralloc(save);
	
}

/**
 @brief Search for a font name
 @return: index into BDFnames[]
*/
int FindFontName(char *str)
{
	int i;

	for(i=0;BDFnames[i].structname != NULL;++i)
	{
		if(strcmp( BDFnames[i].structname,  str) == 0)
		{
			return(i);
		}
	}
	return(-1);
}

/**
 @brief Write Font bitmap data for all charactres in font
 @param[in] *out: File handle
 @param[in] *font: Font pointer
 @return: void
*/
void WriteFontBits(FILE * out, _font *font)
{
	int i;
	int c;
	fprintf(out, "/* Font BIT DATA , MSB Left to Right (padded to byte alignment), Top Down */\n");

	if(font->info)
	{
		fprintf(out,"MEMSPACE_FONT unsigned char %s_bitmap[%d]= { /* %s_bitmap */\n", font->info->STRUCT_NAME, font->Bytes, font->info->STRUCT_NAME);
	}
	for(i=0;i<font->Glyphs;++i)
	{
		WriteCharacterBits(out, font, i);
	}
    fprintf(out, "};\n");
}

/**
 @brief Write Information
 @param[in] *out: File handle
 @param[in] *font: Font pointer
 @return: void
*/
void WriteFontInfo(FILE * out, _font *font)
{
	int i;
	int c;

	if(!font->info)
		return;

    fprintf(out, "#ifdef FONTINFO\n");
    fprintf(out, "MEMSPACE_FONT _fontinfo %s_info[%d] = /* %s_info */\n", 
		font->info->STRUCT_NAME, font->Glyphs, font->info->STRUCT_NAME);
    fprintf(out, "\t{\n");
	emit_str(out,"FILE_NAME", font->info->FILE_NAME);
	emit_str(out,"STRUCT_NAME", font->info->STRUCT_NAME);
	emit_str(out,"COPYRIGHT", font->info->COPYRIGHT);
	emit_str(out,"FONT", font->info->FONT_NAME);
	emit_str(out,"FAMILY_NAME", font->info->FAMILY_NAME);
	emit_str(out,"WEIGHT_NAME", font->info->WEIGHT_NAME);
	emit_str(out,"SLANT", font->info->SLANT);
	emit_str(out,"SPACING", font->info->SPACING);
	fprintf(out, "};\n");
	fprintf(out, "#endif\n");
}

/**
 @brief Write Specification Table
 @param[in] *out: File handle
 @param[in] *font: Font pointer
 @return: void
*/
void WriteFontTable(FILE * out, _font *font)
{
	int i;
	int c;
	
	if(!font->specs || !font->info)
		return;

	fprintf(out, "#ifdef FONTSPECS\n");
	fprintf(out, "MEMSPACE_FONT _fontspecs %s_specs[%d] = /* %s_specs */\n", 
		font->info->STRUCT_NAME, font->Glyphs, font->info->STRUCT_NAME);
	fprintf(out, "\t{\n");

	fprintf(out, "\t\t/* Offset, Width, Height,  X,     Y*/\n");
	for(i=0;i<font->Glyphs;++i)
	{
		c = i + font->First;
		fprintf(out, "\t\t{ % 5d, % 5d, % 5d, % 5d, %5d }, /* [%c]*/\n",
			font->specs[i].Offset,
			font->specs[i].Width,
			font->specs[i].Height,
			font->specs[i].X,
			font->specs[i].Y,
			c);
		if(font->specs[i].Offset == -1)
			break;
	}
	fprintf(out, "};\n");
	fprintf(out, "\n#endif\n");
}
/**
 @brief Write Font bitmap data for one charactre in font
 @param[in] *out: File handle
 @param[in] *font: Font pointer
 @param[in] num: character number
 @return: void
*/
void WriteCharacterBits(FILE * out, _font *font, int num)
{
	int c;
	int x,y;
	int xoff,yoff;
	int w,h;
	int bytes;
	int offset;
	int i, mask;
	unsigned char *ptr = font->bitmap;

	if(num < 0 || num >= font->Glyphs)
		return;

    if(font->specs)
    {

        offset = font->specs[num].Offset;
        w = font->specs[num].Width;
        h = font->specs[num].Height;
        xoff = font->specs[num].X;
        yoff = font->specs[num].Y;
        bytes = ((w * h) + 7)/8;
        ptr += offset;
    }
    else
    {
        w = font->Width;
        h = font->Height;
        xoff = 0;
        yoff = 0;
        bytes = ((w * h) + 7)/8;
        offset = (bytes * num);
		ptr += offset;
    }

	c = num + font->First;

    fprintf(out, "/* index:%d, [%c] 0x%02x, W:% 3d, H:% 3d, X:% 3d, Y:% 3d */\n",
        num, c, c,
        w,
        h,
        xoff,
        yoff
    );

	for(i=0;i<bytes;++i)
	{
		if((i & 15) == 0)
		   fprintf(out,"\t");
		fprintf(out,"0x%02x,", ptr[i]);
		if((i & 15) == 15)
		   fprintf(out,"\n");
	}
	if((i & 15) != 0)
	   fprintf(out,"\n");
}


/**
 @brief Read BDF file
  Read BDF file format
 @see: http://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format
 @param[in] *name: BDF File name
 @param[in] *font: Font pointer
 @param[in] lower: First character
 @param[in] upper: Last character
 @return: number of characters processed
*/
int ReadBdf(char *name, _font *font, int lower, int upper)
{
	FILE *bdf;

	// Our output font byte array offset
	int offset = 0;
	// Current Character value
    int character;
	// HEX data decoding vars
	int num,digit,mask;
	// BITMAP X an Y for decoding data bitmap data
	int x, addr, hexline;
	// Current Glyph table index
    int ind;
	// Font DWIDTH var - horizontal distance to next font
    int Next;
	// Font bitmap size in bytes	
	int bytes;
	// Font bounding box for the currennt working font 
    int BitBox_X = 0;
    int BitBox_Y = 0;
    int BitBox_Width = 0;
    int BitBox_Height = 0;
	// Total number of Glyps defined in this BDF file
	int Glyphs = 0;
	int BBXF = 0;
	int max = 0;

    unsigned char *ptr, *save, *bp;
	
	int len;

	// File line buffer
    char line[MAXLINE];
	// Working token
    char token[MAXLINE];

	// Font STructure Name
    char bdfname[MAXLINE];
    line[0]=0;
    token[0]=0;
	bdfname[0] = 0;

	InitFonts(font);

	bdf = fopen(name,"r");
	if(!bdf )
	{
		fprintf(stderr,"Can't open: %s\n", name);
		return(0);
	}

	font->info->FILE_NAME = stralloc(name);

	// Number glyphs defined in this BDF file
    Glyphs = -1;

	font->First = lower;

	// SCAN start of BDF file until the CHARS token
    while( fgets(line, sizeof(line), bdf) != NULL) 
	{
		trim_tail(line);

		len = strlen(line);
		if(!len)
			continue;

		ptr = get_token(line,token,sizeof(token)-1);
		if(MATCH(token, "FONTBOUNDINGBOX"))
		{
			// We do not have to check ptr == NULL, get_token() does that - token will be empty on NULL

			ptr = get_token(ptr,token,sizeof(token)-1);
			/* Width and Height of the Bounding box - one greater in X,Y of the largest Glyph in all fonts */
			font->Width = atoi(token);
			ptr = get_token(ptr,token,sizeof(token)-1);
			font->Height = atoi(token);

			/* Lower left-hand corner starting at X, Y */
			ptr = get_token(ptr,token,sizeof(token)-1);
			font->X = atoi(token);
			ptr = get_token(ptr,token,sizeof(token)-1);
			font->Y = atoi(token);

			BBXF = 1;
			continue;

		}
		if(MATCH(token, "FONT_ASCENT"))
		{
			// We do not have to check ptr == NULL, get_token() does that - token will be empty on NULL
			ptr = get_token(ptr,token,sizeof(token)-1);
			font->Ascent = atoi(token);
			continue;
		}
		if(MATCH(token, "FONT_DESCENT"))
		{
			// We do not have to check ptr == NULL, get_token() does that - token will be empty on NULL
			ptr = get_token(ptr,token,sizeof(token)-1);
			font->Decent = atoi(token);
			continue;
		}

		if(MATCH(token, "FONT")) 
		{
			ptr = remove_quotes(ptr);
			if(ptr) font->info->FONT_NAME = stralloc(ptr);
			//line_wrap(font->FONT_NAME, 72);
			continue;
		}
		if(MATCH(token, "FAMILY_NAME")) 
		{
			ptr = remove_quotes(ptr);
			if(ptr) font->info->FAMILY_NAME = stralloc( ptr);
			continue;
		}
		if(MATCH(token, "WEIGHT_NAME")) 
		{
			ptr = remove_quotes(ptr);
			if(ptr) font->info->WEIGHT_NAME = stralloc(ptr);
			continue;
		}
		if(MATCH(token, "SLANT")) 
		{
			ptr = remove_quotes(ptr);
			if(ptr) font->info->SLANT = stralloc(ptr);
			continue;
		}
		if(MATCH(token, "SPACING")) 
		{
			int c;
			ptr = remove_quotes(ptr);
			if(ptr) 
			{
				font->info->SPACING = stralloc(ptr);
				c = toupper( font->info->SPACING[0] );
				if(c == 'P')
					font->Fixed = 0;
				if(c == 'C' || c == 'M')
					font->Fixed = 1;
				continue;
			}
		}
		if(MATCH(token, "COPYRIGHT")) 
		{
			ptr = remove_quotes(ptr);
			if(ptr) font->info->COPYRIGHT = stralloc(ptr);
			continue;
		}
		// Character data follows after this
		// Number of Glyphs in the BDF file
		if(MATCH(token, "CHARS")) 
		{
			ptr = get_token(ptr,token,sizeof(token)-1);
			Glyphs = atoi(token);
			break;
		}
    }
	
	// We must have a Font bounding box for these fonts
    if (!BBXF) 
	{
		fprintf(stderr, "FONTBOUNDINGBOX missing!\n");
		fprintf(stderr, "%s\n",font->info->FONT_NAME);
		fclose(bdf);
		return(0);
    }
    if (Glyphs <= 0) 
	{
		fprintf(stderr, "CHARS token is missing\n");
		fprintf(stderr, "%s\n",font->info->FONT_NAME);
		fclose(bdf);
		return(0);
    }


	// Calloc space for ALL Glyph bitmap arrays
	// Each character is a stream of bits W * H rounded to a byte boundry
	// So if we round the width up to a byte boundry we will have a safe margin
	// bp = db_calloc(( Glyphs * ((font->Width) + 7) / 8) * font->Height);
	bp = db_calloc(( Glyphs * ((MAXWIDTH+1) + 7) / 8) * (MAXHEIGHT+1));
	font->bitmap = bp;
	// Save start of glyph array

	// Save Font Size in font entry
    ind = 0;
	// Initialize

	// Font Variables
    hexline = 0;
    BitBox_X = 0;
    BitBox_Y = 0;
    BitBox_Width = 0;
    BitBox_Height = 0;
    character = -1;
    Next = font->Width-1;	// Default horizontal distance to next Glyph

	// SCAN remander of BDF file until end of fie
    while( fgets(line, sizeof(line), bdf) != NULL) 
	{
		trim_tail(line);
		len = strlen(line);
		if(!len)
			continue;

		ptr = get_token(line,token,sizeof(token)-1);
		if(MATCH(token, "STARTCHAR")) 
		{
			ptr = get_token(ptr,token,sizeof(token)-1);
			if(ishexstr(token))
			{
				sscanf(token,"%x", &character);
				/* Reset Font Variables */
				hexline = 0;
				BitBox_X = 0;
				BitBox_Y = 0;
				BitBox_Width = 0;
				BitBox_Height = 0;
			}
			continue;
		} 

		if(MATCH(token, "ENCODING")) 
		{
			ptr = get_token(ptr,token,sizeof(token)-1);

			character = atoi(token);

			/* Reset Font Variables */
			hexline = 0;
			BitBox_X = 0;
			BitBox_Y = 0;
			BitBox_Width = 0;
			BitBox_Height = 0;
			continue;
		} 

		/* Horizontal offset to next Glyph */
		if(MATCH(token, "DWIDTH")) 
		{
			ptr = get_token(ptr,token,sizeof(token)-1);
			Next = atoi(token);
			continue;
		} 

		/* Font bit  box size and offset */
		if(MATCH(token, "BBX")) 
		{
			// Font Width and height 
			ptr = get_token(ptr,token,sizeof(token)-1);
			BitBox_Width = atoi(token);
			ptr = get_token(ptr,token,sizeof(token)-1);
			BitBox_Height = atoi(token);

			// Font offset required to render font
			ptr = get_token(ptr,token,sizeof(token)-1);
			BitBox_X = atoi(token);
			ptr = get_token(ptr,token,sizeof(token)-1);
			BitBox_Y = atoi(token);

			// Bounds checking
    		if(BitBox_Width > MAXWIDTH)
			{
				fprintf(stderr,"Font:%d, Bit Width(%d) > MAXWIDTH(%d)!\n", ind, BitBox_Width,MAXWIDTH);
				fprintf(stderr, "%s\n",font->info->FONT_NAME);
				fclose(bdf);
				return(0);
			}
			// Bounds checking
    		if(BitBox_Height > MAXHEIGHT)
			{
				fprintf(stderr,"Font:%d, Bit Height(%d) > MAXHEIGHT(%d)!\n", ind, BitBox_Height,MAXHEIGHT);
				fprintf(stderr, "%s\n",font->info->FONT_NAME);
				fclose(bdf);
				return(0);
			}
		
			// Bounds checking warnings
    		if(BitBox_Width > font->Width)
			{
				fprintf(stderr,"Font:%d, Bit Width(%d) > Font bounding Box Bit Width(%d)!\n", ind, BitBox_Width,font->Width);
				fprintf(stderr, "%s\n",font->info->FONT_NAME);
			}
			// Bounds checking
    		if(BitBox_Height > font->Height)
			{
				fprintf(stderr,"Font:%d, Bit Height(%d) > Font bounding Box Bit Height(%d)!\n", ind, BitBox_Height,font->Height);
				fprintf(stderr, "%s\n",font->info->FONT_NAME);
			}
			continue;
		} 

		x = 0;
		if(MATCH(token, "BITMAP")) 
		{
			if(character < lower || character  > upper)
				continue;

			// Process BITMAP
			font->specs[ind].Offset = offset;
			font->specs[ind].Width = BitBox_Width;
			font->specs[ind].Height = BitBox_Height;
			font->specs[ind].X = BitBox_X;
			font->specs[ind].Y = BitBox_Y;
			font->Bytes = offset;
			max = 0;

			// Read HEX data until ENDCHAR
			hexline = 0;
			addr = 0;
			// SCAN for HEX BITMAP data or end of file 
			while( fgets(line, sizeof(line), bdf) != NULL) 
			{
				trim_tail(line);
				len = strlen(line);
				if(!len)
					continue;

				ptr = get_token(line,token,sizeof(token)-1);
				// Finished this character ?
				if(MATCH(token, "ENDCHAR")) 
				{
					ptr = get_token(ptr,token,sizeof(token)-1);
					break;
				} 

				// Make sure we have only HEX data!
				if(!ishexstr(token))
					break;
				// Parse one BITMAP HEX data line
				ptr = token;
				x = 0;
				// Process data a byte at a time
				while (*ptr) {
					// We already verified the whole string as HEX
					digit = ishex(*ptr);
					mask = 0x08;
					while(mask && x < BitBox_Width)
					{
						addr = x + BitBox_Width * hexline;
						if(mask & digit)
							bitsetxy(bp, x,hexline, BitBox_Width, BitBox_Height);
						else
							bitclrxy(bp, x,hexline, BitBox_Width, BitBox_Height);
						++x;
						mask >>= 1;
					}
					++ptr;
				}	// while ( HEX string ... )
				// Advance to next line
				++hexline;	
			}	// while( reading HEX data ...)
			bytes = ((BitBox_Width * BitBox_Height) + 7) / 8;
			
			offset += bytes;
			// advance bitmap pointer
			bp += bytes;

			// Out of font memory ???
			if (++ind >= Glyphs ) 
			{
				fprintf(stderr, "Too many characters\n");
				fprintf(stderr, "%s\n",font->info->FONT_NAME);
				fclose(bdf);
				return(0);
			}

			// Reset Font Variables
			hexline = 0;
			BitBox_X = 0;
			BitBox_Y = 0;
			BitBox_Width = 0;
			BitBox_Height = 0;
			character = -1;
			Next = font->Width-1;

		}	// if ( BITMAP token ... )
	} // for( reading BDF file ... )


	font->Bytes = offset;
	font->Glyphs = ind;	

	AdjustFontTable(font);

	AddFontName(font);
	fclose(bdf);

	return(font->Glyphs);
}

/**
 @brief Adjust Font offset, renormalize  X and Y to 0
 Readjust font bounding box 
 @param[in] *font: Font pointer
 @return: void
*/

void AdjustFontTable(_font *font)
{
	int i;
    int W= 0;
    int H= 0;
    int X = 0;
    int Y = 0;

// Adjust Font offset, normalize  X and Y to 0
	for(i=0;i<font->Glyphs;++i)
	{
		if(font->specs[i].X < X)
			X = font->specs[i].X;
		if(font->specs[i].Y < Y)
			Y = font->specs[i].Y;
	}

//Apply X and Y offset
	for(i=0;i<font->Glyphs;++i)
	{
		font->specs[i].X -= X;
		font->specs[i].Y -= Y;
	}

	font->X = 0;
	font->Y = 0;
//printf("X:[%d], Y:[%d]\n", X, Y);

// Adjust Font bounding box Width and Height
	for(i=0;i<font->Glyphs;++i)
	{
		if((font->specs[i].Width + font->specs[i].X) > W)
			W = font->specs[i].Width + font->specs[i].X;
		if((font->specs[i].Height + font->specs[i].Y) > H)
			H = font->specs[i].Height + font->specs[i].Y;
	}
// Update W and H bounding box
	font->Width = W;
	font->Height = H;

//printf("W:[%d], H:[%d]\n", W, H);
}


/**
 @brief Adjust font to full size with no offset
 @param[in] *font: Font pointer
 @return: void
*/
void FontAdjustFull(_font *font)
{
	int x,y;
	int w,h;
	int xoff,yoff;
	int bytes;
	int skip;
	int i;
	int offset = 0;

	unsigned char *ptr = font->bitmap;
    unsigned char *save = db_calloc(( font->Glyphs * ((MAXWIDTH+1) + 7) / 8) * (MAXHEIGHT+1));
    unsigned char *bp = save;


	for(i=0;i<font->Glyphs;++i)
	{
		if(font->specs[i].Offset == -1)
			break;

		w = font->specs[i].Width;
		h = font->specs[i].Height;
		xoff = font->specs[i].X;
		yoff = font->specs[i].Y;
		ptr = font->bitmap + font->specs[i].Offset;

		font->specs[i].Offset = offset;

		// Font DATA MSB first
		skip = font->Height-(yoff+h);
		for (y = 0; y < skip; ++y) 
		{
			for (x = 0; x < font->Width; ++x) 
				bitclrxy(bp,x,y, font->Width, font->Height);
		}

		for (y=0;y < h; ++y) {
			for (x = 0; x < xoff; ++x) 
				bitclrxy(bp,x,y+skip, font->Width, font->Height);
			for (x=0;x < w; ++x) {
				if(bittestxy(ptr, x,y, w,h))
					bitsetxy(bp,x+xoff,y+skip, font->Width, font->Height);
				else
					bitclrxy(bp,x+xoff,y+skip, font->Width, font->Height);
			}
			for (x=xoff+w;x < font->Width; ++x) 
				bitclrxy(bp,x+xoff+w,y+skip, font->Width, font->Height);
		}

		// Font DATA MSB first
		for (y = 0; y < yoff; ++y) 
		{
			for (x = 0; x < font->Width; ++x) 
				bitclrxy(bp,x,y+skip+h, font->Width, font->Height);
		}

		bytes = ((font->Width * font->Height) + 7) / 8;

		font->specs[i].Width = font->Width;
		font->specs[i].Height = font->Height;
		font->specs[i].X = 0;
		font->specs[i].Y = 0;
		
		offset += bytes;
		bp += bytes;
	}
	font->X = 0;
	font->Y = 0;
	font->Bytes = offset;
	db_free(font->bitmap);
	font->bitmap = save;
//fprintf(stderr,"Adjusted Bytes:%d\n", font->Bytes);

}

/**
 @brief Ajust font to use smallest font bounding box for each font
  Can be used to converting large fixed fonts to more compact form
  Creates font->specs if missing.
  Updates font->specs with new font size and offsets
 @param[in] *font: Font pointer
 @return: void
*/

void FontAdjustSmall(_font *font)
{
	int X,Y;
	int x,y;
	int w,h;
	int xdelta,ydelta;
	int bytes;
	int skip;
	int i;
	int offset = 0;		// oldbitmap Glyph offset
	int newoffset = 0;	// new bitmap Glyph offset
	int specf;

	int minx,maxx;
	int width,height;
	int miny,maxy;

	unsigned char *ptr = font->bitmap;
	unsigned char *tmp;

	// compute bitmap that can hold worst case size
    unsigned char *new = db_calloc(( font->Glyphs * ((MAXWIDTH+1) + 7) / 8) * (MAXHEIGHT+1));

	// Do we need to create font-specs  
	// This defines each font bounding box and offsets
	if(font->specs)
	{
		specf = 1;
	}
	else
	{
		specf = 0;
        font->specs = db_calloc(sizeof(_fontspecs) * MAXGLYPHS);
	}

	// Scan old font looking for smallest font bounding box

	for(i=0;i<font->Glyphs;++i)
	{
		// Does font->specs exist ?
		if(specf)
		{
			w = font->specs[i].Width;
			h = font->specs[i].Height;
			X = font->specs[i].X;
			Y = font->specs[i].Y;
			offset = font->specs[i].Offset;
		}
		else	/* We must create fixed specs entry it */
		{
			// fixed fonts without specs rely on main font bounding box
			w = font->Width;
			h = font->Height;
			font->specs[i].Width = w;
			font->specs[i].Height = h;
			X = 0;
			Y = 0;
			X = font->specs[i].X;
			Y = font->specs[i].Y;
			offset = i * ((w*h)+7)/8;

			// update specs entry
			// fixed fonts without specs have 0,0 offsets
			font->specs[i].Offset = offset;
		}

		minx = MAXWIDTH+1;
		maxx = 0;
		miny = MAXHEIGHT+1;
		maxy = 0;
		
		// Compute the smallest font bounding box for this Glyph
		// scan font maxy to miny
		for (y=0;y <h; ++y) {
			for (x=0;x < w; ++x) 
			{
				if(bittestxy(font->bitmap+offset, x,y, w,h))
				{
					if(x < minx)
						minx = x;
					if(x > maxx)
						maxx = x;
					if(y < miny)
						miny = y;
					if(y > maxy)
						maxy = y;
				}
		
			}
		}
		
		// maxy has highest Y (near base of Glyph)
		// minx has lowest X (near left of font)
		if(minx > h)
		{
			minx = 0;
			maxx = 0;
			width = 0;
		}
		else
		{
			width = maxx - minx + 1;
		}
		if(miny > h)
		{
			miny = 0;
			maxy = 0;
			height = 0;
		}
		else
		{
			height = maxy - miny + 1;
		}

		// Update new font bounding box 
		font->specs[i].Width = width;
		font->specs[i].Height = height;

		// Compute X and Y shift update to render font correctly
		// With its new bit array
		if(h && height)
		{
			ydelta = (h-1-maxy);
		}
		else
		{
			ydelta = 0;
		}
		xdelta = minx;

		// if new minx is > 0 , bit array can shift minx
		font->specs[i].X += xdelta;

		// if new maxy is < h-1, bit array can shift down
		font->specs[i].Y += ydelta;

		
		// miny has lowest Y
		// minx has lowest X

		// Write new font using updated bonding box
			// minx,miny are the offsets from old bitmap to new 
			// Recall the fonts are *rendered* top down, left right
			// FIXME maybe we should re-render bdf fonts bottom up ???
			//    (this would make these computations easy )
			//  X offsets are measured from the left
			//  Y offsets are measured from the *bottom* of Glyph 
			//   - where bottom is maxy here.
			// minx and miny is the first bit set scanning the
			// old font from top of glyph to bottom (min to max y)
			// So the new bitmap offset is just miny

		for (y=0;y < h; ++y) {
			for (x=0;x < w ; ++x) 
			{
				if(bittestxy(ptr+offset, x,y, w,h))
				{
					bitsetxy(new+newoffset,x-minx,y-miny, width, height);
				}
				else
				{
				}
			}
		}

		// update to new offset
		font->specs[i].Offset = newoffset;
		// Size of this Glyph
		bytes = ((width * height) + 7) / 8;
		// Offset to next Glyph
		newoffset += bytes;
	}	// for()

	// We do not use main font offsets anymore
	font->X = 0;
	font->Y = 0;

	// Total number of bytes is newoffset
	font->Bytes = newoffset;

	// free old bitmap
	db_free(font->bitmap);

	// replace bitmap
	font->bitmap = new;

	// renormalize if needed - should not matter
	AdjustFontTable(font);
}

// ======================================================

/**
  @brief bit set in byte array
  @param[in] *ptr: byte array
  @param[in] addr: bit offset to clear
  @return  void
*/
void bsetv(unsigned char *ptr, int addr)
{
    int byte, bit;
    byte = addr >> 3;
    bit = (addr & 7);
    ptr[byte] |= (0x80 >> bit);
}

/**
  @brief bit clear in byte array
  @param[in] *ptr: byte array
  @param[in] addr: bit offset to clear
  @return  void
*/
void bclrv(unsigned char *ptr, int addr)
{
    int byte, bit;
    byte = addr >> 3;
    bit = (addr & 7);
    ptr[byte]  &= ~(0x80 >> bit);
}

/**
  @brief Test bit in byte array
  @param[in] *ptr: byte array
  @param[in] addr: bit offset to clear
  @return  1 if bit is set, 0 if not
*/
int btestv(unsigned char *ptr, int addr)
{
    int byte, bit;
    byte = addr >> 3;
    bit = (addr & 7);
    return ( ptr[byte]  & (0x80 >> bit) );
}

/**
  @brief Test bit in width * height size bit array usng x and y offsets
  @param[in] *ptr: byte array
  @param[in] x: bit x offset
  @param[in] y: bit y offset
  @param[in] w: the width of bit array
  @param[in] h: the height of bit array
  @return  1 if bit is set, 0 if not
*/
int bittestxy(unsigned char *ptr, int x, int y, int w, int h)
{
    int byte, bit;
    int addr;

    if(y < 0 || y > h)
    {
        return 0;
    }
    if(x < 0 || x > w)
    {
        return 0;
    }
    addr = y * w + x;
    byte = addr >> 3;
    bit = (addr & 7);
    return( (ptr[byte] & (0x80 >> bit)) );
}

/**
  @brief Set bit in width * height size bit array usng x and y offsets
  @param[in] *ptr: byte array
  @param[in] x: bit x offset
  @param[in] y: bit y offset
  @param[in] w: width of bit array
  @param[in] h: height of bit array
  @return  void
*/
void bitsetxy(unsigned char *ptr, int x, int y, int w, int h)
{
    int byte, bit;
    int addr;

    if(y < 0 || y > h)
    {
        return;
    }
    if(x < 0 || x > w)
    {
        return;
    }
    addr = y * w + x;
    byte = addr >> 3;
    bit = (addr & 7);
    ptr[byte] |= (0x80 >> bit);
}

/**
  @brief Clear bit in width * height size bit array usng x and y offsets
  @param[in] *ptr: byte array
  @param[in] x: bit x offset
  @param[in] y: bit y offset
  @param[in] width: width of bit array
  @param[in] height: height of bit array
  @return  void
*/
void bitclrxy(unsigned char *ptr, int x, int y, int width, int height)
{
    int byte, bit;
    int addr;

    if(y < 0 || y > height)
    {
        return;
    }
    if(x < 0 || x > width)
    {
        return;
    }
    addr = y * width + x;
    byte = addr >> 3;
    bit = (addr & 7);
    ptr[byte] &= ~(0x80 >> bit);
}


/**
 @brief Write Font Preview bit bounding box in ASCII character comments
 @param[in] *out: File handle
 @param[in] *font: Font pointer
 @param[in] num: Font character
 @return: void
*/
void FontPreview(FILE * out, _font *font, int num)
{
	int c;
	int x,y;
	int xoff, yoff;
	int w,h;
	int bytes;
	int offset;
	int i, mask;
	unsigned char *ptr = font->bitmap;

	if(num < 0 || num >= font->Glyphs)
		return;

	c = num + font->First;

    if(font->specs)
    {

        offset = font->specs[num].Offset;
        w = font->specs[num].Width;
        h = font->specs[num].Height;
        xoff = font->specs[num].X;
        yoff = font->specs[num].Y;
        bytes = ((w * h) + 7)/8;
        ptr += offset;
    }
    else
    {
        w = font->Width;
        h = font->Height;
        xoff = 0;
        yoff = 0;
        bytes = ((w * h) + 7)/8;
        offset = (bytes * num);
        ptr += offset;
    }


	fprintf(out, "/* index:%d, [%c] 0x%02x, W:% 3d, H:% 3d, X:% 3d, Y:% 3d */\n", 
		num, c, c, 
		w,
		h,
		xoff,
		yoff 
	);

	// Top border
	fprintf(out,"/* |");
	for (x = 0; x < w; ++x)
	{
			fprintf(out,"-");
	}
	fprintf(out,"| */\n");

	// Font DATA MSB first
	for (y = 0; y < h; ++y) {
		fprintf(out,"/* |");
		for (x = 0; x < w; ++x) {
			if(bittestxy(ptr, x,y, w,h))
				fprintf(out,"*");
			else
				fprintf(out," ");
		}
		fprintf(out,"| */\n");
	}

	// Bottom border
	fprintf(out,"/* |");
	for (x = 0; x < w; ++x)
	{
		fprintf(out,"-");
	}
	fprintf(out,"| */\n");
}

/**
 @brief Write Font Preview as full format in ASCII character comments
 @param[in] *out: File handle
 @param[in] *font: Font pointer
 @param[in] num: Font character
 @return: void
*/
void FontPreviewFull(FILE * out, _font *font, int num)
{
	int c;
	int x,y;
	int w,h;
	int xoff,yoff;
	int bytes;
	int offset;
	int i, mask;
	unsigned char *ptr = font->bitmap;

	if(num < 0 || num >= font->Glyphs)
		return;

	c = num + font->First;

	if(font->specs)
	{

		offset = font->specs[num].Offset;
		w = font->specs[num].Width;
		h = font->specs[num].Height;
		xoff = font->specs[num].X;
		yoff = font->specs[num].Y;
		ptr += offset;
	}
	else
	{
		w = font->Width;
		h = font->Height;
		xoff = 0;
		yoff = 0;
		offset = ((w * h) + 7)/8;
		ptr += (offset * num);
	}

	fprintf(out, "/* index:%d, [%c] 0x%02x, W:% 3d, H:% 3d, X:% 3d, Y:% 3d */\n", 
		num, c, c, 
		w,
		h,
		xoff,
		yoff 
	);

	// Top border
	fprintf(out,"/* |");
	for (x = 0; x < font->Width; ++x)
	{
			fprintf(out,"-");
	}
	fprintf(out,"| */\n");

	// Font DATA MSB first
	//for (y = yoff+h; y < font->Height; ++y) 
	for (y = 0; y < font->Height-(yoff+h); ++y) 
	{
		fprintf(out,"/* |");
		for (x = 0; x < font->Width; ++x) 
				fprintf(out," ");
		fprintf(out,"| */\n");
	}

	for (y=0;y< h; ++y) {
		fprintf(out,"/* |");
		for (x = 0; x < xoff; ++x) 
			fprintf(out," ");
		for (x=0;x < w; ++x) {
			if(bittestxy(ptr, x,y, w,h))
				fprintf(out,"*");
			else
				fprintf(out," ");
		}
		for (x=xoff+w;x < font->Width; ++x) 
			fprintf(out," ");
		fprintf(out,"| */\n");
	}

	// Font DATA MSB first
	for (y = 0; y < yoff; ++y) 
	{
		fprintf(out,"/* |");
		for (x = 0; x < font->Width; ++x) 
				fprintf(out," ");
		fprintf(out,"| */\n");
	}

	// Bottom border
	fprintf(out,"/* |");
	for (x = 0; x < font->Width; ++x)
	{
		fprintf(out,"-");
	}
	fprintf(out,"| */\n");
}

/**
 @brief Write Font Preview as proportional format in ASCII character comments
 @param[in] *out: File handle
 @param[in] *font: Font pointer
 @param[in] num: Font character
 @return: void
*/
void FontPreviewProportional(FILE * out, _font *font, int num)
{
	int c;
	int x,y;
	int w,h;
	int xoff,yoff;
	int bytes;
	int offset;
	int i, mask;
	unsigned char *ptr = font->bitmap;

	if(num < 0 || num >= font->Glyphs)
		return;

	c = num + font->First;

    if(font->specs)
    {

        offset = font->specs[num].Offset;
        w = font->specs[num].Width;
        h = font->specs[num].Height;
        xoff = font->specs[num].X;
        yoff = font->specs[num].Y;
        bytes = ((w * h) + 7)/8;
        ptr += offset;
    }
    else
    {
        w = font->Width;
        h = font->Height;
        xoff = 0;
        yoff = 0;
        bytes = ((w * h) + 7)/8;
        offset = (bytes * num);
        ptr += offset;
    }


    fprintf(out, "/* index:%d, [%c] 0x%02x, W:% 3d, H:% 3d, X:% 3d, Y:% 3d */\n",
        num, c, c,
        w,
        h,
        xoff,
        yoff
    );


	// Top border
	fprintf(out,"/* |");
	for (x = 0; x < w+1; ++x)
	{
			fprintf(out,"-");
	}
	fprintf(out,"| */\n");

	// Font DATA MSB first
	//for (y = yoff+h; y < font->Height; ++y) 
	for (y = 0; y < font->Height-(yoff+h); ++y) 
	{
		fprintf(out,"/* |");
		for (x = 0; x < w+1; ++x) 
				fprintf(out," ");
		fprintf(out,"| */\n");
	}

	for (y=0;y< h; ++y) {
		fprintf(out,"/* |");
		for (x=0;x < w; ++x) {
			if(bittestxy(ptr, x,y, w,h))
				fprintf(out,"*");
			else
				fprintf(out," ");
		}
		fprintf(out," ");
		fprintf(out,"| */\n");
	}

	// Font DATA MSB first
	for (y = 0; y < yoff+1; ++y) 
	{
		fprintf(out,"/* |");
		for (x = 0; x < w+1; ++x) 
				fprintf(out," ");
		fprintf(out,"| */\n");
	}

	// Bottom border
	fprintf(out,"/* |");
	for (x = 0; x < w+1; ++x)
	{
		fprintf(out,"-");
	}
	fprintf(out,"| */\n");
}

/**
 @brief Write all Font characters in a font as ASCII character comments
 @param[in] *out: File handle
 @param[in] *font: Font pointer
 @param[in] preview: 1 bit bounding box, 2 Full size, 3 proportional
 @return: void
*/
void WriteFontBitsPreview(FILE * out, _font *font, int preview)
{
	int i;
	int c;

	fprintf(out, "/* Font BIT DATA , MSB Left to Right (padded to byte alignment), Top Down */\n");

	for(i=0;i<font->Glyphs;++i)
	{
		if(preview == 1)
			FontPreview(out, font, i);
		if(preview == 2)
			FontPreviewFull(out, font, i);
		if(preview == 3)
			FontPreviewProportional(out, font, i);
	}
}

