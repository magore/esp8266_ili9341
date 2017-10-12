/**
 @file fatfs/fatfs_utils.c

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

#include "user_config.h"
#include "fatfs.h"

#ifdef AVR
//#include <stdio.h>
#include <stdlib.h>
#endif

#include "lib/time.h"
#include "lib/stringsup.h"

#include "printf/mathio.h"

/// @brief Display FatFs test diagnostics help menu.
/// @return  void
MEMSPACE
void fatfs_help( int full)
{
    printf("fatfs help\n");
    
    if(full)
    {
        printf(
#ifdef POSIX_TESTS
        "Note: fatfs tests MUST start with \"fatfs\" keyword\n"
#else
        "Note: fatfs prefix is optional\n"
#endif
        "fatfs help\n"
#ifdef FATFS_UTILS_FULL
        "fatfs attrib file p1 p2\n"
        "fatfs cat file\n"
        "fatfs cd dir\n"
        "fatfs copy file1 file2\n"
        "fatfs create file str\n"
#endif
        "fatfs mmc_test\n"
        "fatfs mmc_init\n"
        "fatfs ls dir\n"

#ifdef FATFS_UTILS_FULL
        "fatfs mkdir dir\n"
        "fatfs mkfs\n"
        "fatfs pwd\n"
#endif
        "fatfs status file\n"

#ifdef FATFS_UTILS_FULL
        "fatfs stat file\n"
        "fatfs rm file\n"
        "fatfs rmdir dir\n"
        "fatfs rename old new\n"
#endif
        "\n"
        );
    }
        
}

/// @brief FatFs test parser
///
///
/// - Keywords and arguments are matched against fatfs test functions
/// If ther are matched the function along with its argements are called.
///
///
/// @param[in] str: User supplied command line with FatFs test and arguments.
///
/// @return 1 The ruturn code indicates a command matched.
/// @return 0 if no rules matched
MEMSPACE
int fatfs_tests(int argc,char *argv[])
{
    char *ptr;
    int ind;


    ind = 0;
    ptr = argv[ind++];

    if(!ptr)
        return(0);

// If we have POSIX_TESTS we MUST prefix each test with "fatfs" keyword to avoid name clashing

    if( MATCH(ptr,"fatfs") )
    {
        ptr = argv[ind++];
        if ( !ptr || MATCH(ptr,"help") )
        {
            fatfs_help(1);
            return(1);
        }
    }
#ifdef POSIX_TESTS
    else
    {
        return(0);
    }
#endif

    if (MATCHARGS(ptr,"ls", (ind + 0), argc))
    {
        int i;
        int args = 0;
        printf("ind:%d,argc:%d\n", ind, argc);
        for(i=ind;i<argc;++i)
        {
            //printf("%d:%s\n", i, argv[i]);
            fatfs_ls(argv[i]);
            ++args;
        }
        if(!args)
        {
            fatfs_ls("");
        }
        return(1);
    }

    if (MATCHARGS(ptr,"mmc_test",(ind+0),argc ))
    {
        mmc_test();
        return(1);
    }

    if (MATCHARGS(ptr,"mmc_init",(ind+0),argc))
    {
        mmc_init(1);
        return(1);
    }

    if (MATCHARGS(ptr,"status", (ind + 1), argc))
    {
        fatfs_status(argv[ind]);
        return(1);
    }

#ifdef FATFS_UTILS_FULL
    if (MATCHARGS(ptr,"attrib",(ind+3),argc))
    {
        put_rc( f_chmod(argv[ind],atol(argv[ind+1]),atol(argv[ind+2])) );
        return(1);
    }

    if (MATCHARGS(ptr,"cat", (ind + 1), argc))
    {
        fatfs_cat(argv[ind]);
        return(1);
    }

#if _FS_RPATH
    if (MATCHARGS(ptr,"cd", (ind + 1), argc))
    {
        fatfs_cd(argv[ind]);
        return(1);
    }
#endif

    if (MATCHARGS(ptr,"copy", (ind + 2), argc))
    {
        fatfs_copy(argv[ind],argv[ind+1]);
        return(1);
    }

    if (MATCHARGS(ptr,"create", (ind + 2), argc))
    {
        fatfs_create(argv[ind],argv[ind+1]);
        return(1);
    }

    if (MATCHARGS(ptr,"mkdir", (ind + 1), argc))
    {
        fatfs_mkdir(argv[ind]);
        return(1);
    }

    if (MATCHARGS(ptr,"mkfs", (ind + 0), argc))
    {
        FATFS fs;
        uint8_t *mem;
        int res;
        /* Register work area to the logical drive 0 */
        res = f_mount(&fs, "0:", 0);                    
        put_rc(res);
        if (res)
            return(1);
        mem = safemalloc(1024);
       /* Create FAT volume on the logical drive 0. 2nd argument is ignored. */
        res = f_mkfs("0:", FM_FAT32, 0, mem, 1024);
        safefree(mem);
        put_rc(res);
        return(1);
    }

