#ifndef _WIRE_H_
#define _WIRE_H_
/**
 @file wireframe.c

 @par wireframe viewer
 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief Display wireframe object 
 The code handles fixed, proportional and bounding box format fonts
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option)
 any later version.

 wireframe.c is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

typedef short int wire_t;
typedef struct {
  wire_t x;
  wire_t y;
  wire_t z;
} wire_p;

#define WIRE_ONE  16384 /* 1.0 */
#define WIRE_SEP 32766  /* pen up */
#define WIRE_END 32767  /* END */
#define WIRE_2FP(a)  ( (double) (a) / (double) (WIRE_ONE))
#define FP2_WIRE(a)  ((wire_t) (WIRE_ONE * (a)))

#endif
