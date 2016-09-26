/**
 @file     ADF4351_hal.h
 @version  V0.10
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

#ifndef _ADF4351_HAL_H_
#define _ADF4351_HAL_H_

#include <user_config.h>

// GPIO pin 0
#define ADF4351_LE_PIN   0
#define ADF4351_LE_LOW   GPIO_OUTPUT_SET(0, 0)
#define ADF4351_LE_HI    GPIO_OUTPUT_SET(0, 1)
#define ADF4351_LE_INIT  PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0); ADF4351_LE_HI

#endif

/* adf4351.c */
MEMSPACE void ADF4351_gpio_init ( void );
MEMSPACE void ADF4351_spi_init ( );
void ADF4351_spi_begin ( void );
void ADF4351_spi_end ( void );
uint32_t ADF4351_txrx ( uint32_t value );
