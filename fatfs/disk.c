/**
 @file fatfs/disk.c

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


#include <user_config.h>
#include "ff.h"
#include "diskio.h"
#include "posix.h"


///@brief FatFs Drive Volumes
FATFS Fatfs[_VOLUMES];                            /* File system object for each logical drive */

#if _MULTI_PARTITION != 0
/// @brief FatFs multiple partition drives
const PARTITION Drives[] =
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

///@brief FatFs Error Messages
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013
static char *err_msg[] =
{
    "OK",
    "DISK_ERR",
    "INT_ERR",
    "NOT_READY",
    "NO_FILE",
    "NO_PATH",
    "INVALID_NAME",
    "DENIED",
    "EXIST",
    "INVALID_OBJECT",
    "WRITE_PROTECTED",
    "INVALID_DRIVE",
    "NOT_ENABLED",
    "NO_FILE_SYSTEM",
    "MKFS_ABORTED",
    "TIMEOUT",
    "LOCKED",
    "NOT_ENOUGH_CORE",
    "TOO_MANY_OPEN_FILES",
    "INVALID_PARAMETER",
    NULL
};


/// @brief FAT time structer reference.
/// @see rtc.h
/// @see http://lxr.free-electrons.com/source/fs/fat/misc.c
/// @verbatim
/// typedef struct
/// {
///     WORD   year;                                  /* 2000..2099 */
///     BYTE   month;                                 /* 1..12 */
///     BYTE   mday;                                  /* 1.. 31 */
///     BYTE   wday;                                  /* 1..7 */
///     BYTE   hour;                                  /* 0..23 */
///     BYTE   min;                                   /* 0..59 */
///     BYTE   sec;                                   /* 0..59 */
/// } RTC;
/// @endverbatim


/// @brief Convert Linux POSIX tm_t * to FAT32 time.
///
/// @param[in] t: POSIX struct tm * to convert.
///
/// @return  FAT32 time.
MEMSPACE
uint32_t tm_to_fat(tm_t *t)
{
    uint32_t fat;
/* Pack date and time into a uint32_t variable */
    fat = ((uint32_t)(t->tm_year - 80) << 25)
        | (((uint32_t)t->tm_mon+1) << 21)
        | (((uint32_t)t->tm_mday) << 16)
        | ((uint32_t)t->tm_hour << 11)
        | ((uint32_t)t->tm_min << 5)
        | ((uint32_t)t->tm_sec >> 1);
    return(fat);
}
/// @brief Read DS1307 RTC and convert to FAT32 time.
///
/// @return FAT32 time.
/// @see tm_to_fat().
MEMSPACE
DWORD get_fattime (void)
{
	time_t t;
/* Get time */
    time(&t);
	return( tm_to_fat(localtime(&t)));
}

/// @brief  display FatFs return code as ascii string
///
/// Credit: Part of FatFs avr example project (C)ChaN, 2013
/// @param[in] rc: FatFs status return code
/// @return  void

MEMSPACE
void put_rc (int rc)
{
    char *ptr;
    if(rc > 19)
        ptr = "INVALID ERROR MESSAGE";
    else
        ptr = err_msg[(int)rc];

    printf("rc=%u FR_%s\n", rc, ptr);
}


///@brief Total file space used
DWORD   AccSize;

///@brief Total number or Files and Directories
WORD    AccFiles, AccDirs;

/// @brief  Use were FILINFO structure can be share in many functions
///  See: fatfs_alloc_finfo(), fatfs_scan_files() and fatfs_ls()
static FILINFO __finfo;

#if _USE_LFN
	static char __lfname[_MAX_LFN + 1];  /*< Common buffer to store LFN */
#endif

/// @brief  Allocate FILINFO structure and optional long file name buffer
///
/// @param[in] allocate: If allocate is true use calloc otherwise return static __finfo
/// @see fatfs_free_finfo() 
/// @see fatfs_scan_files()
/// @see fatfs_ls()
/// @return  FILINFO * on success
/// @return  NULL on error

MEMSPACE
FILINFO *fatfs_alloc_finfo( int allocate )
{
    FILINFO *finfo;

    if( allocate )
    {
        finfo = calloc(sizeof(FILINFO),1);
        if(finfo == NULL)
        {
            return(NULL);
        }
#if _USE_LFN
        finfo->lfname = calloc(_MAX_LFN + 1,1);
        finfo->lfsize = _MAX_LFN + 1;

        if(finfo->lfname == NULL)
        {
            free(finfo);
            return(NULL);
        }
#else
        finfo->lfname = NULL;
        finfo->lfsize = 0;
#endif
    }
    else
    {
        finfo = (FILINFO *) &__finfo;
#if _USE_LFN
        finfo->lfname = __lfname;
        finfo->lfsize = _MAX_LFN + 1;
#else
        finfo->lfname = NULL;
        finfo->lfsize = 0;
#endif
    }
    return( finfo );
}


/// @brief  Free a FILINFO structure and optional long file name buffer
/// allocated by fatfs_alloc_finfo()
///
/// @param[in] finfo: FatFs FILINFO pointer to free
/// @see fatfs_alloc_finfo()
/// @see fatfs_scan_files() 
/// @see fatfs_ls()
/// @return  void

