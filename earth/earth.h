
#ifndef __EARTH_H__
#define __EARTH_H__
#include "earth_inc.h"


void map2fp ( mapxyz *in , point *out );
void Point2Display ( point *p , int *x , int *y );
void earth_draw ( point *view , double scale , uint16_t color );

#endif
