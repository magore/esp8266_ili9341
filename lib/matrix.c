/**
  @file matrix.c

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
#include "user_config.h"

#include "matrix.h"

///@brief Credits: https://www.cs.rochester.edu/~brown/Crypto/assts/projects/adj.html
///@author Paul Bourke 2002


/**
  @brief Test is a matrix is square
  @param[in] MatA: matrix A 
  @return 1 if ssquare, 0 if not
*/
MEMSPACE
int TestSquare(mat_t MatA)
{
	return( (MatA.rows == MatA.cols) ? 1 : 0 );
}

/**
  @brief Allocate a matrix
  @param[in] size: size of square matrix to allocate 
  @return float **
*/
MEMSPACE
mat_t MatAlloc(int rows, int cols)
{
	int r;
	mat_t MatA;
	float *fptr;

	if(rows < 1)
	{
#if MATDEBUG & 1
		//FIXME
		printf("MatAlloc: rows < 1\n");
#endif
		rows = 1;
	}
	if(cols< 1)
	{
#if MATDEBUG & 1
		//FIXME
		printf("MatAlloc: cols < 1\n");
#endif
		cols = 1;
	}
	
	MatA.data = safecalloc(rows,sizeof(float *));
	if(MatA.data == NULL)
	{
		MatA.rows = 0;
		MatA.cols = 0;
		return(MatA);
	}

	for (r=0;r<rows;r++)
	{
		fptr = safecalloc(cols,sizeof(float));
		if(fptr == NULL)
		{
			MatFree(MatA);
			return(MatA);
		}

		MatA.data[r] = fptr;
	}
	MatA.rows = rows;
	MatA.cols = cols;
	if(cols == rows)
		MatA.size = cols;
	else
		MatA.size = 0;
	return(MatA);
}

/**
  @brief Allocate a matrix
  @param[in] size: size of square matrix to allocate 
  @return float **
*/
MEMSPACE
mat_t MatAllocSQ(int size)
{
	return(MatAlloc(size,size));
}


/**
  @brief Free a matrix
  @param[in] **Mat: Matrix to free
*/
MEMSPACE
void MatFree(mat_t matF)
{
	int i;
	if(matF.data) 
	{
		for (i=0;i<matF.rows;i++)
		{
			if(matF.data[i])
			{
				safefree(matF.data[i]);
				matF.data[i] = NULL;
			}
			else
			{
#if MATDEBUG & 1
				printf("MatFree: attempt to free null matF row: %d\n", i);
#endif
			}
		}
		safefree(matF.data);
		matF.data = NULL;
	}
	else
	{
#if MATDEBUG & 1
		printf("MatFree: attempt to free null matF\n");
#endif
	}
}

/**
  @brief Load a matrix
  @param[in] *V: matrix data 
  @param[in] size: size of square matrix
*/
MEMSPACE
mat_t MatLoad(void *V, int rows, int cols)
{
	mat_t MatA = MatAlloc(rows,cols);
	float *f = (float *) V;

	int r,c,k;
	
	k = 0;
	for(r=0;r<rows;++r)
	{
		for(c=0;c<cols;++c)
		{
			MatA.data[r][c] = f[k++];
		}
	}
	return(MatA);
}

/**
  @brief Load a square matrix
  @param[in] *V: square matrix data 
  @param[in] size: size of square matrix
*/
MEMSPACE
mat_t MatLoadSQ(void *V, int size)
{
	return(MatLoad(V,size,size));
}


/**
  @brief Print a matrix
  @param[in] Mat: Matrix to print
*/
MEMSPACE
void MatPrint(mat_t matrix)
{
	int r,c;
	
	printf("size: rows(%d), cols(%d)\n", matrix.rows,matrix.cols);
	for(r=0;r<matrix.rows;++r)
	{
		for(c=0;c<matrix.cols;++c)
		{
			printf("%+e ", (double) matrix.data[r][c]);
		}
		printf("\n");
	}
	printf("\n");
}

