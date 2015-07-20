/**
 @file fatfs/fatfs_utils.c

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

#include "user_config.h"
#include "posix.h"
#include "disk.h"
#include "fatfs_utils.h"

/// @brief FatFS size lookup table
static  const BYTE ft[] = {0,12,16,32};

/// @brief Perform key FatFs diagnostics tests.
///
/// - Perform all basic file tests
/// - Assumes the device is formatted
///
/// @return void
MEMSPACE
void mmc_test(void)
{
    struct stat p;

	DEBUG_PRINTF("==============================\n");
    DEBUG_PRINTF("START MMC TEST\n");
	fatfs_status("/");
    DEBUG_PRINTF("MMC Directory List\n");
    fatfs_ls("/");
    fatfs_cd("/");
    fatfs_create("test.txt","this is a test");
    fatfs_cat("test.txt");
    fatfs_copy("test.txt","test2.txt");
    fatfs_cat("test2.txt");
    fatfs_mkdir("/tmp");
    fatfs_copy("test.txt","tmp/test3.txt");
    fatfs_cat("tmp/test3.txt");
    fatfs_cd("/tmp");
    fatfs_pwd();
    fatfs_ls("");
    fatfs_cat("test3.txt");
    stat("test3.txt", &p);                        // POSIX test
    dump_stat(&p);

    DEBUG_PRINTF("END MMC TEST\n");
	DEBUG_PRINTF("==============================\n");
}


/// @brief Display FatFs test diagnostics help menu.
///
/// @return  void
MEMSPACE
void fatfs_help( void )
{
    DEBUG_PRINTF("debug N\n"
        "mmc_init\n"
        "mmc_test\n"
        "ls dir\n"
        "create file str\n"
        "cat file\n"
        "status str\n"
        "stat str\n"
        "rm str\n"
        "mkdir str\n"
        "rmdir str\n"
        "attrib p1 p2\n"
        "copy file1 file2\n"
        "rename file1 file2\n"
        "cd path\n"
        "pwd\n"
        "fatfs_help\n");
}



/// @brief  List files under a specified directory
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @param[in] ptr: pathname of directory to list
///
/// @see fatfs_filinfo_list().
/// @return  void.
MEMSPACE
void fatfs_ls(char *ptr)
{
    long p1;
    UINT s1, s2;
    int res;
    FILINFO *fno;
    DIR dirs;                                     /* Directory object */
    FATFS *fs;

    fno = fatfs_alloc_finfo(0);
    if(fno == NULL)
    {
        return;
    }

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;

    DEBUG_PRINTF("Listing:[%s]\n",ptr);

    res = f_opendir(&dirs, ptr);
    if (res) { put_rc(res); return; }
    p1 = s1 = s2 = 0;
    while(1)
    {
        res = f_readdir(&dirs, fno);
        if ((res != FR_OK) || !fno->fname[0]) break;
        if (fno->fattrib & AM_DIR)
        {
            s2++;
        }
        else
        {
            s1++; p1 += fno->fsize;
        }
        fatfs_filinfo_list(fno);
    }
    DEBUG_PRINTF("%4u File(s),%10lu bytes total\n%4u Dir(s)", s1, p1, s2);
    if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
        DEBUG_PRINTF(", %10luK bytes free\n", p1 * fs->csize / 2);
}

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
    int res;
    FIL fp;
    int i;
	long size =0;
	char *ptr;
	int ret;

    DEBUG_PRINTF("Reading[%s]\n", name);
    res = f_open(&fp, name, FA_OPEN_EXISTING | FA_READ);
    if (res)
    {
        DEBUG_PRINTF("cat error\n");
        put_rc(res);
        f_close(&fp);
        return;
    }

	ptr = calloc(514,1);
	if(!ptr)
	{
		DEBUG_PRINTF("Calloc failed!\n");
		f_close(&fp);
		return;
	}
    while(1)
    {
/// @todo FIXME
        res = f_read(&fp, ptr, 512, &s1);
        if(res)
        {
            DEBUG_PRINTF("cat read error\n");
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
            putchar(ptr[i]);
    }
    DEBUG_PRINTF("\n");
    f_close(&fp);
	free(ptr);
    DEBUG_PRINTF("%lu bytes\n", size);
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
    long p1;
	char *ptr;
	

    DEBUG_PRINTF("Opening %s\n", from);
    res = f_open(&file1, from, FA_OPEN_EXISTING | FA_READ);
    if (res)
    {
        put_rc(res);
        return;
    }
    DEBUG_PRINTF("Creating %s\n", to);
    res = f_open(&file2, to, FA_CREATE_ALWAYS | FA_WRITE);
    if (res)
    {
        put_rc(res);
        f_close(&file1);
        return;
    }
	ptr = calloc(512,1);
	if(!ptr)
	{
		DEBUG_PRINTF("Calloc failed!\n");
        f_close(&file1);
        f_close(&file2);
		return;
	}
    DEBUG_PRINTF("\nCopying...\n");
    p1 = 0;
    for (;;)
    {
        res = f_read(&file1, ptr, 512, &s1);
        if (res || s1 == 0) break;                /* error or eof */
        res = f_write(&file2, ptr, s1, &s2);
        p1 += s2;
        DEBUG_PRINTF("Copied: %08ld\r", p1);
        if (res || s2 < s1) break;                /* error or disk full */
    }
    if (res)
        put_rc(res);
    DEBUG_PRINTF("%lu bytes copied.\n", p1);
	free(ptr);
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
void fatfs_create(char *name,char *str)
{
    UINT s1;
    UINT len;
    int res;
    FIL fp;
    len = strlen(str);
    DEBUG_PRINTF("Creating [%s]\n", name);
    DEBUG_PRINTF("Text[%s]\n", str);
    res = f_open(&fp, name, FA_CREATE_ALWAYS | FA_WRITE);
    if (res)
    {
        DEBUG_PRINTF("Create error\n");
        put_rc(res);
        f_close(&fp);
        return;
    }
    res = f_write(&fp, str, (UINT)len, &s1);
    if (res)
    {
        DEBUG_PRINTF("Write error\n");
        put_rc(res);
        return;
    }
    if (len != s1)
    {
        DEBUG_PRINTF("Write error - wanted(%d) got(%d)\n",s1,len);
        put_rc(res);
        return;
    }
    f_close(&fp);
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
    DEBUG_PRINTF("rm [%s]\n", name);
    put_rc(f_unlink(name));
}


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
    DEBUG_PRINTF("mkdir [%s]\n", name);
    put_rc(f_mkdir(name));

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
    DEBUG_PRINTF("rmdir [%s]\n", name);
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

    DEBUG_PRINTF("stat [%s]\n", name);
    f_stat(name, &info);
    fatfs_filinfo_list(&info);
}

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
    DEBUG_PRINTF("cd [%s]\n", name);
    put_rc(f_chdir(name));
}


