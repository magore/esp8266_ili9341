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
	char tmp[256];
	int i;

	if (argc!=2) {
		std::cerr << "usage: " << argv[0] << " file.ttf\n";
		exit( 1 );
	}

	fname = argv[1];

	printf("#ifdef VFONTS\n");
	printf("#ifndef _VFONTS_H_\n");
	printf("#define _VFONTS_H_\n");
	printf("// Font file: %s\n", fname);

	for(i=32;i<128;++i)
	{
		printf("//MEMSPACE_FONT\n");
		sprintf(name,"0x%02x",i);
		font2svg::glyph g( fname, name );
		if(i == 32)
			strcpy(tmp,g.facename(tmp,256));
		std::cout << g.outline();
		g.free();
	}

	printf("path_t *vfont[]= {\n");
	for(i=32;i<128;++i)
	{
		printf("\t&%s_glyph_%02x,\n", tmp,i);
	}
	printf("\tNULL\n");
	printf("};\n");
	printf("#endif /* _VFONTS_H_ */\n");
	printf("#endif // VFONTS\n");

  return 0;
}
