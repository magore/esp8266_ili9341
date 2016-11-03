
/**
 @file cordic.c

 @brief Cordic Routines
 Handle angle outside of the first quadrant
 Added standalone test to verify CORDIC against math library
 Add Documentation and references
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


// Run a standalone test
// #define TEST

#define CORDIC_TABLE  /* include the generated Cordic table */

#ifdef TEST
#define MEMSPACE   /* */
#include <stdio.h>
#include <stdlib.h>
#else
#include "user_config.h"
#include <stdint.h>
#include <string.h>
#include "printf/mathio.h"
#endif

#include <math.h>

#include "cordic2c_inc.h"
#include "cordic.h"

/// @brief  Convert Degrees to Rads
/// @param[in] deg: degrees
/// @return  radians
MEMSPACE
double deg2rad(double deg)
{
    return (deg * M_PI / 180.0);
}

/// @brief  Compute quadrant of angle and the quadrant modulus
/// Note:
/// Integer part is quadrant
/// @see cordic.h, we use fixed point numbers, where 1.0=Cordic_One
/// Note: 1.0 = 90 degrees
/// @param[in] quads: 1.0 = 90 degrees
/// @param[out] quad: quadrant 0 = 0 .. 89.9999, 1 = 90 ... 179.999 ..., etc
/// @return  fractional part of quads
MEMSPACE
double angle_quad(double quads, int *quad)
{

    int s,q;
    double intpart;

    s = 0;
    if(quads < 0.0)
    {
        quads = -quads;
        s = 4;                                    // sign information flag
    }

    quads = modf(quads, &intpart);
    q = intpart;
    q &= 3;
    q |= s;
    *quad = q;
    return(quads);

}


/// @brief  Main Cordic routine - used for basic trig and vector rotations
/// We use fixed point numbers, where 1.0=Cordic_One
/// @ref cordic.h
/// @see http://en.wikipedia.org/wiki/CORDIC
/// @param[in, out] x: in: Cordik_K, out: Cos of z
/// @param[in, out] y: in: 0, out: Sin of z
/// @param[in, out] z: in: fixed point version of angle in quads, out: not used
/// @return void
Cordic_T X,Y,Z;
void Circular (Cordic_T x, Cordic_T y, Cordic_T z)
{
    int i;

    X = x;
    Y = y;
    Z = z;

    for (i = 0; i < Cordic_T_Bits; ++i)
    {
        x = X >> i;
        y = Y >> i;

        if(i < 14)
            z = v_atangrad[i];
        else
            z >>= 1;

        if(Z >= 0)
        {
            X -= y;
            Y += x;
            Z -= z;
        }
        else
        {
            X += y;
            Y -= x;
            Z += z;
        }
    }
}


/// @brief  Compute Sin and Cos from angle in quads using Cordic
/// @see http://en.wikipedia.org/wiki/CORDIC
/// @param[in] angle: angle in quads ( 1 quad = 90 degrees)
/// @param[out] *s: sin
/// @param[out] *c: cos
/// @return void
MEMSPACE
void cordic_quad(double angle, double *s, double *c)
{
#ifdef TEST
    double ts,tc;
    double es,ec;
    double degree, rad, orig;
#endif

    double cs,cc;
    double quads, tmp;
    int quad;
    Cordic_T a;

    quads = angle_quad(angle,&quad);

    a = FP2Cordic(quads);                         /* convert to CORDIC fixed point */
    Circular(Cordic_K,0,a);
    cc = Cordic2FP(X);
    cs = Cordic2FP(Y);

// Angle 90 to < 180 degrees swap sin and cos, and negate cos
    if(quad & 1)
    {
        tmp = cc;
        cc = cs;
        cs = tmp;
        cc = -cc;
    }

// Angle 180 >.. 270 degrees negate both cos and sin
    if(quad & 2 )
    {
        cs = -cs;
        cc = -cc;
    }
// Angle < 0 degrees negate only sin
    if(quad & 4 )
    {
        cs = -cs;
    }

    *c = cc;
    *s = cs;

// Test difference between CORDIC and LIBC cos() and sin()
#ifdef TEST
    degree = angle * 90;
    rad = angle * M_PI / 2.0;
    tc = cos(rad);
    ts = sin(rad);

    ec = cc - tc;
    es = cs - ts;
    printf("Degree:%lf, rad:%lf, quad:%d\n",
        (double)angle, (double) rad, (int)quad);
    printf("ccos:%lf, cos:%lf, error:%lf\n",
        cc, tc, ec);
    printf("csin:%lf, sin:%lf, error:%lf\n",
        cs, ts, es);
#endif

}