/// @brief  Display current working directory.
///
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
///
/// @return  void.

MEMSPACE
void fatfs_pwd(void)
{
#if _FS_RPATH >= 2
    int res;
    char str[128];
    res = f_getcwd(str, sizeof(str)-2);
    if (res)
        put_rc(res);
    else
        DEBUG_PRINTF("pwd [%s]\n", str);
#endif
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
int fatfs_tests(char *str)
{

	int res;
    int len;
    char *ptr;
    long p1, p2;

    ptr = skipspaces(str);

    if ((len = token(ptr,"mmc_test")) )
    {
        ptr += len;
        mmc_test();
        return(1);
    }
    else if ((len = token(ptr,"mmc_init")) )
    {
        ptr += len;
        mmc_init(1);
        return(1);
    }
    else if ((len = token(ptr,"mkfs")) )
    {
        FATFS fs;
        ptr += len;
        res = f_mount(&fs, "0:", 0);                    /* Register work area to the logical drive 0 */
		put_rc(res);
		if (res)
			return(1);
        res = f_mkfs("0:", 0, 0);                       /* Create FAT volume on the logical drive 0. 2nd argument is ignored. */
		put_rc(res);
        return(1);
    }
    else if ((len = token(ptr,"ls")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_ls(ptr);
        return(1);
    }
    else if ((len = token(ptr,"create")) )
    {
        char *name,*end;
        ptr += len;
        name=skipspaces(ptr);
        end=nextspace(name);
        ptr=skipspaces(end);
        *end=0;
        fatfs_create(name,ptr);
        return(1);
    }
    else if ((len = token(ptr,"cat")) )
    {
        ptr += len;
        ptr = skipspaces(ptr);
        fatfs_cat(ptr);
        return(1);
    }
    else if ((len = token(ptr,"status")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_status(ptr);
        return(1);
    }
    else if ((len = token(ptr,"stat")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_stat(ptr);
        return(1);
    }
    else if ((len = token(ptr,"rm")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_rm(ptr);
        return(1);
    }
    else if ((len = token(ptr,"mkdir")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_mkdir(ptr);
        return(1);
    }
    else if ((len = token(ptr,"rmdir")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_rmdir(ptr);
        return(1);
    }
    else if ((len = token(ptr,"attrib")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        t_sscanf(ptr,"%lu %lu", &p1,&p2);
        put_rc(f_chmod(ptr, p1, p2));
        return(1);
    }
    else if ((len = token(ptr,"copy")) )
    {
        char name1[128],name2[128];
        ptr += len;
        ptr=skipspaces(ptr);
        t_sscanf(ptr,"%s %s",name1,name2);
        fatfs_copy(name1,name2);
        return(1);
    }
    else if ((len = token(ptr,"rename")) )
    {
        char name1[128],name2[128];
        ptr += len;
        ptr=skipspaces(ptr);
        t_sscanf(ptr,"%s %s",name1,name2);
        fatfs_rename(name1,name2);
        return(1);
    }
#if _FS_RPATH
    else if ((len = token(ptr,"cd")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        fatfs_cd(ptr);
        return(1);
    }
#if _FS_RPATH >= 2
    else if ((len = token(ptr,"pwd")) )
    {
        fatfs_pwd();
        return(1);
    }
    else if ( (len = token(ptr,"help")) )
    {
        fatfs_help();
        return(1);
    }
    return(0);
#endif
#endif
}
