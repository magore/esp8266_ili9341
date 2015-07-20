/**
 @file util.c

 @brief Flash read and bit utilities

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
#include "util.h"

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


///@brief Skip white space in a string - tabs and spaces.
///
///@param[in] ptr: input string
///
///@return pointer to first non white space character 
MEMSPACE 
char *skipspaces(char *ptr)
{
    if(!ptr)
        return(ptr);

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;
    return(ptr);
}

///@brief Skip to first white space in a string - tabs and spaces.
///
///@param[in] ptr: input string
///
///@return pointer to first white space character 

MEMSPACE 
char *nextspace(char *ptr)
{
    if(!ptr)
        return(ptr);

    while(*ptr)
    {
        if(*ptr == ' ' || *ptr == '\t')
            break;
        ++ptr;
    }
    return(ptr);
}

///@brief Skip characters defined in user string.
///
///@param[in] str: string
///@param[in] pat: pattern string
///
///@return pointer to string after skipped characters.

MEMSPACE 
char *skipchars(char *str, char *pat)
{
    char *base;
    if(!str)
        return(str);

    while(*str)
    {
        base = pat;
        while(*base)
        {
            if(*base == *str)
                break;
            ++base;
        }
        if(*base != *str)
            return(str);
        ++str;
    }
    return(str);
}


///@brief Trim White space and control characters from end of string.
///
///@param[in] str: string
///
///@return void
///@warning Overwrites White space and control characters with EOS.
MEMSPACE 
void trim_tail(char *str)
{
    int len = strlen(str);
    while(len--)
    {
        if(str[len] > ' ')
            break;
        str[len] = 0;
    }
}



///@brief Allocate space for string with maximum size.
///
/// - Copies tring into allocated space limited to maximum size.
///
///@param[in] str: user string.
///@param[in] len: maximum string length.
///
///@return pointer to alocated string.

MEMSPACE 
char *strnalloc(char *str, int len)
{
    char *ptr;

    if(!str)
        return(NULL);
    ptr = calloc(len+1,1);
    if(!ptr)
        return(ptr);
    strncpy(ptr,str,len);
    return(ptr);

}


///@brief Allocate space for string.
///
/// - Copies tring into allocated space.
///
///@param[in] str: user string.
///
///@return pointer to alocated string.
///@return NULL on out of memory.
MEMSPACE 
char *stralloc(char *str)
{
    char *ptr;
    int len;

    if(!str)
        return(str);;
    len  = strlen(str);
    ptr = calloc(len+1,1);
    if(!ptr)
        return(ptr);
    strcpy(ptr,str);
    return(ptr);
}

///@brief return next token
///
/// - Skips all non printable ASCII characters before token
/// - Token returns only printable ASCII
///
///@param[in] str: string to search.
///@param[out] token: token to return
///@param[in] max: maximum token size
///
///@return pointer past token on success .
///@return NULL if no token found
MEMSPACE 
char *get_token(char *str, char *token, int max)
{
    int len;
    char *save;

    *token = 0;

    // NULL ?
    if(!str)
        return(NULL);

    str = skipspaces(str);
    save = str;

    // Find size of token
    len = 0;
    while(*str > ' ' && *str <= 0x7e && len < max)
    {
        // clip token to max length
        if(len < max)
        {
            *token++ = *str++;
            ++len;
        }
    }
    *token = 0;
    // str points past the token
    if(!len)
        return(NULL);
    return(str);
}


///@brief Search for token in a string matching user pattern.
///
/// - Skips all non printable ASCII characters before trying match.
///
///@param[in] str: string to search.
///@param[in] pat: pattern to search for.
///
///@return string lenth on match.
///@return 0 on no match.

MEMSPACE 
int token(char *str, char *pat)
{
    int patlen;
    int len;
    char *ptr;

    len = 0;
    ptr = str;
    while(*ptr > ' ' && *ptr <= 0x7e )
    {
        ++len;
        ++ptr;
    }

    if(!len)
        return(0);

    patlen = strlen(pat);

    if(len != patlen)
        return(0);

    if(strncmp(str,pat,patlen) == 0)
        return(len);
    return(0);
}


///@brief Convert Character into HEX digit.
///
/// @param[in] c: character.
///
/// @return HEX digit [0..F] on success.
/// @return 0xff on fail.
MEMSPACE 
uint8_t hexd( char c )
{
    if( c >= '0' && c <= '9' )
        return(c - '0');
    if( c >= 'a' && c <= 'f')
        return(c - 'a' + 10);
    if( c >= 'A' && c <= 'F')
        return(c - 'A' + 10);
    return(0xff);                                 // error
}
