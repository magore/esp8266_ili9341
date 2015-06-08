/**
 @file user_config.h

 @brief Master include file for project
  Includes all project includes and defines here

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




#ifndef __USER_CONFIG_H__
#define __USER_CONFIG_H__

#define MEMSPACE ICACHE_FLASH_ATTR
//#define MEMSPACE_RO ICACHE_RODATA_ATTR
#define MEMSPACE_RO static const

#include <ets_sys.h>
#include <osapi.h>
#include <os_type.h>
#include <c_types.h>
#include <ip_addr.h>
#include <espconn.h>
#include <gpio.h>
#include <user_interface.h>
#include <mem.h>
#include <util.h>
#include <uart.h>
#include <newlib.h>

#include "font.h"
#include "ili9341_adafruit.h"
#include "ili9341.h"
#include "cordic2c_inc.h"
#include "cordic.h"

#include "wire.h"
#include "hspi.h"
#include "util.h"

#ifdef NETWORK_TEST
#include "network.h"
#endif

int snprintf(char *, size_t, const char *, ...);

#ifdef MIKE_PRINTF
	#include "printf.h"
#endif

#define MEMSPACE_FONT ICACHE_FLASH_ATTR

#endif
