/**
 @file cordic.c
 @brief Cordic Code Modified by Mike Gore 2015 to generate C Cordic tables 
 The code has been adjusted to quads (Where 90 degrees = 1.0) 
 The angle is just the fractional part of a floating point number 
 The interger part is the quadrant. 
 This makes computations for code using the tables much faster.


 Orginal Documentation

 CORDIC algorithms.
 The original code was published in Docter Dobbs Journal issue ddj9010.
 The ddj version can be obtained using FTP from SIMTEL and other places.
 *
 Converted to ANSI-C (with prototypes) by P. Knoppers, 13-Apr-1992.
 *
 The main advantage of the CORDIC algorithms is that all commonly used math
 functions ([a]sin[h] [a]cos[h] [a]tan[h] atah[h](y/x) ln exp sqrt) are
 implemented using only shift, add, subtract and compare. All values are
 treated as integers. Actually they are fixed point doubleing point values.
 The position of the fixed point is a compile time constant (Cordic_T_FractionBits).
 I don't believe that this can be easily fixed...
 
 Some initialization of internal tables and constants is necessary before
 all functions can be used. The constant "Cordic_HalfPI" must be determined before
 compile time, all others are computed during run-time - see main() below.
 
 Of course, any serious implementation of these functions should probably
 have _all_ constants determined sometime before run-time and most
 functions might be written in assembler.
 
 The layout of the code is adapted to my personal preferences.
 PK.
 */

/**
 * _IMPLEMENTING CORDIC ALGORITHMS_
 * by Pitts Jarvis
 *
 */

/**
 * @brief cordicC.c -- J. Pitts Jarvis, III
 * cordicC.c computes CORDIC constants and exercises the basic algorithms.
 * Represents all numbers in fixed point notation.  1 bit sign,
 * Cordic_T_Bits-1-n bit integral part, and n bit fractional part.  n=29 lets us
 * represent numbers in the interval [-4, 4) in 32 bit Cordic_T.  Two's
 * complement arithmetic is operative here.
 */

#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <unistd.h>
#include <libgen.h>

/**
 * @brief cordic algorithm identities for circular functions
 * starting with [x, y, z] and then
 * driving z to 0 gives: [P*(x*cos(z)-y*sin(z)), P*(y*cos(z)+x*sin(z)), 0]
 * driving y to 0 gives: [P*sqrt(x^2+y^2), 0, z+atan(y/x)]
 * where K = 1/P = sqrt(1+1)* . . . *sqrt(1+(2^(-2*i)))
 * special cases which compute interesting functions
 *  sin, cos      [K, 0, a] -> [cos(a), sin(a), 0]
 *  atan          [1, a, 0] -> [sqrt(1+a^2)/K, 0, atan(a)]
 *                [x, y, 0] -> [sqrt(x^2+y^2)/K, 0, atan(y/x)]
 * for hyperbolic functions, starting with [x, y, z] and then
 * driving z to 0 gives: [P*(x*cosh(z)+y*sinh(z)), P*(y*cosh(z)+x*sinh(z)), 0]
 * driving y to 0 gives: [P*sqrt(x^2-y^2), 0, z+atanh(y/x)]
 * where K = 1/P = sqrt(1-(1/2)^2)* . . . *sqrt(1-(2^(-2*i)))
 *  sinh, cosh    [K, 0, a] -> [cosh(a), sinh(a), 0]
 *  exponential   [K, K, a] -> [e^a, e^a, 0]
 *  atanh         [1, a, 0] -> [sqrt(1-a^2)/K, 0, atanh(a)]
 *                [x, y, 0] -> [sqrt(x^2-y^2)/K, 0, atanh(y/x)]
 *  ln            [a+1, a-1, 0] -> [2*sqrt(a)/K, 0, ln(a)/2]
 *  sqrt          [a+(K/2)^2, a-(K/2)^2, 0] -> [sqrt(a), 0, ln(a*(2/K)^2)/2]
 *  sqrt, ln      [a+(K/2)^2, a-(K/2)^2, -ln(K/2)] -> [sqrt(a), 0, ln(a)/2]
 * for linear functions, starting with [x, y, z] and then
 *  driving z to 0 gives: [x, y+x*z, 0]
 *  driving y to 0 gives: [x, 0, z+y/x]
 */

