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

#include "user_config.h"

/// =============================================================
/// =============================================================
/// HAL

#ifndef ADF4351_CS
#error You must define the ADF4351 GPIO pin
#endif

/// Start SPI Hardware Abstraction Layer
/// Keep all hardware dependent SPI code in this section

/// @brief cache of SPI clock devisor
uint32_t ADF4351_clock = -1;

/// @brief Obtain SPI bus for ADF4351 display, raises LE
/// return: void
MEMSPACE
void ADF4351_spi_init(void)
{
	spi_init(ADF4351_clock = 2, ADF4351_CS);
}


/// @brief  Obtain SPI bus for ADF4351 display, LE LOW
/// return: void
void ADF4351_spi_begin()
{
    spi_begin(ADF4351_clock, ADF4351_CS);
}


/// @brief  Release SPI bus from ADF4351, LE HIGH
/// return: void
void ADF4351_spi_end()
{
	spi_end(ADF4351_CS);
}

/// @brief  Transmit 32 bit data value
/// @param[in] value: 32bit data send
/// return: spi data
uint32_t ADF4351_spi_txrx(uint32_t value)
{
    int i;
    uint8_t tmp[4];
    uint32_t ret;

// we have to send MSB to LSB
    for(i=3;i>=0;--i) 
	{
        tmp[i] = value & 0xff;
		value >>=8;
	}

    ADF4351_spi_begin();
    spi_TXRX_buffer(tmp,4);   // send data and read any status from MUXOUT
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