/**
  @brief Create smaller matrix by deleatting specified row and colume
  Used by Minor and Cofactor
  @param[in] MatA: matrix A - row and col must be >= 2
  @param[in] row: row to delete
  @param[in] col: col to delete
  @return submatrix 
*/
MEMSPACE
mat_t DeleteRowCol(mat_t MatA,int row,int col)
{
	int r,c;
	int rM, cM;
	mat_t MatM;
	int rows = MatA.rows;
	int cols = MatA.cols;
	if(cols < 2)
	{
		//FIXME user error
		cols = 2;
#if MATDEBUG & 1
		printf("DeleteRowCol cols:%d < 2 error\n",cols);
#endif
	}
	if(rows < 2)
	{
		//FIXME user error
		rows = 2;
#if MATDEBUG & 1
		printf("DeleteRowCol rows:%d < 2 error\n",rows);
#endif
	}

	MatM = MatAlloc(rows-1,cols-1);
	rM = 0;
	for(r=0;r<MatA.rows;++r)
	{
		// delete row
		if(r == row)
			continue;
		cM = 0;
		for(c=0;c<MatA.cols;++c)
		{
			// delete col
			if(c == col)
				continue;
			MatM.data[rM][cM] = MatA.data[r][c];
			++cM;
		}
		++rM;
	}
	return(MatM);
}

/**
  @brief Transpose matrix 
  @param[in] MatA: matrix A 
  @return Transpose matrix
*/
MEMSPACE
mat_t Transpose(mat_t MatA)
{
    int c,r;
	// allocate using transposed rows and columns
	mat_t MatR = MatAlloc(MatA.cols, MatA.rows);
	// row
	for (r = 0; r < MatA.rows; r++)
	{
		// col
		for( c = 0 ; c < MatA.cols ; c++ )
		{
			// transposed
			MatR.data[c][r] = MatA.data[r][c];
		}
	}
	return(MatR);
}

/**
  @brief Compute determinate of the minor submatrix 
  Minor submatrix has one less row and column as a result
  @see https://en.wikipedia.org/wiki/Minor_(linear_algebra)
  @param[in] MatA: matrix A
  @param[in] row: row to delete
  @param[in] col: col to delete
  @return Determinate
*/
MEMSPACE
float Minor(mat_t MatA, int row, int col)
{
	float D;
	mat_t SubMat = DeleteRowCol(MatA, row, col);
	D = Determinant(SubMat);
	MatFree(SubMat);
	return(D);
}

/**
  @brief Cofactor is determinate of minor submatrix * (-1)exp(row+col)
  Minor submatrix has one less row and column as a result
  @see https://en.wikipedia.org/wiki/Cofactor
  @param[in] MatA: matrix A
  @param[in] row: row to delete
  @param[in] col: col to delete
  @return Determinate * (-1)exp(row + col)
*/
MEMSPACE
float Cofactor(mat_t MatA, int row, int col)
{
	float D = Minor(MatA, row, col);;
	// (-1)exp(row+col)  is -1 if (row+col) odd otherwise 1.0 if even
	D *= ( ((row+col) & 1) ? -1.0 : 1.0);
	return(D);
}

/**
  @brief Adjugate is transpose of cofactor matrix of A
  @see https://en.wikipedia.org/wiki/Adjugate_matrix
  @param[in] MatA: matrix A
  @return Adjugate or A
*/
MEMSPACE
mat_t Adjugate(mat_t MatA)
{
	int r,c;
	// Since the result is the transpose we allocate switching rows and cols here
	mat_t MatAdj = MatAlloc(MatA.cols,MatA.rows);

	for(r = 0; r< MatA.rows;++r)
	{
		for(c = 0; c < MatA.cols;++c)
		{
			// transpose rows and columns when storing Cofactors to get the Adjugate
			MatAdj.data[c][r] = Cofactor(MatA,r,c);
		}
	}
	return(MatAdj);
}

	
/**
  @brief Determinant by recursion using Cofactors
  @see https://en.wikipedia.org/wiki/Determinant
  @param[in] MatA: square matrix A
  @return Determinant or 0
*/
MEMSPACE
float Determinant(mat_t MatA)
{
    int r,c,n,cc;
    float D = 0;

	if(MatA.cols != MatA.rows)
	{
		//FIXME error
#if MATDEBUG & 1
		printf("Determinate: Matrix MUST be square!\n");
#endif
        return(D);
	}

    if (MatA.size < 1)
    {
#if MATDEBUG & 1
		printf("Determinate: Matrix size MUST be > 0!\n");
#endif
        return(D);
    }
	// 1 x 1 case
	if (MatA.size == 1)	 
    {
        D = MatA.data[0][0];
		return(D);
    }
	// 2 x 2 case
    if (MatA.size == 2)
    {
        D = MatA.data[0][0] * MatA.data[1][1] - MatA.data[1][0] * MatA.data[0][1];
		return(D);
    }
	// solve > 2 cases by recursive Cofactor method
	for (n=0;n<MatA.size;++n)
	{
		D += MatA.data[0][n] * Cofactor(MatA, 0, n);
	}
    return(D);
}
 
