/**
 @file wire.h

 @brief wireframe view code

 @par Copyright &copy; 2015 Mike Gore, GPL License
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

#ifndef _WIRE_TYPES_H_
#define _WIRE_TYPES_H_

typedef short int wire_t;
typedef struct {
  wire_t x;
  wire_t y;
  wire_t z;
} wire_p;

typedef struct {
	wire_t p1;
	wire_t p2;
} wire_e;

// FIXED point format for int16_t
#define WIRE_ONE  16384 /* 1.0 */
#define WIRE_HALF (WIRE_ONE/2) /* 0.5 */
#define WIRE_SEP 32766  /* pen up */
#define WIRE_END 32767  /* END */
#define WIRE_2FP(a)  ( (double) (a) / (double) (WIRE_ONE))
#define FP2_WIRE(a)  ((wire_t) (WIRE_ONE * (a)))

#endif
