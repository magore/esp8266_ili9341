#ifndef _CUBE_DATA_H_
#define _CUBE_DATA_H_

/**
 @file cube_data.h

 @par wireframe viewer
 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief Wireframe CUBE data used by wireframe viewer 
 The code handles fixed, proportional and bounding box format fonts
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option)
 any later version.

 cube_data.h is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef WIRE_ONE
#error WIRE_ONE
#endif

//ICACHE_FLASH_ATTR
MEMSPACE_RO
wire_p wire_cube[] = {
/* TOP FACE */
	{WIRE_ONE,WIRE_ONE,WIRE_ONE}, 	/*  x, y, z */
	{-WIRE_ONE,WIRE_ONE,WIRE_ONE},	/* -x, y, z */
	{-WIRE_ONE,-WIRE_ONE,WIRE_ONE}, /* -x,-y, z */
	{WIRE_ONE,-WIRE_ONE,WIRE_ONE},	/*  x,-y, z */
	{WIRE_ONE,WIRE_ONE,WIRE_ONE}, 	/*  x, y, z */
	{WIRE_SEP,WIRE_SEP,WIRE_SEP },

/* BOTTOM FACE */
	{WIRE_ONE,WIRE_ONE,-WIRE_ONE},	/*  x, y,-z */
	{-WIRE_ONE,WIRE_ONE,-WIRE_ONE},	/* -x, y,-z */
	{-WIRE_ONE,-WIRE_ONE,-WIRE_ONE},/* -x,-y,-z */
	{WIRE_ONE,-WIRE_ONE,-WIRE_ONE},	/*  x,-y,-z */
	{WIRE_ONE,WIRE_ONE,-WIRE_ONE},	/*  x, y,-z */
	{WIRE_SEP,WIRE_SEP,WIRE_SEP },

/* Renaming 4 TOP to BOTTOM edges */
	{WIRE_ONE,WIRE_ONE,WIRE_ONE}, 	/*  x, y, z */
	{WIRE_ONE,WIRE_ONE,-WIRE_ONE},	/*  x, y,-z */
	{WIRE_SEP,WIRE_SEP,WIRE_SEP },

	{-WIRE_ONE,WIRE_ONE,WIRE_ONE},	/* -x, y, z */
	{-WIRE_ONE,WIRE_ONE,-WIRE_ONE},	/* -x, y,-z */
	{WIRE_SEP,WIRE_SEP,WIRE_SEP },

	{-WIRE_ONE,-WIRE_ONE,WIRE_ONE}, /* -x,-y, z */
	{-WIRE_ONE,-WIRE_ONE,-WIRE_ONE},/* -x,-y,-z */
	{WIRE_SEP,WIRE_SEP,WIRE_SEP },

	{WIRE_ONE,-WIRE_ONE,WIRE_ONE}, 	/*  x,-y, z */
	{WIRE_ONE,-WIRE_ONE,-WIRE_ONE},	/*  x,-y,-z */
	{WIRE_SEP,WIRE_SEP,WIRE_SEP },


	{WIRE_SEP,WIRE_SEP,WIRE_SEP },	
	{WIRE_END,WIRE_END,WIRE_END }	/* end */
};
#else
	extern wire_p wire_cube[];
#endif 	// _CUBE_DATA_H_