/**
  @brief Calculate Matrix Inverse
  @see https://en.wikipedia.org/wiki/Invertible_matrix
  Method used: Adjugate(MatA) / Det(MatA)
  @param[in] MatA: square matrix A input
  @return Inverse of MatA or error
*/
MEMSPACE
mat_t Invert(mat_t MatA)
{
	int r,c;
	float D;

	mat_t MatAdj;
 
#if MATDEBUG & 2
	printf("MatA\n");
	MatPrint(MatA);
#endif

	D=Determinant(MatA);

	if(!D)
	{
		//FIXME flag error somehow
#if MATDEBUG & 1
		printf("Determinant(MatA) = 0!\n\n");
#endif
		return(MatA);
	}

#if MATDEBUG & 2
	printf("Determinant(MatA):\n%e\n\n", (double) D);
#endif

	MatAdj = Adjugate(MatA);
#if MATDEBUG & 2
	printf("Adjugate(MatA)\n");
	MatPrint(MatAdj);
#endif
	// row
	for (r=0;r<MatAdj.rows;++r)
    {
		// col
		for (c=0;c<MatAdj.cols;++c)
		{
        	MatAdj.data[r][c] /= D;
        }
    }
#if MATDEBUG & 2
	printf("Adjugate(MatA)/Determinant(MatA)\n");
	MatPrint(MatAdj);
#endif

	return(MatAdj);
}

/**
  @brief Calculate Pseudo Matrix Inverse
  Used for least square fitting of non square matrix with excess solutions
  Pseudo Inverse matrix(A) = 1/(AT × A) × AT
  @param[in] MatA: matrix A input - does not have to be square
  @return Pseudo Inverse of MatA or error
*/
MEMSPACE
mat_t PseudoInvert(mat_t MatA)
{
	// AT = Transpose(A)
	mat_t MatAT = Transpose(MatA);

	// AT * A
	mat_t MatR = MatMul(MatAT,MatA);

	// 1/(AT * A)
	mat_t MatI = Invert(MatR);

	// Pseudo Inverse (AT × A)–1 × AT\n
	mat_t MatPI = MatMul(MatI,MatAT);

	MatFree(MatR);
	MatFree(MatI);
	MatFree(MatAT);
	
	return(MatPI);
}


/**
  @brief Multiply two matrix
 @see https://en.wikipedia.org/wiki/Matrix_multiplication_algorithm
  C = AB, A is n × m matrix, B is m × p matrix 
  C result n × p matrix  (dimensions row size of A, column size of B)
  Cij = Sum(k=1 .. m) Aik * Bik
  @param[in] MatA:  matrix A
  @param[in] MatB:  matrix B
  @return MatA * MatB
  Result dimensions is (row size of A, column size of B)
*/
MEMSPACE
mat_t MatMul(mat_t MatA, mat_t MatB)
{
	float sum = 0;
	mat_t MatR = MatAlloc(MatA.rows,MatB.cols);

	int rA,cB,rB;

	if (MatA.cols != MatB.rows)
#if MATDEBUG & 1
		printf("error MatA cols(%d) != MatB rows(%d)\n", MatA.cols, MatB.rows);
#endif
	
	// A row
    for (rA = 0; rA < MatA.rows; ++rA) 
	{
		// col B
		for (cB = 0; cB < MatB.cols; ++cB) 
		{
			// row B
			for (rB = 0; rB < MatB.rows; ++rB) 
			{
				sum += (MatA.data[rA][rB] * MatB.data[rB][cB]);
			}
			MatR.data[rA][cB] = sum;
			sum = 0;
		}
    }
	return(MatR);
}

