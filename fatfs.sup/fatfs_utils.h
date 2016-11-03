/**
 @file fatfs/fatfs_utils.h
 
 @brief fatfs test utilities with user interface

 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par Copyright &copy; 2013 ChaN.
 @par Credit: part of FatFs avr example project (C)ChaN, 2013.
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

#ifndef _FATFS_UTILS_H
#define _FATFS_UTILS_H


/* fatfs_utils.c */
MEMSPACE void fatfs_help ( void );
MEMSPACE void mmc_test ( void );
MEMSPACE void fatfs_ls ( char *ptr );
MEMSPACE void fatfs_rename ( const char *oldpath , const char *newpath );
MEMSPACE void fatfs_cat ( char *name );
MEMSPACE void fatfs_copy ( char *from , char *to );
MEMSPACE void fatfs_create ( char *name , char *str );
MEMSPACE void fatfs_rm ( char *name );
MEMSPACE void fatfs_mkdir ( char *name );
MEMSPACE void fatfs_rmdir ( char *name );
MEMSPACE void fatfs_stat ( char *name );
MEMSPACE void fatfs_cd ( char *name );
MEMSPACE void fatfs_pwd ( void );
MEMSPACE int fatfs_tests ( char *str );


#endif
