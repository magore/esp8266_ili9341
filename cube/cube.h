/*
 * cube.h
 *
 *  Created on: 28 џэт. 2015 у.
 *      Author: Sem
 */

#ifndef INCLUDE_CUBE_H_
#define INCLUDE_CUBE_H_

#define AMOUNT_NODE 	8

extern void cube_calculate(double degreeX, double degreeY, double degreeZ, double scale, int16_t shiftX, int16_t shiftY, int16_t shiftZ);
extern void cube_draw(uint16_t color);

#endif /* INCLUDE_CUBE_H_ */