/**
  @brief Read a matrix
  @param[in] *name: matrix data 
  @return mat_t matrix data
  Note: on error rows and cols = 0, data = NULL;
*/
MEMSPACE
mat_t MatRead(char *name)
{

	FILE *fp;
	char *ptr;
	int r,c;
	int rows = 0;
	int cols = 0;
	int cnt;
	int lines = 0;
	mat_t MatR;
	char tmp[128];

	MatR.rows = 0;
	MatR.cols = 0;
	MatR.data = NULL;

	fp = fopen(name,"r");
	if(fp == NULL)
	{
		return(MatR);
	}

	// Read Matrix header with rows and columns
	ptr = fgets(tmp,253,fp);
	if(ptr == NULL)
	{
		fclose(fp);
		return(MatR);
	}
	//printf("line:%d, %s\n", lines, tmp);
	++lines;
	cnt = sscanf(ptr,"Matrix R:%d C:%d", (int *) &rows, (int *) &cols);
	if(rows < 1 || cols < 1)
	{
		printf("sscanf: %d\n",cnt);
		printf("MatRead expected header: Matrix R:%d C:%d\n",tmp);
		fclose(fp);
		return(MatR);
	}

	// FIXME set limts or just let alloc fail ???
	MatR = MatAlloc(rows,cols);
	if(MatR.data == NULL)
	{
		printf("MatRead(%s: &d,&d) could not alloc memory\n", name, rows,cols);
		fclose(fp);
		return(MatR);
	}
	
	for(r=0;r<rows;++r)
	{
		// Read rows and columns
		ptr = fgets(tmp,253,fp);
		//printf("line:%d, %s\n", lines, tmp);
		++lines;
		if(ptr == NULL)
		{
			fclose(fp);
			MatFree(MatR);
			return(MatR);
		}
		for(c=0;c<cols;++c)
		{
			MatR.data[r][c] = strtod(ptr,&ptr);
		}
	}
	fclose(fp);
	return(MatR);
}

/**
  @brief Write a matrix
  @param[in] *name: matrix data 
  @param[in] MatW: Matrix
  @return status 1 = success, 0 = fail
*/
MEMSPACE
int MatWrite(char *name, mat_t MatW)
{

	FILE *fp;
	int r,c;

	fp = fopen(name,"w");
	if(fp == NULL)
	{
		return(0);
	}

	fprintf(fp,"Matrix R:%d C:%d\n",MatW.rows,MatW.cols);
	for(r=0;r<MatW.rows;++r)
	{
		for(c=0;c<MatW.cols;++c)
		{
			fprintf(fp,"%e ", (double) MatW.data[r][c]);
		}
		fprintf(fp,"\n");
	}
	fclose(fp);
	return(1);
}

	
#ifdef MATTEST
// ==================
// test1
// compute Adj(AJ)

float AJ[3][3] =
{
	{ -3, 2, -5 },
	{ -1, 0, -2 },
	{  3, -4, 1 }
};

// ==================
// test2
// Matrix Multiplication
// compute C * D

float C[3][3] = 
{
	{ 1,2,0 },
	{ 0, 1, 1 },
	{ 2, 0, 1 }
};
float D[3][3] = 
{
	{ 1, 1, 2 },
	{ 2, 1, 1 },
	{ 1, 2, 1 }
};

// ==================
// test3
// Test functions required for 3 point screen calibration
// See: https://www.maximintegrated.com/en/app-notes/index.mvp/id/5296
//      Calibration in touch-screen systems
//
// Calculation
// 1/A * X
// 1/A * Y
//
// Answer should be
// xd = 0.0635 x + 0.0024 y + 18.9116
// yd = -0.0227 x + 0.1634 y + 37.8887
// 	Where (x, y) are touch panel coordinates 
// 	and (xd, yd) is the adjusted screen coordinate 
float A3[3][3] = {
	{ 650, 2000, 1 },
	{ 2800, 1350, 1 },
	{ 2640, 3500, 1 }
};

float X3[3][1] = 
{ 
	{65},
	{200},
	{195}
};

float Y3[3][1] = 
{ 	
	{350},
	{195},
	{550}
};

// ==================
// test4
// Test functions required for 5 point screen calibration
// See: https://www.maximintegrated.com/en/app-notes/index.mvp/id/5296
//      Calibration in touch-screen systems
//
// Calculation - note: AT = transpose(A)
// (1/(AT * A) * AT) * X
// (1/(AT * A) * AT) * Y
//
// Answer should be
// 	xd = 0.0677 x + 0.0190 y - 33.7973
// 	yd = -0.0347 x + 0.2100 y - 27.4030
// 	Where (x, y) are touch panel coordinates 
// 	and (xd, yd) is the adjusted screen coordinate 


float A5[5][3] = {
	{ 1700, 2250, 1 },
	{ 750, 1200, 1 },
	{ 3000, 1500, 1 },
	{ 2500, 3400, 1 },
	{ 600, 3000, 1 }
};

