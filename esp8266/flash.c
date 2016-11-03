/**
 @file flash.c

 @brief Flash read and bit test utilities

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

#include "user_config.h"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "flash.h"

#define USE_CACHE
// Cached flash read on 32bit boundry
#ifdef USE_CACHE

/// @brief  Read bit bit value from flash
/// Supports 8 bits reads from memory spaced that must have 32bit aligned data
/// Caches the 32bit read
/// @param[in] *p: address to read
/// @return  uint8_t
// Cached address and data
uint32_t _addr = 0;
uint32_t _data;
uint8_t *p8 = (uint8_t *) &_data;
uint8_t read_flash8(uint8_t *p)
{
    uint8_t offset;
    uint32_t addr = (uint32_t)p;

    offset = 3 & addr;                            // byte offset
    addr &= ~3;                                   // align 4 byte address

    if(addr != _addr)
    {
        _addr = addr;
        _data = *(uint32_t *) _addr;
    }
    return(p8[offset]);
}


#else
/// @brief  Read bit bit value from flash
/// Supports 8 bits reads from memory spaced that must have 32bit aligned data
/// No cached version
/// @param[in] *p: address to read
/// @return  uint8_t
uint8_t read_flash8(uint8_t *p)
{
    uint8_t offset;
    uint32_t data;
    uint32_t addr = (uint32_t)p;
    uint8_t *p8 = (uint8_t *) &data;

    offset = 3 & addr;                            // byte offset
    addr &= ~3;                                   // align 4 byte address
    data = *(uint32_t *) addr;
    return(p8[offset]);
}
#endif

/// @brief  Copy data from Flash to Ram
/// Uses flash_read8() to avoid alighnment problems
/// @param[in] *src: address to read from
/// @param[out] *dest: address to write to
/// @param[in] size: number of bytes to copy
/// @return  void
void cpy_flash(uint8_t *src, uint8_t *dest, int size)
{
    int i;
    for(i=0;i<size;++i)
    {
        dest[i] = read_flash8((uint8_t *)src+i);
    }
}


/// @brief 16 bits reads from Flash memory space
/// Uses cpy_flash() to avoid alighnment problems
/// @param[in] *p: address to read
/// @return  uint16_t
uint16_t read_flash16(uint8_t *p)
{
    uint16_t tmp;
    cpy_flash(p,(uint8_t *)&tmp, 2);
    return(tmp);
}


/// @brief 32 bits reads from Flash memory space
/// Uses cpy_flash() to avoid alighnment problems
/// @param[in] *p: address to read
/// @return  uint32_t
uint32_t read_flash32(uint8_t *p)
{
    uint32_t tmp;
    cpy_flash(p,(uint8_t *) &tmp, 4);
    return(tmp);
}


/// @brief 64 bits reads from Flash memory space
/// Uses cpy_flash() to avoid alighnment problems
/// @param[in] *p: address to read
/// @return  uint64_t
uint64_t read_flash64(uint8_t *p)
{
    uint64_t tmp;
    cpy_flash(p,(uint8_t *) &tmp, 8);
    return(tmp);
}


/// @brief Pointer read from Flash memory space
/// Uses cpy_flash() to avoid alighnment problems
/// @param[in] *p: address to read
/// @return  uint32_t
uint32_t read_flash_ptr(uint8_t *p)
{
    uint32_t tmp;
    cpy_flash(p,(uint8_t *) &tmp, 4);
    return(tmp);
}


/// @brief Test bit in byte array
/// @param[in] *ptr: byte array
/// @param[in] off: bit offset to test
/// @return  1 if bit is set, 0 if not
int bittestv(unsigned char *ptr, int off)
{
    int data;
    data = read_flash8(ptr + (off>>3));
    return( (data & (0x80 >> (off&7))) );
}


/// @brief Test bit in w * h size bit array usng x and y offsets
/// @param[in] *ptr: byte array
/// @param[in] x: bit x offset
/// @param[in] y: bit y offset
/// @param[in] w: bit array wide
/// @param[in] h: bit array high
/// @return  1 if bit is set, 0 if not
int bittestxy(unsigned char *ptr, int x, int y, int w, int h)
{
    int off;

    if(y < 0 || y > h)
    {
        return 0;
    }
    if(x < 0 || x > w)
    {
        return 0;
    }
    off = y * w + x;
    return(bittestv(ptr, off));
}