/** 
 * compute atan(x) and atanh(x) using infinite series
 *  atan(x) =  x - x^3/3 + x^5/5 - x^7/7 + . . . for x^2 < 1
 *  atanh(x) = x + x^3/3 + x^5/5 + x^7/7 + . . . for x^2 < 1
 * To calcuate these functions to 32 bits of precision, pick
 * terms[i] s.t. ((2^-i)^(terms[i]))/(terms[i]) < 2^-32
 * For x <= 2^(-11), atan(x) = atanh(x) = x with 32 bits of accuracy
 */

// =================================================================
typedef int Cordic_T;
typedef unsigned int UCordic_T;
typedef double FCordic_T;

// Number of bits in Cordic_T
#define Cordic_T_Bits        (int)(sizeof(Cordic_T) << 3)
#define FCordic_T_Bits       (int)(sizeof(FCordic_T) << 3)
// 3 integer bits 1 sign bin
#define Cordic_T_FractionBits    (int)(Cordic_T_Bits - 3 - 1)

#define Cordic_T_STR "typedef int Cordic_T;"
#define UCordic_T_STR "typedef unsigned int UCordic_T;"
#define FCordic_T_STR "typedef double FCordic_T;"
// =================================================================

static Cordic_T v_atan[Cordic_T_Bits+1];
static Cordic_T X;
static Cordic_T Y;
static Cordic_T Z;

#define Cordic_One     	(1UL << Cordic_T_FractionBits)
#define Cordic_K   		(Cordic_One * 0.6072529350088812561694)
#define Cordic_INVK   	(Cordic_One / 0.6072529350088812561694)
#define Cordic_KP		(Cordic_One * 1.20749706776307212887772)
#define Cordic_INVKP	(Cordic_One * 1/1.20749706776307212887772)
#define Cordic_HalfPI	((UCordic_T) (Cordic_One * M_PI_2) )
#define Cordic2FP(a)    ( (double) (a) / (double) (Cordic_One)) 
#define FP2Cordic(a)    ((Cordic_T) (Cordic_One * (a)))

/**
  @brief Create Cordic tables 
  Normalize base number system to 1.0 == Cordic_One == PI/2 (90 degrees)
  Example .5 is 50 gradians or PI/4
  This value as great adavantages:
  - interger part is small on the unit circle
  - integer part is the circle quadrant number (think range reductions)
*/

/// @brief  Dump Cordic C structure
/// @param [in] str: program name
/// @return  void
void dump_tables(char *str)
{
	int i;
	Cordic_T xx;
	char cwd[512];

	getcwd(cwd, 510);

	printf("#ifndef _CORDIC_INC_H\n");
	printf("#define _CORDIC_INC_H\n");
 	printf("//Mike Gore 2015, Cordic C Table\n");
 	printf("//Generated by program:[%s/%s]\n", cwd,str);
// Dump types
	printf("%s /* %u */\n", Cordic_T_STR, Cordic_T_Bits);
	printf("%s /* %u */\n", UCordic_T_STR, Cordic_T_Bits);
	printf("%s /* %u */\n", FCordic_T_STR, FCordic_T_Bits);
	printf("#define Cordic_T_Bits     %d\n", (int) Cordic_T_Bits);
    printf("#define Cordic_T_FractionBits  %d\n", (int) Cordic_T_FractionBits);


// Dump defines
	printf("#define Cordic_One    0x%lx /* %.15le */\n", 
		(long) Cordic_One, Cordic2FP(Cordic_One));
	printf("#define Cordic_K      0x%lx /* %.15le */\n", 
		(long) Cordic_K ,Cordic2FP(Cordic_K));
	printf("#define Cordic_INVK   0x%lx /* %.15le */\n", 
		(long) Cordic_INVK ,Cordic2FP(Cordic_INVK));
	printf("#define Cordic_KP     0x%lx /* %.15le */\n", 
		(long) Cordic_KP ,Cordic2FP(Cordic_KP));
	printf("#define Cordic_INVKP  0x%lx /* %.15le */\n", 
		(long) Cordic_INVKP ,Cordic2FP(Cordic_INVKP));
	printf("#define Cordic_HalfPI 0x%lx /* %.15le */\n", 
		(long) Cordic_HalfPI ,Cordic2FP(Cordic_HalfPI));
	printf("#define Cordic2FP(a)  ( (double) (a) / (double) (Cordic_One)) \n");
	printf("#define FP2Cordic(a)  ((Cordic_T) (Cordic_One * (a)))\n");

	printf("#ifdef CORDIC_TABLE\n");
// Dump normalize atan table
	xx = Cordic_One;
	for(i=0;i<Cordic_T_Bits;++i) {
			v_atan[i] = Cordic_One * atan( Cordic2FP(xx)) * (2.0/M_PI);
			xx >>= 1;
	}
	printf("static const Cordic_T v_atangrad[] = {\n");
	for(i=0;i<=Cordic_T_FractionBits;++i) {
		printf("\t0x%lx, /* %.8le */\n", 
		(long) v_atan[i], Cordic2FP(v_atan[i]));
	}
	printf("\t0\n};\n");

	printf("#else // CORDIC_TABLE\n");
	printf("extern const Cordic_T v_atangrad[];\n");
	printf("#endif // CORDIC_TABLE\n");
	printf("#endif // _CORDIC_INC_H\n");
}


