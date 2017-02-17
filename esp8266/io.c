/**
 @file gpio.c

 @brief GPIO driver for ESP8255
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
#include "user_config.h"

#include <eagle_soc.h>

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "hspi.h"
#include "gpio.h"

///@brief set chip enable
void chip_select(int addr)
{
	switch(addr)
	{
#ifdef ADF4351_CS
		case ADF4351_CS:
			GPIO_OUTPUT_SET(ADF4351_CS, 0);
			break;
#endif
#ifdef ILI9341_CS
		case ILI9341_CS:
			GPIO_OUTPUT_SET(ILI9341_CS, 0);
			break;
#endif
#ifdef MMC_CS
		case MMC_CS:
			GPIO_OUTPUT_SET(MMC_CS,0);
			break;
#endif
		// DISABLE all chip selects
		default:
#ifdef ADF4351_CS
			GPIO_OUTPUT_SET(ADF4351_CS, 1);
#endif
#ifdef ILI9341_CS
			GPIO_OUTPUT_SET(ILI9341_CS, 1);
#endif
#ifdef MMC_CS
			GPIO_OUTPUT_SET(MMC_CS,1);
#endif
			break;
	}
#ifndef HAVE_DECODER
#endif
}

///@brief disable all chip enable pins
void chip_disable()
{
	chip_select(0xff);
}


///@brief Initialize chip enables
void chip_select_init()
{
#ifdef ADF4351_CS
	gpio_io_mode(ADF4351_CS);
	GPIO_OUTPUT_SET(ADF4351_CS, 1);
#endif

#ifdef ILI9341_CS
	gpio_io_mode(ILI9341_CS);
	GPIO_OUTPUT_SET(ILI9341_CS, 1);
#endif

#ifdef MMC_CS
	gpio_io_mode(MMC_CS);
	GPIO_OUTPUT_SET(MMC_CS, 1);
#endif

#ifndef HAVE_DECODER
#endif

}

void chip_addr_init()
{
#ifdef ADDR_0
	gpio_io_mode(ADDR_0);
	GPIO_OUTPUT_SET(ADDR_0, 1);
#endif

#ifdef ADDR_1
	gpio_io_mode(ADDR_1);
	GPIO_OUTPUT_SET(ADDR_0, 1);
#endif
}

void chip_addr(int addr)
{
	switch(addr)
	{
#ifdef ADDR_0
		case 0:
			GPIO_OUTPUT_SET(ADDR_0, 0);
#ifdef ADDR_1
			GPIO_OUTPUT_SET(ADDR_1, 0);
#endif
			break;
		case 1:
			GPIO_OUTPUT_SET(ADDR_0, 1);
#ifdef ADDR_1
			GPIO_OUTPUT_SET(ADDR_1, 0):
#endif
			break;
#endif // ADDR_0

#ifdef ADDR_1
		case 2:
#ifdef ADDR_0
			GPIO_OUTPUT_SET(ADDR_0, 0);
#endif
			GPIO_OUTPUT_SET(ADDR_1, 1);
			break;
		case 3:
#ifdef ADDR_0
			GPIO_OUTPUT_SET(ADDR_0, 1);
#endif
			GPIO_OUTPUT_SET(ADDR_1, 1);
			break;
#endif // ADDR_1

		default:
			break;
	}
}

///@brief set GPIO pin to normal I/O mode
void gpio_io_mode(int pin)
{
	switch(pin)
	{
		case  0:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO0_U, FUNC_GPIO0);
			break;
		case  1:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0TXD_U, FUNC_GPIO1);
			break;
		case  2:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO2_U, FUNC_GPIO2);
			break;
		case  3:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_U0RXD_U, FUNC_GPIO3);
			break;
		case  4: // some esp8266-12 boards have incorrect labels 4 and 5 swapped
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
			break;
		case  5: // some esp8266-12 boards have incorrect labels 4 and 5 swapped
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
			break;
		case  6:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CLK_U, FUNC_GPIO6);
			break;
		case  7:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA0_U, FUNC_GPIO7);
			break;
		case  8:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA1_U, FUNC_GPIO8);
			break;
		case  9:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA2_U, FUNC_GPIO9);
			break;
		case 10:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_DATA3_U, FUNC_GPIO10);
			break;
		case 11:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_SD_CMD_U, FUNC_GPIO11);
			break;
		case 12:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDI_U, FUNC_GPIO12);
			break;
		case 13:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTCK_U, FUNC_GPIO13);
			break;
		case 14:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTMS_U, FUNC_GPIO14);
			break;
		case 15:
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_MTDO_U, FUNC_GPIO15);
			break;
	}
}


