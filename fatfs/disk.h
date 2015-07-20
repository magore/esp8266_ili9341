/**
 @file fatfs/disk.h

@brief Allocate, Free and display FILINFO structurs, getfattime(), display error messages

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

#ifndef _DISK_H_
#define _DISK_H_

#include <user_config.h>

#include "ff.h"
#include "diskio.h"

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

/* disk.c */
MEMSPACE uint32_t tm_to_fat ( tm_t *t );
MEMSPACE DWORD get_fattime ( void );
MEMSPACE void put_rc ( int rc );
MEMSPACE FILINFO *fatfs_alloc_finfo ( int allocate );
MEMSPACE void fatfs_free_filinfo ( FILINFO *finfo );
MEMSPACE int fatfs_scan_files ( char *path );
MEMSPACE void fatfs_status ( char *ptr );
MEMSPACE void fatfs_filinfo_list ( FILINFO *info );

#endif