float X5[5][1] = 
{ 
	{100},
	{50},
	{200},
	{210},
	{65}
};

float Y5[5][1] = 
{ 	
	{350},
	{200},
	{200},
	{600},
	{600}
};

int main(int argc, char *argv[])
{
	mat_t MatA,MatX,MatY;
	mat_t MatI;
	mat_t MatPI;
	mat_t MatC,MatD;
	mat_t MatR;
	mat_t MatAdj;


	// =============================
	// test Adjugate
	printf("==========================================\n");
	MatA = MatLoad(AJ,3,3);
		printf("MatA\n");
		MatPrint(MatA);
	MatAdj = Adjugate(MatA);
		printf("Adjugate(MatA)\n");
		MatPrint(MatAdj);
	MatFree(MatAdj);
	MatFree(MatA);
	printf("==========================================\n");
	printf("\n");

	// =============================
	// test MatMul
	printf("==========================================\n");
	MatC = MatLoadSQ(C,3);
		printf("MatC\n");
		MatPrint(MatC);
	MatD = MatLoadSQ(D,3);
		printf("MatD\n");
		MatPrint(MatD);
	MatR = MatMul(MatC,MatD);
		printf("MatC * MatD\n");
		MatPrint(MatR);
	MatFree(MatC);
	MatFree(MatD);
	MatFree(MatR);
	printf("==========================================\n");
	printf("\n");

	// ===============================================================
	// ===============================================================
	// Test functions required for 3 point screen calibration
	// See: https://www.maximintegrated.com/en/app-notes/index.mvp/id/5296
	//      Calibration in touch-screen systems
	printf("==========================================\n");
	printf("Set of three display positions\n");
	MatX = MatLoad(X3,3,1);
		printf("X\n");
		MatPrint(MatX);

	MatY = MatLoad(Y3,3,1);
		printf("Y\n");
		MatPrint(MatY);

	printf("Correponding touch screen positions, differing scale and skew\n");
	MatA = MatLoadSQ(A3,3);
		printf("A\n");
		MatPrint(MatA);

	// 1/A
	MatI = Invert(MatA);
		printf("1/(A)\n");
		MatPrint(MatI);
	MatFree(MatA);

	printf("Solution Matrix to translate touch screen to screen positions\n");
	// MatR = 1/A * X
	MatR = MatMul(MatI,MatX);
		printf("1/A * X\n");
		MatPrint(MatR);
	MatFree(MatR);
	MatFree(MatX);

	// MatR = 1/A * Y
	MatR = MatMul(MatI,MatY);
		printf("1/A * Y\n");
		MatPrint(MatR);
	MatFree(MatR);
	MatFree(MatY);

	MatFree(MatI);
	printf("==========================================\n");
	printf("\n");

	// ===============================================================
	// ===============================================================
	// Test functions required for N point screen calibration
    // Use least square solution by Pseudo Invert method
	// See: https://www.maximintegrated.com/en/app-notes/index.mvp/id/5296
	//      Calibration in touch-screen systems
	printf("==========================================\n");
	printf("Set of five display positions\n");
	MatX = MatLoad(X5,5,1);
		printf("X\n");
		MatPrint(MatX);

	MatY = MatLoad(Y5,5,1);
		printf("Y\n");
		MatPrint(MatY);

	printf("Correponding touch screen positions, differing scale and skew\n");
	MatA = MatLoad(A5,5,3);
		printf("A\n");
		MatPrint(MatA);

	printf("Compute pseudo-inverse matrix, 1/(AT × A) × AT\n");
	MatPI = PseudoInvert(MatA);
		printf("PI = Pseudo Invert(A)\n");
		MatPrint(MatPI);
	MatFree(MatA);

	printf("Solution Matrix to translate touch screen to screen positions\n");
	// MatR = PI * X
	MatR = MatMul(MatPI,MatX);
		printf("(R = PI * X\n");
		MatPrint(MatR);
	MatFree(MatR);
	MatFree(MatX);

	// MatR = 1/A * Y
	MatR = MatMul(MatPI,MatY);
		printf("R = PI * Y\n");
		MatPrint(MatR);
	MatFree(MatR);
	MatFree(MatY);

	MatFree(MatPI);

	printf("==========================================\n");
	printf("\n");
	
}
#endif

		