#if _FS_RPATH
#if _FS_RPATH >= 2
    if (MATCHARGS(ptr,"pwd", (ind + 0), argc))
    {
        fatfs_pwd();
        return(1);
    }
#endif // #if _FS_RPATH >= 2
#endif // #if _FS_RPATH 


    if (MATCHARGS(ptr,"rename", (ind + 2), argc))
    {
        fatfs_rename(argv[ind],argv[ind+1]);
        return(1);
    }

    if (MATCHARGS(ptr,"rmdir", (ind + 1), argc))
    {
        fatfs_rmdir(argv[ind]);
        return(1);
    }

    if (MATCHARGS(ptr,"rm", (ind + 1), argc))
    {
        fatfs_rm(argv[ind]);
        return(1);
    }

    if (MATCHARGS(ptr,"stat", (ind + 1), argc))
    {
        fatfs_stat(argv[ind]);
        return(1);
    }
#endif

    return(0);
}


/// @brief Perform key FatFs diagnostics tests.
///
/// - Perform all basic file tests
/// - Assumes the device is formatted
///
/// @return void
MEMSPACE
void mmc_test(void)
{
    printf("==============================\n");
    printf("START MMC TEST\n");
    fatfs_status("/");
    printf("MMC Directory List\n");
    fatfs_ls("/");

#ifdef FATFS_UTILS_FULL
#if _FS_RPATH
    fatfs_cd("/");
#endif
    fatfs_create("test.txt","this is a test");
    fatfs_cat("test.txt");
    fatfs_ls("/");
    fatfs_create("test.txt","this is a test");
    fatfs_cat("test.txt");
    fatfs_copy("test.txt","test2.txt");
    fatfs_cat("test2.txt");
#if _FS_RPATH
    fatfs_mkdir("/tmp");
    fatfs_copy("test.txt","tmp/test3.txt");
    fatfs_cat("tmp/test3.txt");
    fatfs_cd("/tmp");
    fatfs_pwd();
    fatfs_cat("test3.txt");
    fatfs_ls("");
#endif
#endif

    printf("END MMC TEST\n");
    printf("==============================\n");
}

