/**
 @file io.c

 @brief IO driver for ESP8255
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

#ifndef INCLUDE_IO_H_
#define INCLUDE_IO_H_

// not defined in eagle_soc.h
#define FUNC_GPIO6 3
#define FUNC_GPIO7 3
#define FUNC_GPIO8 3
#define FUNC_GPIO11 3

/* io.c */
void chip_select ( int addr );
void chip_disable ( void );
void chip_select_init ( void );
void chip_addr_init ( void );
void chip_addr ( int addr );
void gpio_io_mode ( int pin );

#endif                                            /* INCLUDE_IO_H_ */
