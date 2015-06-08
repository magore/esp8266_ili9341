/**
 @file cordic.h

 @brief Cordic Routines
 Handle angle outside of the first quadrant
 Added standalone test to verify CORDIC against math library
 Add Documenation and references
 @see http://en.wikipedia.org/wiki/CORDIC
 @see cordic.h, we use fixed point numbers, where 1.0=Cordic_One
 Note: 1.0 = 90 degrees

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



#ifndef _CORDIC_H_
#define _CORDIC_H_

/// @brief Point definition
typedef struct
{
    double x;
    double y;
    double z;
} point;

/* cordic/cordic.c */
MEMSPACE double deg2rad ( double d );
double angle_quad ( double quads , int *quad );
void Circular ( Cordic_T x , Cordic_T y , Cordic_T z );
MEMSPACE void cordic_quad ( double angle , double *s , double *c );
MEMSPACE void cordic_deg ( double deg , double *s , double *c );
MEMSPACE void cordic_rad ( double rad , double *s , double *c );
MEMSPACE void scale_point ( point *P , double scale );
MEMSPACE void shift_point ( point *P , point *shift );
MEMSPACE void rotate ( point *P , point *V );
MEMSPACE void PerspectiveProjection ( point *P , double scale , int x , int y );

#endif // _CORDIC_H_
