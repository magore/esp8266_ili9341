/**
 @file server.c

 @brief Network test client
  This code receives a message and displays iit.

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


#ifndef _SERVER_H_
#define _SERVER_H_

#include <c_types.h>
#include <espconn.h>

MEMSPACE void servertest_message ( window *win );
MEMSPACE void servertest_receive ( void *arg , char *pdata , unsigned short len );
MEMSPACE void servertest_setup ( int port );

#endif
