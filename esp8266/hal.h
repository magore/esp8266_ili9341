/**
 @file hal.c
 @brief HAL layer
  SPI, GPIO, address and chip select
 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
  Please retain a copy of this notice in any code you use it in.

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

#ifndef _INCLUDE_HAL_H_
#define _INCLUDE_HAL_H_


///@brief AVR GPIO MACROs
#ifdef AVR
/// @brief AVR GPIO pin definitions
#define GPIO_A0 0
#define GPIO_A1 1
#define GPIO_A2 2
#define GPIO_A3 3
#define GPIO_A4 4
#define GPIO_A5 5
#define GPIO_A6 6
#define GPIO_A7 7

#define GPIO_B0 8
#define GPIO_B1 9
#define GPIO_B2 10
#define GPIO_B3 11
#define GPIO_B4 12
#define GPIO_B5 13
#define GPIO_B6 14
#define GPIO_B7 15

#define GPIO_C0 16
#define GPIO_C1 17
#define GPIO_C2 18
#define GPIO_C3 19
#define GPIO_C4 20
#define GPIO_C5 21
#define GPIO_C6 22
#define GPIO_C7 23

#define GPIO_D0 24
#define GPIO_D1 25
#define GPIO_D2 26
#define GPIO_D3 27
#define GPIO_D4 28
#define GPIO_D5 29
#define GPIO_D6 30
#define GPIO_D7 31

#define GPIO_E0 32
#define GPIO_E1 33
#define GPIO_E2 34
#define GPIO_E3 35
#define GPIO_E4 36
#define GPIO_E5 37
#define GPIO_E6 38
#define GPIO_E7 39

#define GPIO_F0 40
#define GPIO_F1 41
#define GPIO_F2 42
#define GPIO_F3 43
#define GPIO_F4 44
#define GPIO_F5 45
#define GPIO_F6 46
#define GPIO_F7 47

#define GPIO_G0 48
#define GPIO_G1 49
#define GPIO_G2 50
#define GPIO_G3 51
#define GPIO_G4 52
#define GPIO_G5 53
#define GPIO_G6 54
#define GPIO_G7 55

/// @brief GPIO MACRO notes
///   We do not use {} around the macro statements so they behave like functions.
///   Consider what would happen if you used breaces (should be obviious)
///      if ( GPIB_IO_RD(SENSOR) )
///      {
///       printf"SENSOR");
///      }
///      else
///      {
///  do stuff
///      }
/// @brief program SFR to permit normal GPIO mode

/// @brief program SFR to permit normal gpio input/output
#define GPIO_MODE(pin)      gpio_sfr_mode(pin) /* FIXME TODO */

/// @brief program input mode
#define GPIO_DIR_IN(pin)    BIT_CLR(_SFR_IO8(((pin)>>3)*3+DDRA), ((pin) & 7)) 
#define GPIO_PIN_RD(pin)    BIT_TST(_SFR_IO8(((pin)>>3)*3+PINA), ((pin) & 7))
#define GPIO_RD(pin)        (GPIO_DIR_IN(pin), GPIO_PIN_RD(pin))

/// @brief program output latch
#define GPIO_LATCH_LOW(pin) BIT_CLR(_SFR_IO8(((pin)>>3)*3+PORTA), ((pin) & 7)) 
#define GPIO_LATCH_HI(pin)  BIT_SET(_SFR_IO8(((pin)>>3)*3+PORTA), ((pin) & 7)) 
#define GPIO_LATCH_RD(pin)  BIT_TST(_SFR_IO8(((pin)>>3)*3+PORTA), ((pin) & 7))

/// @brief program output mode
#define GPIO_DIR_OUT(pin)   BIT_SET(_SFR_IO8(((pin)>>3)*3+DDRA), ((pin) & 7))
#define GPIO_LOW(pin)       GPIO_LATCH_LOW(pin), GPIO_DIR_OUT(pin)
#define GPIO_HI(pin)        GPIO_LATCH_HI(pin),  GPIO_DIR_OUT(pin)
#define GPIO_OUT(pin,vpinl) (vpinl) ? GPIO_HI(pin) : GPIO_LOW(pin) 

//FIXME do pull up modes
#define GPIO_FLOAT(pin)     GPIO_DIR_IN(pin)

///  @brief Notes about AVR and PIC port BIT differences.
///  AVR                             PIC
///  DDR 1=out, 0 =in                TRIS 0=out,1=in
///  PORT=val same as LATCH=val      PORT=val same as LATCH=val
///  val=PORT, reads LATCH           val=PORT reads PIN state
///  val=PIN,  reads PIN state       val=LATCH reads latch