/// @brief  Display X,Y,Z as floating point
/// @param[in] str: string header
/// @return  void
void PrintXYZ (char *str)
{
	printf("/* %s\n",str);
	printf("X: %.8lf\n", Cordic2FP(X));
	printf("Y: %.8lf\n", Cordic2FP(Y));
	printf("Z: %.8lf\n", Cordic2FP(Z));
	printf("*/\n");
}


/// @brief  Main Cordic rountine - used for basic trig and vector rotations
/// @see http://en.wikipedia.org/wiki/CORDIC
/// @param[in,out] x: Cordik_K, out: Cos of z
/// @param[in,out] y: 0, out: Sin of z
/// @param[in,out] z: fixed point version of angle 
/// @return void
void Circular (Cordic_T x, Cordic_T y, Cordic_T z)
{
    int i;

    X = x;
    Y = y;
    Z = z;

    for (i = 0; i <= Cordic_T_FractionBits; ++i)
    {
        x = X >> i;
        y = Y >> i;

		if(i < 14)
			z = v_atan[i];
		else
			z >>= 1;

		if(Z >= 0) {
			X -= y;
			Y += x;
			Z -= z;
		}
		else {
			X += y;
			Y -= x;
			Z += z;
		}
    }
}

/**
  @brief  This is the circular method. One slight change from the 
  other methods is the y < vecmode test. this is to implement arcsin, 
  otherwise it can be y < 0 and you can compute arcsin from arctan using
  trig identities, so its not essential.  
  @see http://en.wikipedia.org/wiki/CORDIC
  @param[in,out] x: in: Cordik_K, out: Cos of z
  @param[in,out] y: in: 0
  @param[in,out] z: in: 0
  @param[in] vecmode: arcsize value
  @return void
*/
void cordit1(Cordic_T x, Cordic_T y, Cordic_T z, Cordic_T vecmode)
{

    int i;

    X = x;
    Y = y;
    Z = z;

    for (i = 0; i < Cordic_T_FractionBits; ++i) {
        x = X >> i;
        y = Y >> i;
		if(i < 14)
			z = v_atan[i];
		else
			z >>= 1;
        if (vecmode >= 0 && Y < vecmode || vecmode < 0  && Z >= 0) {
            X -= y;
            Y += x;
            Z -= z;
        }
        else {
            X += y;
            Y -= x;
            Z += z;
        }
    }
}

/// @brief  Comutute ArcSine (a)
///  Only works for |a| < 0.98 
/// @param[in] a: Sine
/// @return  ArcSine (a)
Cordic_T asinCordic(Cordic_T a)
{
    Cordic_T x, y, z;
    

    x = Cordic_K;
    y = 0;
    z = 0;

    int neg = 1;
    if (a < 0) {
        a = -a;
        neg = 0;
    }
        

    cordit1(x, y, z, a);

    if (neg) Z = -Z;
    return Z;
}


/// @brief  Create C Cordic Tables andd test the results
/// @return  0
int main (int argc, char *argv[])
{
	Cordic_T x1;
	char str[256];
	double d;
	int a;

	dump_tables( basename( (char *) argv[0] ) );

	printf("// Test results\n");
	for(d=0;d<=1;d+=.1)
	{
		x1 = FP2Cordic(d);
		Circular (Cordic_K, 0L, x1);
		a = (int) 100 * d + 0.000005;	// rounding - 0.1 is not exact
		sprintf(str,"%d Gradians", a);
		PrintXYZ (str);
	}
	printf("// End of Test results\n");

    return (0);
}
