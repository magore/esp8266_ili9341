/**
 @file     ADF4351_hal.c
 @version V0.10
 @date     22 Sept 2016
 
 @brief ADF4351_HAL driver

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

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "adf4351_hal.h"

/// =============================================================
/// =============================================================
/// HAL

/// Start SPI Hardware Abstraction Layer
/// Keep all hardware dependent SPI code in this section

/// @brief cache of SPI clock devisor
uint16_t ADF4351_clock = -1;

/// @brief Obtain SPI bus for ADF4351 display, raises LE
/// return: void
MEMSPACE
void ADF4351_spi_init(void)
{
#ifdef ESP8266
    ADF4351_LE_INIT;
    ADF4351_LE_LOW;
    hspi_init( (ADF4351_clock = 2) , 0);
    hspi_waitReady();
#else
    ADF4351_LE_LOW;
    SPI0_Init(ADF4351_clock = 2);   //< Initialize the SPI bus
    SPI0_Mode(0);       //< Set the clocking mode, etc
#endif
}


/// @brief  Obtain SPI bus for ADF4351 display, LE LOW
/// return: void
void ADF4351_spi_begin()
{
#ifdef ESP8266
    hspi_waitReady();
    hspi_init(ADF4351_clock, 0);
#else
    SPI0_Init(ADF4351_clock = 2);   //< Initialize the SPI bus
    SPI0_Mode(0);       			//< Set the clocking mode, etc
#endif
    ADF4351_LE_LOW;
}


/// @brief  Release SPI bus from ADF4351, LE HIGH
/// return: void
void ADF4351_spi_end()
{
#ifdef ESP8266
    hspi_waitReady();
	//hspi_cs_disable(ADF4351_LE_PIN);
    ADF4351_LE_HI;
    hspi_waitReady();	//just a short delay, nops would work
    ADF4351_LE_LOW;
#else
    ADF4351_LE_HI;
    ADF4351_LE_LOW;
#endif
}

/// @brief  Transmit 32 bit data value
/// @param[in] value: 32bit data send
/// return: spi data
uint32_t ADF4351_spi_txrx(uint32_t value)
{
    int i;
    uint8_t tmp[4];
    uint32_t ret;

// LITTLE_ENDIAN and BIG_ENDIAN are both defined and of no value
// extensa has LSB to MSB byte order LITTLE_ENDIAN
// FIXME use functions
// we have to send MSB to LSB
    for(i=3;i>=0;--i) 
	{
        tmp[i] = value & 0xff;
		value >>=8;
	}

    ADF4351_spi_begin();

#ifdef ESP8266
    hspi_TXRX(tmp,4);   // send data and read any status from MUXOUT
#else
    SPI0_TXRX(tmp,4);
#endif

    ADF4351_spi_end();  // data

// MUXOUT output is tied to SPI RX
// which is controlled by register 2 DB28:DB26
// so we only get valid data if register 2 has been configured correctly
// MSB ... LSB order
    ret = 0;
    for(i=3;i>=0;--i)
    {
        ret <<= 8;
        ret |= tmp[i];
    }
    return(ret);
}


