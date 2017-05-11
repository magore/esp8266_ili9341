/**
 @file hardware/bits.h

 @brief Bit manipulation macros.

 @par Edit History
 - [1.1]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014-2017 Mike Gore, Inc. All rights reserved.

*/

#ifndef _BITS_H_
#define _BITS_H_

///  Note: IF x and y are constants the compiler will fully reduce the expression
#define BIT_SET(x,y)    (x |=  (1 << (y)))
#define BIT_CLR(x,y)    (x &= ~(1 << (y)))
#define BIT_TST(x,y)    ((x  &  (1 << (y))) ? (int) 1 : (int) 0)

#ifdef AVR
#define BIT0    0
#define BIT1    1
#define BIT2    2
#define BIT3    3
#define BIT4    4
#define BIT5    5
#define BIT6    6
#define BIT7    7

#define BIT8    8
#define BIT9    9
#define BIT10   10
#define BIT11   11
#define BIT12   12
#define BIT13   13
#define BIT14   14
#define BIT15   15
#endif	// AVR


#endif // ifndef _BITS_H_
