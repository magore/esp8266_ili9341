/**
 @file hardware/bits.h

 @brief Bit manipulation macros.

 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 @par Copyright &copy; 2014 Mike Gore, Inc. All rights reserved.

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


///  Notes about AVR and PIC port BIT differences.
///  AVR                             PIC
///  DDR 1=out, 0 =in                TRIS 0=out,1=in
///  PORT=val same as LATCH=val      PORT=val same as LATCH=val
///  val=PORT, reads LATCH           val=PORT reads PIN state
///  val=PIN,  reads PIN state       val=LATCH reads latch

///  Notes:
///   "SENSOR" gets convereted into the required DDR and BIT definitions.
///   We use the comma operator to return the PIN test.
///   We do not use {} around the statements so they behave like functions.
///   Consider what would happen if you used breaces (should be obviious)
///      if ( GPIB_IO_RD(SENSOR) )
///      {
///       printf"SENSOR");
///      }
///      else
///      {
///  do stuff
///      }


#define AVR_DIR_IN(a)       BIT_CLR( a ## _DDR,  a ## _BIT)
#define AVR_DIR_OUT(a)      BIT_SET( a ## _DDR,  a ## _BIT)
#define AVR_LATCH_LOW(a)    BIT_CLR( a ## _PORT, a ## _BIT)
#define AVR_LATCH_HI(a)     BIT_SET( a ## _PORT, a ## _BIT)
#define AVR_LATCH_RD(a)     BIT_TST( a ## _PORT, a ## _BIT)
#define AVR_PIN_RD(a)       BIT_TST( a ## _PIN,  a ## _BIT)
#define AVR_PULLUP(a)       AVR_LATCH_HI(a)
#define AVR_NO_PULLUP(a)    AVR_LATCH_LOW(a)
#define IO_HI(a)            AVR_LATCH_HI(a), AVR_DIR_OUT(a)
#define IO_LOW(a)           AVR_LATCH_LOW(a), AVR_DIR_OUT(a)
#define IO_RD(a)            ( AVR_DIR_IN(a), AVR_PIN_RD(a))
#define IO_FLOAT(a)         AVR_DIR_IN(a)
#define IO_LATCH_RD(a)      ( AVR_LATCH_RD(a) )
#define IO_JUST_RD(a)       ( PIN_RD(a))
#define IO_JUST_LOW(a)      ( AVR_LATCH_LOW(a) )
#define IO_JUST_HI(a)       ( AVR_LATCH_HI(a) )
#endif


#endif // ifndef _BITS_H_
