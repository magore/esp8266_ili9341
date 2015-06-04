/**
 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief BDF C code test/preview tool

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the 
 terms of the GNU General Public License as published by the Free Software 
 Foundation, either version 3 of the License, or (at your option)
 any later version.

 bdfview is distributed in the hope that it will be useful,
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
#include <string.h>
#include <limits.h>
#include <errno.h>

#include "font.h"
#include "bdffontutil.h"

#define FONTINFO
#define FONTSPECS
#include "fonts.h"


main(int argc, char *argv[])
{
	char *ptr;
	int i;
	int preview = 1;
	int fontind = 0;
	int numfonts = 0;
	int table = 0;
	int bits = 0;
	_font *p;

	for(numfonts=0;allfonts[numfonts] != NULL;++numfonts)
		;

	printf("/* We have (%d) fonts */\n",numfonts);
	for(i=1;i<argc;++i)
	{
		ptr = argv[i];
		if(*ptr == '-')
		{
			++ptr;
			if(*ptr == 't')
			{
				table = 1;
				continue;
			}
			if(*ptr == 'b')
			{
				bits = 1;
				continue;
			}
			if(*ptr == 'p')
			{
				++ptr;
				if(*ptr)
				{
					preview = atoi(ptr);
				}
				else
				{
					preview = atoi(argv[++i]);
				}
				continue;
			}
			if(*ptr == 'n')
			{
				++ptr;
				if(*ptr)
				{
					fontind = atoi(ptr);
				}
				else
				{
					fontind = atoi(argv[++i]);
				}
				continue;
			}
		}
	}
	if(fontind >= numfonts)
	{
		fprintf(stderr, "font index >= %d\n", numfonts);
		exit(1);

	}
	p = allfonts[fontind];
	FontHeaderInfo( stdout, p, argv[0], NULL);

	WriteFontInfo( stdout, p);

	Convert_Font2c(stdout, p);

#ifdef JUNK
    if(bits)
		WriteFontBits ( stdout, p);
    if(table)
		WriteFontTable ( stdout, p);
#endif

	WriteFontBitsPreview(stdout, p,preview);
}
