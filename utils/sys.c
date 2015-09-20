/**
 @file sys.c

 @brief Memory and system utilities

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

#include <user_config.h>
#include "sys.h"


/// @brief Free buffer
/// POSIX function
/// We only call os_free() is pointer is not null
/// @param[in] *p: buffer to free
/// @return  void 
MEMSPACE 
void free(void *p)
{
    if(p)
        os_free((int) p);
}

/// @brief calloc buffer
/// POSIX function
/// @param[in] nmemb: number of elements
/// @param[in] size:  size of elements
/// @return  void * buffer
MEMSPACE 
void *calloc(size_t nmemb, size_t size)
{
    void *p = (void *)os_zalloc( (nmemb * size) );
    return(p);
}

/// @brief malloc buffer
/// POSIX function
/// @param[in] size:  size of buffer
/// @return  void * buffer
MEMSPACE 
void *malloc(size_t size)
{
    void *p = (void *) os_malloc( size );
    return(p);
}

/// @brief reset system
/// @return  void
MEMSPACE 
void reset(void)
{
    system_restart();
}


/// @brief reset watchdog
/// @return  void
MEMSPACE 
void wdt_reset( void )
{
  WRITE_PERI_REG(0x60000914, 0x73);
}

