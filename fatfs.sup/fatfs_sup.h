/**
 @file fatfs/fatfs_sup.h

@brief Allocate, Free and display FILINFO structurs, getfattime(), display error messages

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

#ifndef _FATFS_SUP_H_
#define _FATFS_SUP_H_

#if _MULTI_PARTITION != 0
extern const PARTITION Drives[] =
{
    {
        0,0
    }
    ,
    {
        0,1
    }
};
#endif

extern DWORD   AccSize;                           // Total file space used
extern WORD    AccFiles, AccDirs;                 // Total files and directories
extern FATFS   Fatfs[_VOLUMES];                   // File system object for each logical drive

/* fatfs_sup.c */
MEMSPACE uint32_t tm_to_fat ( tm_t *t );
MEMSPACE DWORD get_fattime ( void );
MEMSPACE void put_rc ( int rc );
MEMSPACE int fatfs_scan_files ( char *path );
MEMSPACE char *fatfs_fstype ( int type );
MEMSPACE void fatfs_status ( char *ptr );
MEMSPACE void fatfs_filinfo_list ( FILINFO *info );

#endif
