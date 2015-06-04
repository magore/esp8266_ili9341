
/**
 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief BDF to C code converter Copyright &copy; 2015 Mike Gore

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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <string.h>
#include <limits.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>

#include "font.h"
#include "bdffontutil.h"

/**
  @brief Display Usage
  @param[in] *prog: program name 
  @return void
*/
void usage(char *prog)
{
	printf("%s: [-o font_output_file ] [ -l lower_bound ] [ -u upper_bound ] [ -preview ] [ -Preview ] bdf files...\n",
		prog);
	printf(" -o: output file containg C STructures of converted font data\n");
	printf(" -u: limit Glyph processing above limit\n");	
	printf(" -l: limit Glyph processing below limit\n");	
	printf(" -p 1 Preview font bitmap data - only parts with 1 bits set\n");	
	printf(" -p 2 Preview Full font bitmap data - everything inside font bounding box\n");	
	printf(" -p 3 Preview Proportional font bitmap data and fixed - everything inside font bounding box\n");	
	printf(" -f convert any font to fixed size bitmap - everything inside font bounding box\n");
	printf(" -s Compact font to smallest bounding box\n");
	printf("    Notes: modifies or creates font specs\n");
	printf(" bdf files...: One of more BDF format font files\n");
}

/**
 @brief convert BDF fonts to C structures

 bdffonf2c [-o font_output_file ] [ -l lower_bound ] [ -u upper_bound ] [ -preview ] [ -Preview ] bdf files..."
	pro
 -o: output file containg C STructures of converted font data
 -u: limit Glyph processing above limit");
 -l: limit Glyph processing below limit");
 -p 1 Preview font bitmap data - only parts with 1 bits set");
 -p 2 Preview Full font bitmap data - everything inside font bounding box");
 -p 3 Preview Proportional font bitmap data and fixed - everything inside font bounding box");
 -f convert any font to fixed size bitmap - everything inside font bounding box
 -s Compact font to smallest bounding box
    Notes: modifies or creates font specs
 bdf files...: One of more BDF format font files
*/

int main(int argc, char *const argv[])
{
	int i;
	struct stat buf;
	_font fx;
    char *name, *cfonts, *ptr;;
	// Dump font ascii preview
	int font_preview = 0;
	int font_adjust_full = 0;
	int font_adjust_small = 0;
	// Glyph code of first font to process - we ignore fonts less then this
	int lower_bound = 32;
	// Glyph code of last font to process - we ignore fonts greater then this 
	int upper_bound = 127;

	// We always emit fonts specs and info parts
	// These flags just determin if we emit them in the converted font file
	int fontinfo_f = 0;
	int fontspecs_f = 0;

	char str[MAXLINE];

	int bdfind = 0;
	int find = 0;
	FILE *FI;

	FILE *FO = NULL;
	cfonts = NULL;

	for(i=1;i<argc;++i)
	{
		ptr = argv[i];
		if(*ptr == '-')
		{
			ptr++;
			if(*ptr == 'o')
			{
				++ptr;
				if(*ptr)
				{
					cfonts = ptr;
				}
				else
				{
					cfonts = argv[++i];

				}
				continue;
			}
			if(*ptr == 'l')
			{
				++ptr;
				if(*ptr)
				{
					lower_bound = atoi(ptr);
				}
				else
				{
					lower_bound = atoi(argv[++i]);
				}
				continue;
			}
			if(*ptr == 'u')
			{
				++ptr;
				if(*ptr)
				{
					upper_bound = atoi(ptr);
				}
				else
				{
					upper_bound = atoi(argv[++i]);
				}
				continue;
			}
			if(*ptr == 'p')
			{
				++ptr;
				if(*ptr)
				{
					font_preview = atoi(ptr);
				}
				else
				{
					font_preview = atoi(argv[++i]);
				}
				continue;
			}
			if(*ptr == 'f')
			{
				++ptr;
				font_adjust_full = 1;
				continue;
			}
			if(*ptr == 's')
			{
				++ptr;
				font_adjust_small = 1;
				continue;
			}
		}
		else
		{
			if(find < MAXFONTS)
			{
				if(stat(argv[i], (struct stat *) &buf) != 0)
				{
					fprintf(stderr,"File:[%s] missing, skipping\n", argv[i]);
					continue;
				}
				fnames[find++] = stralloc(argv[i]);
			}
		}
	}

	if(font_adjust_full && font_adjust_small)
	{
		fprintf(stderr,"Can not have -f and -s set at the same time\n");	
		exit(1);
	}

	if(!find)
	{
		usage(basename(argv[0]));
		fprintf(stderr,"No font files specified\n");
		exit(1);
	}

	if(cfonts == NULL)
	{
		FO = stdout;
		cfonts = "stdout";
	}
	else
	{
        if(stat(cfonts, (struct stat *) &buf) == 0)
		{
			fprintf(stderr,"Font file: %s - exists - not overwritting\n", cfonts);
			exit(1);

		}
		FO = fopen(cfonts,"w");
		if(!FO)
		{
			fprintf(stderr,"Can't open: %s\n", cfonts);
			exit(1);
		}
	}

	
	bdfind = 0;
	for(i=0;i<find;++i)
	{
		int ret;
		name = fnames[i];

		if(!ReadBdf(name, &fx, lower_bound, upper_bound))
		{
			FreeFont(&fx);
			fprintf(stderr,"Can't open:[%s]\n", name);
			continue;
		}
		ret = FindFontName(fx.info->STRUCT_NAME);
		if(ret != -1)
		{
			fprintf(stderr,"Duplicate font:[%s] in file:[%s], skipping\n", fx.info->STRUCT_NAME, name);
			fprintf(stderr,"Exists in font:[%s] in file:[%s]\n\n", 
				BDFnames[ret].structname,
				BDFnames[ret].filename
			);
			FreeFont(&fx);
			continue;
		}

	
		if(font_adjust_full)
		{
			//fprintf(stderr,"Bytes:%d\n", fx.Bytes);
			FontAdjustFull(&fx);
			fontspecs_f = 0;
			//fprintf(stderr,"New Bytes:%d\n", fx.Bytes);
		}

		if(font_adjust_small)
		{
			//fprintf(stderr,"Bytes:%d\n", fx.Bytes);
			FontAdjustSmall(&fx);
			fontspecs_f = 1;
			//fprintf(stderr,"New Bytes:%d\n", fx.Bytes);
		}

		BDFnames[bdfind].filename = name;
		BDFnames[bdfind].structname = stralloc(fx.info->STRUCT_NAME);

		
		FontHeaderInfo(FO, &fx, argv[0], cfonts);

		if(fontinfo_f = 0)
		{
			fprintf(FO,"#define FONTINFO\n");
		}
		if(fontspecs_f)
		{
			fprintf(FO,"#define FONTSPECS\n");
		}

		Convert_Font2c(FO, &fx);
		WriteFontBitsPreview ( FO, &fx, font_preview);
		FreeFont(&fx);
		++bdfind;
	}

	fprintf(FO, "\n\n\n");
	fprintf(FO, "/* All cfonts */\n");
	fprintf(FO, "_font *allfonts[%d] = {\n", bdfind+1);
	for(i=0;i<bdfind;++i)
	{
		fprintf(FO, "\t&%s,\n", BDFnames[i].structname);
	}
	fprintf(FO, "\tNULL\n");
	fprintf(FO, "};\n");

	if(FO != stdout)
		fclose(FO);
	return(0);
}
