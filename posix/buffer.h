#ifndef _BUFFER_H_
#define _BUFFER_H_

/**
 @file buffer.h

@brief character read buffering wrappers for FatFS 
       Character at a time operation is FatFS are VERY slow

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL  License
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for specific Copyright details

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


typedef struct 
{
    FILE *fp;
    int ind;
    int size;
    int len;
    int ungetf;
    int ungetc;
    uint8_t *buf;
} buffer_t;

/* buffer.c */
buffer_t *buffer_read_open ( char *name , uint8_t *buf , int size );
void buffer_read_close ( buffer_t *p );
void buffer_ungetc ( buffer_t *p , int c );
int buffer_getc ( buffer_t *p );
uint8_t *buffer_gets ( uint8_t *str, int size , buffer_t *p );


#endif // _BUFFER_H
