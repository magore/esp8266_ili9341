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

/// @brief CUBE data +/- 0.5 is a cube with sides of 1.0 
//ICACHE_FLASH_ATTR
MEMSPACE_RO
wire_p cube_points[] = {
/* TOP FACE */
	{WIRE_ONE/2,WIRE_ONE/2,WIRE_ONE/2}, 	/*  x, y, z */
	{-WIRE_ONE/2,WIRE_ONE/2,WIRE_ONE/2},	/* -x, y, z */
	{-WIRE_ONE/2,-WIRE_ONE/2,WIRE_ONE/2}, /* -x,-y, z */
	{WIRE_ONE/2,-WIRE_ONE/2,WIRE_ONE/2},	/*  x,-y, z */
/* BOTTOM FACE */
	{WIRE_ONE/2,WIRE_ONE/2,-WIRE_ONE/2},	/*  x, y,-z */
	{-WIRE_ONE/2,WIRE_ONE/2,-WIRE_ONE/2},	/* -x, y,-z */
	{-WIRE_ONE/2,-WIRE_ONE/2,-WIRE_ONE/2},/* -x,-y,-z */
	{WIRE_ONE/2,-WIRE_ONE/2,-WIRE_ONE/2},	/*  x,-y,-z */
	{WIRE_ONE/2,WIRE_ONE/2,-WIRE_ONE/2},	/*  x, y,-z */
};

MEMSPACE_RO
wire_e cube_edges[] = {
/* TOP FACE */
	{ 0,1 },
	{ 1,2 },
	{ 2,3 },
	{ 3,0 },

/* BOTTOM FACE */
	{ 4,5 },
	{ 5,6 },
	{ 6,7 },
	{ 7,4 },

/* Remaining Edges */
	{ 0,4 },
	{ 1,5 },
	{ 2,6 },
	{ 3,7 },

/* END */
	{ -1,-1},
};
	
#else
	extern wire_p cube_points[];
	extern wire_e cube_edges[];
#endif 	// _CUBE_DATA_H_