MEMSPACE
void fatfs_free_filinfo( FILINFO *finfo )
{
#if _USE_LFN
    if(finfo->lfname && finfo->lfname != __lfname )
    {
        free(finfo->lfname);
    }
#endif
    if(finfo && finfo != (FILINFO *) &__finfo)
    {
        free(finfo);
    }
}


/// @brief  Compute space used, number of directories and files contained under a specified directory
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013
///
/// @param[in] path:
/// @see f_opendir()
/// @see f_readdir() 
/// @see AccDirs:  Total number of directories
/// @see AccFiles: Total number of Files
/// @see AccSize:  Total size of all files
/// @return 0 if no error
/// @return FafFs error code

MEMSPACE
int fatfs_scan_files (
char* path                                        /* Pointer to the working buffer with start path */
)
{
    FRESULT res;
    FILINFO *fno;
    DIR dirs;
    int i;
    char *fn;

    fno = fatfs_alloc_finfo(0);
    if(fno == NULL)
    {
        errno = ENOMEM;
        return(FR_NOT_ENOUGH_CORE);
    }

    res = f_opendir(&dirs, path);
    if (res == FR_OK)
    {
        i = strlen(path);
        while (((res = f_readdir(&dirs, fno)) == FR_OK) && fno->fname[0])
        {
            if (_FS_RPATH && fno->fname[0] == '.') continue;
#if _USE_LFN
            fn = *fno->lfname ? fno->lfname : fno->fname;
#else
            fn = fno->fname;
#endif
            if (fno->fattrib & AM_DIR)
            {
                AccDirs++;
                *(path+i) = '/'; strcpy(path+i+1, fn);
                res = fatfs_scan_files(path);
                *(path+i) = '\0';
                if (res != FR_OK) break;
            }
            else
            {
                AccFiles++;
                AccSize += fno->fsize;
            }
#ifdef ESP8266
			wdt_reset();
#endif
        }
    }

    return res;
}


/// @brief  Compute space used, number of directories and files contained used by a drive
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013
///
/// @param[in] ptr: Drive path like "/"
/// @see f_getfree()  drive free space
/// @see fatfs_scan_files()
/// @see AccDirs:  Total number of directories
/// @see AccFiles: Total number of Files
/// @see AccSize:  Total size of all files
/// @return  void

MEMSPACE
void fatfs_status(char *ptr)
{
    long p2;
    int res;
    FATFS *fs;

    const BYTE ft[] = {0,12,16,32};

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;
    printf("fatfs status:%s\n",ptr);
    res = f_getfree(ptr, (DWORD*)&p2, &fs);
    if (res)
    {
        put_rc(res);
        return;
    }
    printf("FAT type = FAT%u\nBytes/Cluster = %lu\nNumber of FATs = %u\n"
        "Root DIR entries = %u\nSectors/FAT = %lu\nNumber of clusters = %lu\n"
        "FAT start (lba) = %lu\nDIR start (lba,clustor) = %lu\nData start (lba) = %lu\n\n...",
        ft[fs->fs_type & 3], (DWORD)fs->csize * 512, fs->n_fats,
        fs->n_rootdir, fs->fsize, fs->n_fatent - 2,
        fs->fatbase, fs->dirbase, fs->database
        );
    AccSize = AccFiles = AccDirs = 0;
    res = fatfs_scan_files(ptr);
    if (res)
    {
        put_rc(res);
        return;
    }
    printf("\r%u files, %lu bytes.\n%u folders.\n"
        "%lu KB total disk space.\n%lu KB available.\n",
        AccFiles, AccSize, AccDirs,
        (fs->n_fatent - 2) * (fs->csize / 2), p2 * (fs->csize / 2)
        );
}


/// @brief  Display FILINFO structure in a readable format
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
/// - Example:
/// @verbatim
/// ----A 2014/10/16 00:39        14    test2.txt  
/// D---- 2014/10/12 21:29         0          tmp 
/// @endverbatim
///
/// @param[in] : FILINFO pointer
/// @return  void

MEMSPACE
void fatfs_filinfo_list(FILINFO *info)
{
	char attrs[6];
	int i;
    if(info->fname[0] == 0)
    {
        printf("fatfs_filinfo_list: empty\n");
        return;
    }
	attrs[0] = (info->fattrib & AM_DIR) ? 'D' : '-';
	attrs[1] = (info->fattrib & AM_RDO) ? 'R' : '-';
	attrs[2] = (info->fattrib & AM_HID) ? 'H' : '-';
	attrs[3] = (info->fattrib & AM_SYS) ? 'S' : '-';
	attrs[4] = (info->fattrib & AM_ARC) ? 'A' : '-';
	attrs[5] = 0;
	printf("%s %u/%02u/%02u %02u:%02u %9lu %12s",
        attrs,
        (info->fdate >> 9) + 1980, (info->fdate >> 5) & 15, info->fdate & 31,
        (info->ftime >> 11), (info->ftime >> 5) & 63,
        info->fsize, &(info->fname[0]));

#if _USE_LFN
	if(info->lfname)
		printf("  %s", info->lfname);
#endif

    printf("\n");
}
