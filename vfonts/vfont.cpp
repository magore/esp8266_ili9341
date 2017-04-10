/**
 @file vfont.cpp
 @brief Create C structure from TTF font 
 @par Based on font_to_scg project by Don Bright  
    Copyright (c) 2013, donbright <hugh.m.bright gmail.com>
    @see https://github.com/donbright/font_to_svg
    @see LICENSE 
    @see README.md for technical information 

 @par Copyright &copy; 2017 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
   please retain a copy of this notice in any code you use it in.

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


#include <fstream>
/**
  vfont.hpp is based on https://github.com/donbright/font_to_svg
  @par vfont.hpp Copyright (c) 2013, donbright <hugh.m.bright gmail.com>
  @See LICENSE
  @See README.md for technical information 
  Modified to produce C structures by Mike Gore 2017
*/
#include "vfont.hpp"


int main( int argc, char * argv[] )
{
	const char *fname;
	char name[32];
	int i;

	if (argc!=2) {
		std::cerr << "usage: " << argv[0] << " file.ttf\n";
		exit( 1 );
	}

	fname = argv[1];

	printf("#ifndef _VFONT_H_\n");
	printf("#define _VFONT_H_\n");

printf(
    "typedef struct {\n"
    "    int16_t xoff;   /* X offset */\n"
    "    int16_t yff;    /* Y offset */\n"
    "    int16_t w;      /* Width */\n"
    "    int16_t h;      /* Height */\n"
    "    int16_t xinc;   /* Distance to Next Character by X */\n"
    "    int16_t yinc;   /* Distance to Next Character by Y */\n"
    "    int16_t hby;    /* gm.horiBearingY */\n"
    "    int16_t vby;    /* gm.vertBearingY */\n"
    "    int16_t v[];    /* Data */\n"
    "} path_t;\n"
"\n");



	for(i=32;i<128;++i)
	{
		sprintf(name,"0x%02x",i);
		font2svg::glyph g( fname, name );
		std::cout << g.outline();
		g.free();
		if(i == 32)
		{
			printf("\t{ '.' }\n");
			printf("};\n");
		}
	}

	printf("path_t *vfont[]= {\n");
	for(i=32;i<128;++i)
	{
		printf("\t&_vec%d,\n",i);
	}
	printf("};\n");
	printf("#endif /* _VFONT_H_ */\n");

  return 0;
}
