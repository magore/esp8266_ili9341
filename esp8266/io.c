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
#ifdef SWAP45
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
#else
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
#endif
			break;
		case  5: // some esp8266-12 boards have incorrect labels 4 and 5 swapped
#ifdef SWAP45
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO4_U, FUNC_GPIO4);
#else
			PIN_FUNC_SELECT(PERIPHS_IO_MUX_GPIO5_U, FUNC_GPIO5);
#endif
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