///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] ptr: pathname of directory to list
///
/// @see fatfs_filinfo_list().
/// @return  void.
MEMSPACE
void fatfs_ls(char *name)
{
    long p1;
    UINT s1, s2;
    int res;
    FILINFO fno;
    DIR dirs;                                     /* Directory object */
    FATFS *fs;
    char buff[256]; 

    if(!name || !*name)
    {
        strcpy(buff,".");
    }
    else
    {
        strcpy(buff,name);
    }
    printf("Listing:[%s]\n",buff);

    res = f_opendir(&dirs, buff);
    if (res) { put_rc(res); return; }
    p1 = s1 = s2 = 0;
    while(1)
    {
        res = f_readdir(&dirs, &fno);
        if ((res != FR_OK) || !fno.fname[0]) break;
        if (fno.fattrib & AM_DIR)
        {
            s2++;
        }
        else
        {
            s1++; p1 += fno.fsize;
        }
        fatfs_filinfo_list(&fno);
#ifdef ESP8266
        optimistic_yield(1000);
        wdt_reset();
#endif
    }
    printf("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
    if (f_getfree(buff, (DWORD*)&p1, &fs) == FR_OK)
        printf(", %10luK bytes free\n", p1 * fs->csize / 2);
}



#ifdef FATFS_UTILS_FULL
/// @brief  Display the contents of a file
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: file name.
///
/// @return  void.
MEMSPACE
void fatfs_cat(char *name)
{
    UINT s1;
    FIL fp;
    int res;
    int i;
    int ret;
    long size;
    char *ptr;

    printf("Reading[%s]\n", name);
    res = f_open(&fp, name, FA_OPEN_EXISTING | FA_READ);
    if (res)
    {
        printf("cat error\n");
        put_rc(res);
        f_close(&fp);
    }

    ptr = safecalloc(512,1);
    if(!ptr)
    {
        printf("Calloc failed!\n");
        f_close(&fp);
    }

    size = 0;
    while(1)
    {
/// @todo FIXME
        res = f_read(&fp, ptr, 512, &s1);
        if(res)
        {
            printf("cat read error\n");
            put_rc(res);
            break;
        }
        ret = s1;
        if (!s1)
        {
            break;
        }
        size += ret;
        for(i=0;i<ret;++i)
        {
            //FIXME putchar depends on fdevopen having been called
            if(stdout)
                putchar(ptr[i]);
            else
                uart_putc(0,ptr[i]);
        }
#ifdef ESP8266
            optimistic_yield(1000);
            wdt_reset();
#endif
    }
    printf("\n");
    f_close(&fp);
    safefree(ptr);
    printf("%lu bytes\n", size);
}


/// @brief  Copy a file.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] from: source file.
/// @param[in] to:   destination file.
///
/// @return  void.

MEMSPACE
void fatfs_copy(char *from,char *to)
{
    UINT s1, s2;
    FIL file1,file2;
    int res;
    long size;
    char *ptr;
#ifdef ESP8266
#define MSIZE 4096
#else
#define MSIZE 512
#endif
    printf("Opening %s\n", from);
    res = f_open(&file1, from, FA_OPEN_EXISTING | FA_READ);
    if (res)
    {
        put_rc(res);
        return;
    }
    printf("Creating %s\n", to);
    res = f_open(&file2, to, FA_CREATE_ALWAYS | FA_WRITE);
    if (res)
    {
        put_rc(res);
        f_close(&file1);
        return;
    }
    ptr = safecalloc(MSIZE,1);
    if(!ptr)
    {
        printf("Calloc failed!\n");
        f_close(&file1);
        f_close(&file2);
        return;
    }
    printf("\nCopying...\n");
    size = 0;
    for (;;)
    {
        res = f_read(&file1, ptr, MSIZE, &s1);
        if (res || s1 == 0) break;                /* error or eof */
        res = f_write(&file2, ptr, s1, &s2);
        size += s2;
        printf("Copied: %08ld\r", size);
        if (res || s2 < s1) break;                /* error or disk full */
    }
    if (res)
        put_rc(res);
    printf("%lu bytes copied.\n", size);
    safefree(ptr);
    f_close(&file1);
    f_close(&file2);
}

/// @brief  Create a new file from a user supplied string.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: name of file to create.
/// @param[in] str: string containing file contents.
///
/// @return  void.
MEMSPACE
void fatfs_create(char *name, char *str)
{
    UINT s1;
    UINT len;
    int res;
    FIL fp;
    printf("Creating [%s]\n", name);
    printf("Text[%s]\n", str);
    res = f_open(&fp, name, FA_CREATE_ALWAYS | FA_WRITE);
    if (res)
    {
        printf("Create error\n");
        put_rc(res);
        f_close(&fp);
        return;
    }

    len = strlen(str);
    res = f_write(&fp, str, (UINT)len, &s1);

    if (res)
    {
        printf("Write error\n");
        put_rc(res);
        return;
    }
    if (len != s1)
    {
        printf("Write error - wanted(%d) got(%d)\n",s1,len);
        put_rc(res);
        return;
    }

    f_close(&fp);
}


#if _FS_RPATH >= 2
/// @brief  Change directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return  void.
MEMSPACE
void fatfs_cd(char *name)
{
    printf("cd [%s]\n", name);
    put_rc(f_chdir(name));
}
#endif



/// @brief  Make a directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return  void
MEMSPACE
void fatfs_mkdir(char *name)
{
    printf("mkdir [%s]\n", name);
    put_rc(f_mkdir(name));

}

#if _FS_RPATH >= 2
/// @brief  Display current working directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @return  void.
MEMSPACE
void fatfs_pwd(void)
{
    int res;
    char str[128];
    res = f_getcwd(str, sizeof(str)-2);
    if (res)
        put_rc(res);
    else
        printf("pwd [%s]\n", str);
}
#endif

/// @brief Rename a file
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] oldpath: old name.
/// @param[in] newpath: new name.
///
/// @return  void
MEMSPACE
void fatfs_rename(const char *oldpath, const char *newpath)
{
/* Rename an object */
    int rc;
    rc = f_rename(oldpath, newpath);
    if(rc)
    {
        put_rc(rc);
    }
}

/// @brief  Delete a file by name.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: File name to delete.
///
/// @return  void.

MEMSPACE
void fatfs_rm(char *name)
{
    printf("rm [%s]\n", name);
    put_rc(f_unlink(name));
}

/// @brief  Delete a directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: Directory name.
///
/// @return  void.
MEMSPACE
void fatfs_rmdir(char *name)
{
    printf("rmdir [%s]\n", name);
    put_rc(f_unlink(name));
}


/// @brief Display FILINFO status of a file by name.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] name: File name.
///
/// @return  void

MEMSPACE
void fatfs_stat(char *name)
{
    FILINFO info;
    int res;

    printf("stat [%s]\n", name);
    res = f_stat(name, &info);
    if(res == FR_OK)
    {
        fatfs_filinfo_list(&info);
    }
    else
    {
        put_rc(res);
    }
}

#endif


