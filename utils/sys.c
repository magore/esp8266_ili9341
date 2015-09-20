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

/// @brief calloc may be aliased to safecalloc
#undef calloc
/// @brief free may be aliased to safefree
#undef free
/// @brief malloc may be aliased to safecalloc
#undef malloc

/// @brief malloc buffer
/// POSIX function
/// @param[in] size:  size of buffer
/// @return  void * buffer
MEMSPACE 
void *malloc(size_t size)
{
	void *p = (void *)os_zalloc( size );
    return(p);
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

/// @brief Return Free memory 
///
/// @return free memory in bytes.
/// @see malloc().
MEMSPACE 
uint32_t freeRam()
{
	 return system_get_free_heap_size();
}


/// @brief Display Free memory and regions
/// @return void
MEMSPACE
void PrintRam()
{
    printf("Heap Free(%d) bytes\n" , system_get_free_heap_size());
    printf("Heap Start(%08x), Heap End(%08x), Delta(%d)\n" , 
		HEAP_START, HEAP_END, HEAP_END-HEAP_START);
}

/// @brief Safe Calloc -  Display Error message if Calloc fails
///
///  - We check if the pointer was in the heap.
///  - Otherwise it may have been statically defined - display error.
/// @param[in] nmemb: number of elements
/// @param[in] size:  size of elements
/// @return  void.
MEMSPACE 
void *safecalloc(size_t nmemb, size_t size)
{
    void *p = calloc(nmemb, size);
    if(!p)
    {
        printf("safecalloc(%d,%d) failed!\n", nmemb, size);
    }
    return(p);
}

/// @brief Safe Malloc -  Display Error message if Malloc fails
///
///  - We check if the pointer was in the heap.
///  - Otherwise it may have been statically defined - display error.
/// @param[in] size:  size 
/// @return  void.
MEMSPACE 
void *safemalloc(size_t size)
{
    void *p = calloc(size, 1);
    if(!p)
    {
        printf("safemalloc(%d) failed!\n", size);
    }
    return(p);
}

/// @brief Safe free -  Only free a pointer if it is in malloc memory range.
///  We want to try to catch frees of static or bogus data
///
/// FIXME HEAP_END is not likely exact 
///  If it is not exact it is larger or equal so the test is usefull
// @see https://github.com/esp8266/esp8266-wiki/wiki/Memory-Map
///
///  - We check if the pointer was in the heap.
///  - Otherwise it may have been statically defined - display error.
/// @param[in] p: pointer to free.
/// @return  void.
MEMSPACE 
void safefree(void *p)
{
	if( (uint32_t) p >= HEAP_START \
		&& (uint32_t) p <= HEAP_END)
	{
		free(p);
		return;
	}
    printf("safefree: FREE ERROR (%08x)\n", p);
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

