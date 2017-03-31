/**
  @file matrix.h

 @brief matrix functions

 @par Copyright &copy; 2016 Mike Gore, GPL License
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

#ifndef _MATRIX_H_
#define _MATRIX_H_

typedef struct _mat {
    float **data;
	int cols;
	int rows;
    int size;
} mat_t;



/* matrix.c */
int TestSquare ( mat_t MatA );
mat_t MatAlloc ( int rows , int cols );
mat_t MatAllocSQ ( int size );
void MatFree ( mat_t matF );
mat_t MatLoad ( void *V , int rows , int cols );
mat_t MatLoadSQ ( void *V , int size );
void MatPrint ( mat_t matrix );
mat_t DeleteRowCol ( mat_t MatA , int row , int col );
mat_t Transpose ( mat_t MatA );
float Minor ( mat_t MatA , int row , int col );
float Cofactor ( mat_t MatA , int row , int col );
mat_t Adjugate ( mat_t MatA );
float Determinant ( mat_t MatA );
mat_t Invert ( mat_t MatA );
mat_t PseudoInvert ( mat_t MatA );
mat_t MatMul ( mat_t MatA , mat_t MatB );
mat_t MatRead ( char *name );

#endif // _MATRIX_H_
