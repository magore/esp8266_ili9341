/**
 @file fatfs/fatfs_utils.h
 
 @brief fatfs test utilities with user interface

 @par Copyright &copy; 2014-2017 Mike Gore, All rights reserved. GPL  License
 @see http://github.com/magore/hp85disk
 @see http://github.com/magore/hp85disk/COPYRIGHT.md for specific Copyright details

 @par Credit: part of FatFs avr example project (C)ChaN, 2013.
 @par Copyright &copy; 2013 ChaN.

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

#ifndef _FATFS_TESTS_H
#define _FATFS_TESTS_H

/* fatfs_tests.c */
MEMSPACE void fatfs_help ( int full );
MEMSPACE int fatfs_tests ( int argc , char *argv []);
MEMSPACE void mmc_test ( void );
MEMSPACE void fatfs_cat ( char *name );
MEMSPACE void fatfs_copy ( char *from , char *to );
MEMSPACE void fatfs_create ( char *name , char *str );
MEMSPACE void fatfs_cd ( char *name );
MEMSPACE void fatfs_ls ( char *name );
MEMSPACE void fatfs_mkdir ( char *name );
MEMSPACE void fatfs_pwd ( void );
MEMSPACE void fatfs_rename ( const char *oldpath , const char *newpath );
MEMSPACE void fatfs_rm ( char *name );
MEMSPACE void fatfs_rmdir ( char *name );
MEMSPACE void fatfs_stat ( char *name );

#endif
