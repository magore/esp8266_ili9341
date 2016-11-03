/**
 @file fatfs/mmc.c

 @brief MMC upper level interface to FatFS
   - Originally part of FatFs avr example project (C)ChaN, 2013.
   - Specifically: avr_complex/main.c from ffsample.zip.
   - Modifications by Mike Gore.
   - I added hardware abstraction layer or mmc.c to make porting easier.

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

#ifndef _MMC_H_
#define _MMC_H_

/* mmc.c */
MEMSPACE int wait_ready ( UINT wt );
MEMSPACE DSTATUS mmc_disk_initialize ( void );
MEMSPACE DSTATUS mmc_disk_status ( void );
MEMSPACE DRESULT mmc_disk_read ( BYTE *buff , DWORD sector , UINT count );
MEMSPACE DRESULT mmc_disk_write ( const BYTE *buff , DWORD sector , UINT count );
MEMSPACE DRESULT mmc_disk_ioctl ( BYTE cmd , void *buff );
void mmc_disk_timerproc ( void );

#endif                                            // _MMC_H_
