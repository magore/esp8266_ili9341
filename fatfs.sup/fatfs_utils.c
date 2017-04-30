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
#include "fatfs.h"

#ifdef AVR
//#include <stdio.h>
#include <stdlib.h>
#endif

#include "lib/time.h"
#include "lib/stringsup.h"

#include "printf/mathio.h"

#include "fatfs.sup/posix.h"


/// @brief FatFS size lookup table
static  const BYTE ft[] = {0,12,16,32};


/// @brief Display FatFs test diagnostics help menu.
/// @return  void
MEMSPACE
void fatfs_help( void )
{
	printf("\n"
	"attrib p1 p2\n"
	"cat file\n"
#ifdef FATFS_UTILS_FULL
	"cd path\n"
	"copy file1 file2\n"
	"create file str\n"
#endif
	"mmc_init\n"
	"mmc_test\n"
	"ls dir\n"
	"uls dir\n"
#ifdef FATFS_UTILS_FULL
	"mkdir str\n"
	"mkfs\n"
	"pwd\n"
#endif
	"status str\n"
#ifdef FATFS_UTILS_FULL
	"stat str\n"
	"ustat str\n"
	"rm str\n"
	"rmdir str\n"
	"rename file1 file2\n"
#endif
	"fatfs_help\n");
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
#ifdef POSIX_WRAPPERS
    struct stat p;
#endif

	printf("==============================\n");
    printf("START MMC TEST\n");
	fatfs_status("/");
    printf("MMC Directory List\n");
    fatfs_ls("/");
    fatfs_cat("test.txt");

#ifdef FATFS_UTILS_FULL
    fatfs_cd("/");
    fatfs_create("test.txt","this is a test");
    fatfs_cat("test.txt");
    fatfs_ls("/");
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
#endif

#ifdef POSIX_WRAPPERS
    stat("test3.txt", &p);                        // POSIX test
    dump_stat(&p);
#endif

    printf("END MMC TEST\n");
	printf("==============================\n");
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
    FILINFO fno;
    DIR dirs;                                     /* Directory object */
    FATFS *fs;

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;

    printf("Listing:[%s]\n",ptr);

    res = f_opendir(&dirs, ptr);
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
    if (f_getfree(ptr, (DWORD*)&p1, &fs) == FR_OK)
        printf(", %10luK bytes free\n", p1 * fs->csize / 2);
}

int ls_info(char *name, int verbose)
{
	int i;
    struct stat sp;
	uint16_t mask;
	char *cm = "rwx";
	char attr[12], *p;


	if(!verbose)
	{
		printf("%s\n",name);
		return(0);
	}

    if(stat(name, &sp) == -1) 
	{
        printf("can not stat: %s\n", name);
        return(0);
	}


	p = attr;
    if(S_ISDIR(sp.st_mode))
        *p++ = 'd';
	else
        *p++ = '-';

	mask = 4 << 6;
	for(i=0;i<9;++i)
	{
		// User
		if( sp.st_mode & mask)
			*p++ = cm[ i % 3];
		else
			*p++ = '-';
		mask >>= 1;
	}
	*p = 0;

	printf("%s none none %12ld %s %s\n",
		attr, 
        (long) sp.st_size, 
        mctime((time_t)sp.st_mtime),
        basename(name));
	return(1);
}

void ls(char *path, int verbose)
{
	struct stat st;
	DIR *dirp;
	dirent_t *de;
	char name[MAX_NAME_LEN+1];

    printf("Listing:[%s]\n",path);

	if (stat(path, &st)) 
	{ 
		printf("ls: cannot stat [%s]\n", path); 
		return; 
	}

	switch (st.st_mode & S_IFMT) 
	{
		case S_IFREG:
			ls_info(path,verbose);
			break;
		case S_IFDIR:
			dirp = opendir(path);
			if(!dirp)
			{
				printf("opendir failed\n");
				return;
			}
			while ( (de = readdir(dirp)) ) 
			{
				if(de->d_name[0] == 0)
					break;

//printf("name:[%s]\n", de->d_name);

				// FIXME neeed beetter string length tests here
				strncpy(name,path,MAX_NAME_LEN);
				strcat(name,"/");
				strcat(name,de->d_name);
				ls_info(name,verbose);
#ifdef ESP8266
				optimistic_yield(1000);
				wdt_reset();
#endif
			}
			closedir(dirp);
			break;
	  }
}


#ifdef FATFS_UTILS_FULL
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
#endif

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


#ifdef FATFS_UTILS_FULL
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
void fatfs_create(char *name,char *str)
{
    UINT s1;
    UINT len;
    int res;
    FIL fp;
    len = strlen(str);
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
        printf("pwd [%s]\n", str);
#endif
}
#endif


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

    int len;
    char *ptr;
	char name1[128],name2[128];
#ifdef FATFS_UTILS_FULL
    long p1, p2;
	int res;
#endif

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
    else if ((len = token(ptr,"ls")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_ls(name1);
        return(1);
    }
    else if ((len = token(ptr,"uls")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        ls(name1,1);
        return(1);
    }
    else if ((len = token(ptr,"ustat")) )
    {
		struct stat p;	
        ptr += len;
		ptr = get_token(ptr, name1, 126);
		stat(name1, &p);                        // POSIX test
		dump_stat(&p);
        return(1);
    }
    else if ((len = token(ptr,"cat")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_cat(name1);
        return(1);
    }
    else if ((len = token(ptr,"status")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_status(name1);
        return(1);
    }
#ifdef FATFS_UTILS_FULL
    else if ((len = token(ptr,"mkfs")) )
    {
        FATFS fs;
		uint8_t *mem;
        ptr += len;
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
    else if ((len = token(ptr,"create")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_create(name1,ptr);
        return(1);
    }
    else if ((len = token(ptr,"stat")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_stat(name1);
        return(1);
    }
    else if ((len = token(ptr,"rm")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_rm(name1);
        return(1);
    }
    else if ((len = token(ptr,"mkdir")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_mkdir(name1);
        return(1);
    }
    else if ((len = token(ptr,"rmdir")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_rmdir(name1);
        return(1);
    }
    else if ((len = token(ptr,"attrib")) )
    {
        ptr += len;
        ptr=skipspaces(ptr);
        sscanf(ptr,"%lu %lu", &p1,&p2);
        put_rc(f_chmod(ptr, p1, p2));
        return(1);
    }
    else if ((len = token(ptr,"copy")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
		ptr = get_token(ptr, name2, 126);
        fatfs_copy(name1,name2);
        return(1);
    }
    else if ((len = token(ptr,"rename")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
		ptr = get_token(ptr, name2, 126);
        fatfs_rename(name1,name2);
        return(1);
    }
#if _FS_RPATH
    else if ((len = token(ptr,"cd")) )
    {
        ptr += len;
		ptr = get_token(ptr, name1, 126);
        fatfs_cd(name1);
        return(1);
    }
#if _FS_RPATH >= 2
    else if ((len = token(ptr,"pwd")) )
    {
        fatfs_pwd();
        return(1);
    }
#endif // #if _FS_RPATH 
#endif // #if _FS_RPATH >= 2
#endif // #ifdef FATFS_UTILS_FULL
    else if ( (len = token(ptr,"fatfs_help")) )
    {
        fatfs_help();
        return(1);
    }
    return(0);
}
