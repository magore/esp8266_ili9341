/**
 @file cordic.h
 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par Edit History
      - [1.0]   [Mike Gore]  Initial revision of file.

 @brief Cordic Routines
 Handle angle outside of the first quadrant
 Added standalone test to verify CORDIC against math library
 Add Documenation and references
 See http://en.wikipedia.org/wiki/CORDIC
See cordic.h, we use fixed point numbers, where 1.0=Cordic_One
Note: 1.0 = 90 degrees
@par Edit History
- [1.0]   [Mike Gore]  Initial revision of file.

This is free software: you can redistribute it and/or modify it under the
terms of the GNU General Public License as published by the Free Software
Foundation, either version 3 of the License, or (at your option)
any later version.

cordic.h is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef _CORDIC_H_
#define _CORDIC_H_
#ifndef TEST
	#include <user_config.h>
#endif
#include "cordic2c_inc.h"

/// @brief Point definition
typedef struct
{
    double x;
    double y;
    double z;
} point;

double angle_quad ( double quads , int *quad );
void Circular ( Cordic_T x , Cordic_T y , Cordic_T z );
void cordic_quad ( double angle , double *s , double *c );
MEMSPACE void cordic_deg ( double deg , double *s , double *c );
MEMSPACE void cordic_rad ( double rad , double *s , double *c );
MEMSPACE void scale_point ( point *p , double scale );
MEMSPACE void shift_point ( point *p , point *shift );
MEMSPACE void rotate ( point *P , point *V );
MEMSPACE void PerspectiveProjection ( point *P );
#endif                                            // _CORDIC_H_