/// @brief  Compute Sin and Cos from angle in degrees using Cordic
/// @see http://en.wikipedia.org/wiki/CORDIC
/// @param[in] deg: angle in degrees
/// @param[in,out] *s: sin
/// @param[in,out] *c: cos
/// @return void
MEMSPACE
void cordic_deg(double deg, double *s, double *c)
{
    cordic_quad(deg/90.0, s, c);
}


/// @brief  Compute Sin and Cos from angle in Rads using Cordic
/// @see http://en.wikipedia.org/wiki/CORDIC
/// @param[in] rad: angle in radians
/// @param[out] *s: sin
/// @param[out] *c: cos
/// @return void
MEMSPACE
void cordic_rad(double rad, double *s, double *c)
{
    cordic_quad(rad * 2.0 /M_PI, s, c);
}


/// @brief  Scale x,y,z by scale factor
/// @param[in] *P: x,y,z point
/// @param[in] scale: scale factor
/// @return void
MEMSPACE
void scale_point(point *P, double scale)
{
    P->x *= scale;
    P->y *= scale;
    P->z *= scale;
}


/// @brief  Shift x,y,z by shift
/// @param[in] *P: x,y,z point
/// @param[in] shift: shift to apply to x,y,z
/// @return void
MEMSPACE
void shift_point(point *P, point *shift)
{
    P->x += shift->x;
    P->y += shift->y;
    P->z += shift->x;
}


/// @brief  Rotate point P by View point
/// @param[in] *P: x,y,z point
/// @param[in] *V: View point
/// @return void
MEMSPACE
void rotate(point *P, point *V)
{
    double sinx, siny, sinz, cosx, cosy, cosz;
    double x,y,z,x1,y1,z1;

// Point
    x = P->x;
    y = P->y;
    z = P->z;

// Transform Point

// View is in degrees
    cordic_deg(V->x, &sinx, &cosx);
    cordic_deg(V->y, &siny, &cosy);
    cordic_deg(V->z, &sinz, &cosz);

    x1 = x*cosz + y*sinz;                         // Rotation around axis Z
    y1 = -x*sinz + y*cosz;
    z1 = z;

    x = x1;                                       // Rotation around axis X
    y = y1*cosx + z1*sinx;
    z = -y1*sinx + z1*cosx;

    x1 = x*cosy - z*siny;                         // Rotation around axis Y
    y1 = y;
    z1 = x*siny + z*cosy;

// Transformed point
    P->x = x1;
    P->y = y1;
    P->z = z1;
}


/*
 @brief // Perspective Projection with Offset and Scale
 @param[in] *P: x,y,z point, out: x,y ( z = 0 )
 @param [in] scale: scale factor
 @param [out] x: X offset
 @param [out] y: Y offset
 @return void
*/
MEMSPACE 
void PerspectiveProjection(point *P, double scale, int x, int y)
{
    P->x = (P->x + P->z / 2) * scale + x;
    P->y = (P->y - P->z / 2) * scale + y;
    P->z = 0;
}

#ifdef TEST
/// @brief  Stand alone test program to verify Cordic conversions
/// @return void
int main()
{
    double d;
    double s,c;
    int i;

    i = 0;

// Make sure CORIC converges correctly over a few rotations
    for(d=0;d<=360;d+=45)
    {
        cordic_deg(d-1,&s,&c);
        cordic_deg(d,&s,&c);
        cordic_deg(d+1,&s,&c);
    }
    return(0);
}
#endif

