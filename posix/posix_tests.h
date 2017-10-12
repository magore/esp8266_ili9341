#ifndef _POSIX_TESTS_H_
#define _POSIX_TESTS_H_
/**
 @file posix_tests.h

 @brief POSIX wrapper for FatFS

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
#endif

/* posix_tests.c */
MEMSPACE void posix_help ( int full );
MEMSPACE int posix_tests ( int argc , char *argv []);
MEMSPACE long cat ( char *name , int dopage );
MEMSPACE long copy ( char *from , char *to );
MEMSPACE int hexdump ( char *name , int dopage );
MEMSPACE int setpage ( int count );
MEMSPACE int testpage ( int count );
MEMSPACE int ls_info ( char *name , int verbose );
MEMSPACE int ls ( char *name , int verbose );
MEMSPACE long logfile ( char *name , char *str );
MEMSPACE uint16_t sum ( char *name );
MEMSPACE long upload ( char *name );