/*
 FIXME make sure these OLD AVR definitions are not being used
	AVR_DIR_IN(a)
	AVR_DIR_OUT(a)
	AVR_LATCH_LOW(a)
	AVR_LATCH_HI(a)
	AVR_LATCH_RD(a)
	AVR_PIN_RD(a)
	AVR_PULLUP(a)
	AVR_NO_PULLUP(a)
	IO_HI(a)
	IO_LOW(a)
	IO_RD(a)
	IO_FLOAT(a)
	IO_LATCH_RD(a)
	IO_JUST_RD(a)
	IO_JUST_LOW(a)
	IO_JUST_HI(a)
*/

#endif	// AVR

///@brief ESP8266 GPIO MACROs
#ifdef ESP8266

// not defined in eagle_soc.h
#define FUNC_GPIO6 3
#define FUNC_GPIO7 3
#define FUNC_GPIO8 3
#define FUNC_GPIO11 3

/// @brief program SFR to permit normal GPIO mode
#define GPIO_MODE(pin)       gpio_sfr_mode(pin);

/// @brief program input mode
#define GPIO_DIR_IN(pin)     GPIO_REG_WRITE(GPIO_ENABLE_W1TC_ADDRESS, 1<<(pin))
#define GPIO_PIN_RD(pin)     BIT_TST(GPIO_REG_READ(GPIO_IN_ADDRESS),(pin))
/// @brief program input mode and read
#define GPIO_RD(pin)         GPIO_DIR_IN(pin), GPIO_PIN_RD(pin)

/// @brief program output latch
#define GPIO_LATCH_LOW(pin)  GPIO_REG_WRITE(GPIO_OUT_W1TC_ADDRESS, 1<<(pin))
#define GPIO_LATCH_HI(pin)   GPIO_REG_WRITE(GPIO_OUT_W1TS_ADDRESS, 1<<(pin))
/// @brief read output latch
#define GPIO_LATCH_RD(pin)   BIT_TST(GPIO_REG_READ(GPIO_OUT_DATA),(pin))

/// @brief program output mode
#define GPIO_DIR_OUT(pin)    GPIO_REG_WRITE(GPIO_ENABLE_W1TS_ADDRESS, 1<<(pin))
#define GPIO_HI(pin)         GPIO_LATCH_HI(pin),  GPIO_DIR_OUT(pin)
#define GPIO_LOW(pin)        GPIO_LATCH_LOW(pin), GPIO_DIR_OUT(pin)
#define GPIO_OUT(pin,val)    (val) ? GPIO_HI(pin) : GPIO_LOW(pin)


// GPIO pin 16 is special - it uses the RTC GPIO pin
/// @brief program SFR to permit normal GPIO mode
#define GPIO16_MODE()      gpio_sfr_mode(16)

/// @brief program input mode
#define GPIO16_DIR_IN()    gpio16_dir(0)
#define GPIO16_PIN_RD()    ( READ_PERI_REG(RTC_GPIO_IN_DATA) & 1 )
/// @brief program input mode and read
#define GPIO16_RD()        ( GPIO16_DIR_IN(), GPIO16_PIN_RD() )

/// @brief program output latch
#define GPIO16_LATCH_LOW() \
	WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT)&(uint32_t)0xfffffffe) )
#define GPIO16_LATCH_HI()  \
	WRITE_PERI_REG(RTC_GPIO_OUT, (READ_PERI_REG(RTC_GPIO_OUT)&(uint32_t)0xfffffffe)|(uint32_t)1)
#define GPIO16_LATCH_RD()  ( READ_PERI_REG(RTC_GPIO_OUT) & 1 )

/// @brief program output mode
#define GPIO16_DIR_OUT()   gpio16_dir(1)
#define GPIO16_HI()        GPIO16_LATCH_HI(), GPIO16_DIR_OUT()
#define GPIO16_LOW()       GPIO16_LATCH_LOW(), GPIO16_DIR_OUT()
#define GPIO16_OUT(val)    (val) ? GPIO16_HI() : GPIO16_LOW()

//FIXME do pull up modes
#define GPIO16_FLOAT()     GPIO16_DIR_IN()

void MEMSPACE gpio_sfr_mode ( int pin );
void MEMSPACE gpio16_dir ( uint8_t out );

#endif	// ESP8266

/* hal.c */
void gpio_sfr_mode ( int pin );
void gpio16_dir ( uint8_t out );
void gpio_out ( uint8_t pin , uint8_t val );
uint8_t gpio_rd ( uint8_t pin );
void chip_select_init ( uint8_t pin );
void chip_select ( uint8_t pin );
void chip_deselect ( uint8_t pin );
void chip_addr_init ( void );
void chip_addr ( int addr );
void spi_waitReady ( void );
void spi_begin ( uint32_t clock , int pin );
void spi_end ( uint8_t pin );
uint8_t spi_chip_select_status ( void );
void spi_TX_buffer ( const uint8_t *data , int count );
void spi_RX_buffer ( const uint8_t *data , int count );
void spi_TXRX_buffer ( const uint8_t *data , int count );
uint8_t spi_RX ( void );
void spi_TX ( uint8_t data );
uint8_t spi_TXRX ( uint8_t data );

#endif    /* _INCLUDE_HAL_H_ */
