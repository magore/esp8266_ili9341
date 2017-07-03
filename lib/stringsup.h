/**
 @file stringsup.h 

 @brief Various string and character functions

 @par Copyright &copy; 2016 Mike Gore, GPL License
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
#ifndef _STRINGSUP_H_
#define _STRINGSUP_H_

// Named address space
#ifndef MEMSPACE
#define MEMSPACE /**/
#endif

// Weak attribute
#ifndef WEAK_ATR
#define WEAK_ATR __attribute__((weak))
#endif

/// @brief undo any existing macros
#undef isdigit
#undef isupper
#undef islower
#undef tolower
#undef toupper


/* stringsup.c */
MEMSPACE void sep ( void );
MEMSPACE int WEAK_ATR isdigit ( int c );
MEMSPACE int WEAK_ATR isupper ( int c );
MEMSPACE int WEAK_ATR islower ( int c );
MEMSPACE int WEAK_ATR tolower ( int c );
MEMSPACE int WEAK_ATR toupper ( int c );
MEMSPACE void *memchr ( const void *str , int c , size_t size );
MEMSPACE size_t WEAK_ATR strlen ( const char *str );
MEMSPACE WEAK_ATR char *strcpy ( char *dest , const char *src );
MEMSPACE WEAK_ATR char *strncpy ( char *dest , const char *src , size_t size );
MEMSPACE WEAK_ATR char *strcpy ( char *dest , const char *src );
MEMSPACE WEAK_ATR char *strcat ( char *dest , const char *src );
MEMSPACE WEAK_ATR char *strncat ( char *dest , const char *src , size_t max );
MEMSPACE void WEAK_ATR reverse ( char *str );
MEMSPACE void WEAK_ATR strupper ( char *str );
MEMSPACE void trim_tail ( char *str );
MEMSPACE char *skipspaces ( char *ptr );
MEMSPACE char *nextspace ( char *ptr );
MEMSPACE char *skipchars ( char *str , char *pat );
MEMSPACE int WEAK_ATR strcmp ( const char *str , const char *pat );
MEMSPACE int WEAK_ATR strncmp ( const char *str , const char *pat , size_t len );
MEMSPACE int WEAK_ATR strcasecmp ( const char *str , const char *pat );
MEMSPACE int WEAK_ATR strncasecmp ( const char *str , const char *pat , size_t len );
MEMSPACE int MATCH ( char *str , char *pat );
MEMSPACE int MATCHARGS ( char *str , char *pat , int min , int argc );
MEMSPACE int MATCHI ( char *str , char *pat );
MEMSPACE int MATCH_LEN ( char *str , char *pat );
MEMSPACE int MATCHI_LEN ( char *str , char *pat );
MEMSPACE int split_args ( char *str , char *argv [], int max );
MEMSPACE char *get_token ( char *str , char *token , int max );
MEMSPACE int token ( char *str , char *pat );
MEMSPACE int32_t get_value ( char *str );
MEMSPACE char *strnalloc ( char *str , int len );
MEMSPACE char *stralloc ( char *str );



#endif
