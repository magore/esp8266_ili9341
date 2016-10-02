/**
 @file wire.h

 @brief wireframe view code

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

#ifndef _WIRE_H_
#define _WIRE_H_

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


#ifdef USER_CONFIG
#include "user_config.h"
#endif

#include "wire_types.h"

// Named address space
#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

// Weak attribute
#ifndef WEAK_ATR
#define WEAK_ATR __attribute__((weak))
#endif


/* wire.c */
void wire2fp ( wire_p *in , point *out );
void wire_draw ( window *win , const wire_p *wire , const wire_e *edge , point *view , int x , int y , double scale , uint16_t color );

#endif
