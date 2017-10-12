/**
 @file posix_tests.c

 @brief fatfs test utilities with user interface

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
#include "user_config.h"

#include "fatfs.h"

#ifdef AVR
//#include <stdio.h>
#include <stdlib.h>
#endif
#include "lib/time.h"
#include "lib/stringsup.h"
#include "printf/mathio.h"
#include "posix.h"
#include "posix_tests.h"

#ifdef ESP8266
#define BUFSIZE 4096
#else
#define BUFSIZE 512
#endif
MEMSPACE 
void posix_help(int full)
{
    printf("posix help\n");
    if(full)
    {
        printf(
#ifdef POSIX_TESTS
            "posix prefix is optional\n"
#endif
        "posix chmod file NNN\n"
        "posix cat file [-p]\n"
        "posix cd dir\n"
        "posix copy file1 file2\n"
        "posix hexdump file [-p]\n"
        "posix log str\n"
        "posix ls dir [-l]\n"
        "posix mkdir dir\n"
        "posix mkfs\n"
        "posix page NN"
        "posix pwd\n"
        "posix stat file\n"
        "posix sum file\n"
        "posix rm file\n"
        "posix rmdir dir\n"
        "posix rename old new\n"
        "posix upload file\n"
        "\n" );
    }
}

/// @brief POSIX tests
///
///
/// @param[in] agc: cont of arguments
///
/// @return 1 The ruturn code indicates a command matched.
/// @return 0 if no rules matched
MEMSPACE
int posix_tests(int argc,char *argv[])
{
    char *ptr;
    int ind;

    ind = 0;
    ptr = argv[ind++];

    if(!ptr)
        return(0);

    if( MATCH(ptr,"posix") )
    {
        ptr = argv[ind++];
        if ( !ptr || MATCH(ptr,"help") )
        {
            posix_help(1);
            return(1);
        }
    }

    if (MATCHARGS(ptr,"cat", (ind + 1), argc))
    {
        int i;
        int page = 0;
        for(i=ind;i<argc;++i)
        {
            if(MATCH(argv[i],"-p"))
                page = 1;
        }
        for(i=ind;i<argc;++i)
        {
            if(!MATCH(argv[i],"-p"))
                cat(argv[ind], page);
        }
        return(1);
    }

    if (MATCHARGS(ptr,"chmod",(ind+2),argc))
    {
        chmod( argv[ind],strtol(argv[ind+1],NULL,8));
        return(1);
    }


    if (MATCHARGS(ptr,"copy", (ind + 2), argc))
    {
        copy(argv[ind],argv[ind+1]);
        return(1);
    }

    if (MATCHARGS(ptr,"cd", (ind + 1), argc))
    {
        chdir(argv[ind]);
        return(1);
    }


    if (MATCHARGS(ptr,"hexdump", (ind + 1), argc))
    {
        int i;
        int page = 0;
        for(i=ind;i<argc;++i)
        {
            if(MATCH(argv[i],"-p"))
                page = 1;
        }
        for(i=ind;i<argc;++i)
        {
            if(!MATCH(argv[i],"-p"))
                hexdump(argv[ind], page);
        }
        return(1);
    }

    if (MATCHARGS(ptr,"log", (ind + 2), argc))
    {
        logfile(argv[ind],argv[ind+1]);
        return(1);
    }

    if (MATCHARGS(ptr,"ls", (ind + 0), argc))
    {
        int i;
        int args = 0;
        for(i=ind;i<argc;++i)
        {
            if(!MATCH(argv[i],"-l"))
                ls(argv[i],1);
            ++args;
        }
        if(!args)
        {
            ls("",1);
        }
        return(1);
    }

    if (MATCHARGS(ptr,"mkfs", (ind + 1), argc))
    {

        mkfs(argv[ind++]);
        return(1);
    }

    if (MATCHARGS(ptr,"mkdir", (ind + 1), argc))
    {
        int mode = 0777;
        if((ind+2) <= argc)
        {
            mode = strtol(argv[ind+1],NULL,8);
        }
        mkdir(argv[ind],mode);
        return(1);
    }

    if (MATCHARGS(ptr,"page", (ind + 1), argc))
    {
        setpage(atoi(argv[ind]));
        return(1);
    }

    if (MATCHARGS(ptr,"pwd", (ind + 0), argc))
    {
        char path[256];
        printf("%s\n", getcwd(path, sizeof(path)-2));
        return(1);
    }

    if (MATCHARGS(ptr,"rename", (ind + 2), argc))
    {
        rename(argv[ind],argv[ind+1]);
        return(1);
    }

    if (MATCHARGS(ptr,"rm", (ind + 1), argc))
    {
        unlink(argv[ind]);
        return(1);
    }

    if (MATCHARGS(ptr,"sum", (ind + 1), argc))
    {
        sum(argv[ind]);
        return(1);
    }

    if (MATCHARGS(ptr,"stat", (ind + 1), argc))
    {
        struct stat p;  
        stat(argv[ind], &p);                        // POSIX test
        dump_stat(&p);
        return(1);
    }

    if (MATCHARGS(ptr,"rmdir", (ind + 1), argc))
    {
        rmdir(argv[ind]);
        return(1);
    }

    if (MATCHARGS(ptr,"upload", (ind + 1), argc))
    {
        upload(argv[ind]);
        return(1);
    }

    return(0);
}

/// @brief  Display the contents of a file
/// @param[in] name: file name.
/// @param[in] option: --p page display
/// @return  void.
MEMSPACE
long cat(char *name, int dopage)
{
    FILE *fp;
    int count = 0;
    int size = 0;
    char line[256];

    fp = fopen(name,"rb");
    if (!fp)
    {
        printf("Can't open: %s\n", name);
        return(0);
    }
    while(fgets(line,sizeof(line)-2,fp) != NULL)
    {
        trim_tail(line);
        size += strlen(line);
        puts(line);
        if(dopage)
        {
            count = testpage(++count);
            if(count < 0)
                break;
        }

#ifdef ESP8266
        optimistic_yield(1000);
        wdt_reset();
#endif
    }
    printf("\n");
    fclose(fp);
    printf("%ld bytes\n", (long)size);
    return(size);
}


/// @brief  Copy a file.
/// - Credit: part of FatFs avr example project (C)ChaN, 2013.
/// @param[in] from: source file.
/// @param[in] to:   destination file.
/// @return  bytes written
MEMSPACE
long copy(char *from,char *to)
{
    FILE *fi,*fo;
    char *buf;
    long size = 0;
    int len;

    printf("Opening %s\n", from);

    fi = fopen(from,"rb");
    if (fi == NULL)
    {
        printf("Can't open: %s\n", from);
        return(0);
    }

    printf("Creating %s\n", to);
    fo = fopen(to,"wb");
    if (fo == NULL)
    {
        printf("Can't open: %s\n", to);
        fclose(fo);
        return(0);
    }

    buf = safecalloc(BUFSIZE,1);
    if(!buf)
    {
        fclose(fi);
        fclose(fo);
        return(0);
    }

    printf("\nCopying...\n");
    while( ( len = fread(buf,1,BUFSIZE,fi) ) > 0)
    {
        if( (int) fwrite(buf,1,len,fo) < len)
        {
            printf("Write error\n");
            break;
        }
        size += len;
        printf("Copied: %08ld\r", size);
#ifdef ESP8266
            optimistic_yield(1000);
            wdt_reset();
#endif
    }
    printf("%lu bytes copied.\n", size);
    safefree(buf);
    fclose(fi);
    fclose(fo);
    return(size);
}



/// @brief hex listing of file with paging, "q" exits
/// @param[in] *name: file to hexdump
/// @retrun void
MEMSPACE
int hexdump(char *name, int dopage)
{
    long addr;
    int i,len,count;

    FILE *fi;
    char buf[0x20];

    fi=fopen(name,"rb");
    if(fi == NULL) 
    {
        printf("Can' open: %s\n", name);
        return(0);
    }

    count = 0;
    addr = 0;
    while( (len = fread(buf,1, 16, fi)) > 0) 
    {
        printf("%08lx : ", addr);

        for(i=0;i<len;++i) 
            printf("%02x ",0xff & buf[i]);
        for(;i<16;++i) 
            printf("   ");

        printf(" : ");

        for(i=0;i<len;++i) 
        {
            if(buf[i] >= 0x20 && buf[i] <= 0x7e)
                putchar(buf[i]);
            else
                putchar('.');
        }
        for(;i<16;++i) 
            putchar('.');

        printf("\n");
        addr += len;
        if(dopage)
        {
            count = testpage(++count);
            if(count < 0)
                break;
        }
#ifdef ESP8266
        optimistic_yield(1000);
        wdt_reset();
#endif
    }
    printf("\n");
    fclose(fi);
    return(1);
}

/// @brief Used to page output of functions like cat, hexdump, etc
/// @param[in] *name: file to hexdump
/// @retrun void
static int _pagesize = 25;
MEMSPACE
int setpage(int count)
{
    _pagesize = count;
    return(_pagesize);
}


/// @brief Used to page output of functions like cat, hexdump, etc
/// @param[in] *name: file to hexdump
/// @retrun -1 = quit, return count or 0 at new page
MEMSPACE
int testpage(int count)
{
    int c;
    if(count >= _pagesize)
    {
        printf("More..");
#ifdef ESP8266
        while (!kbhit(0))
        {
            optimistic_yield(1000);
            wdt_reset();
        }
#endif
        c = getchar();
        printf("\r");
        if(c == 'q')
            return(-1);         // quit
        if(c == '\n')
            return(_pagesize-1); // single line
        return(0);              // new page
    }
    return (count);
}


/// @brief  list one file
/// @param[in] path: file name or directory
/// @param[in] verbose: 1 = detail, 0 = name only
/// @return  1 on success 0 on fail
MEMSPACE
int ls_info(char *name, int verbose)
{
    int i;
    struct stat sp;
    uint16_t mask;
    char *cm = "rwx";
    char attr[12], *p;


    if(stat(name, &sp) == -1) 
    {
        printf("can not stat: %s\n", name);
        return(0);
    }

    if(!verbose)
    {
        printf("%s\n",basename(name));
        return(1);
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

/// @brief  Directory listing
/// @param[in] path: file name or directory
/// @param[in] option: -l for detail
/// @return  number of files
MEMSPACE
int ls(char *name, int verbose)
{
    struct stat st;
    DIR *dirp;
    int files = 0;
    int len,len2;
    dirent_t *de;
    char fullpath[MAX_NAME_LEN+1];

    fullpath[0] = 0;
    if(!name || !*name || MATCH(name,".") )
    {
        if( !getcwd(fullpath, sizeof(fullpath)-2) )
        {
            printf("ls: Can't get current directory\n"); 
            return(0); 

        }
    }
    else
    {
        strcpy(fullpath,name);
    }
    len = strlen(fullpath);


    printf("Listing:[%s]\n",fullpath);

    if (stat(fullpath, &st)) 
    { 
        printf("ls: cannot stat [%s]\n", fullpath); 
        return(0); 
    }

    switch (st.st_mode & S_IFMT) 
    {
    case S_IFREG:
        ls_info(fullpath,verbose);
        break;
    case S_IFDIR:
        dirp = opendir(fullpath);
        if(!dirp)
        {
            printf("opendir failed\n");
            return(0);
        }
        while ( (de = readdir(dirp)) ) 
        {
            if(de->d_name[0] == 0)
                break;
            // FIXME neeed beetter string length tests here
            len2 = strlen(de->d_name);
            if(len + len2 >= MAX_NAME_LEN)
            {
                printf("name:[%s] too long with full path\n",de->d_name);
                continue;
            }
            if(!MATCH(fullpath,"/") )
            {
                strcat(fullpath,"/");
            }
            strcat(fullpath,de->d_name);
            files +=ls_info(fullpath,verbose);
            // restore path
            fullpath[len] = 0;
#ifdef ESP8266
            optimistic_yield(1000);
            wdt_reset();
#endif
        }
        closedir(dirp);
        break;
    }
    printf("Files: %d\n", (int)files);
    return(files);
}


/// @brief  Log string to a file
/// @param[in] name: name of file to create.
/// @param[in] str: string containing file contents.
/// @return  size of string, or 0 on error
MEMSPACE
long logfile(char *name, char *str)
{
    long size = 0;
    FILE *fo;

    fo = fopen(name,"ab");
    if (fo)
    {
        printf("Can't open: %s\n", name);
        return(0);
    }

    size = strlen(str);
    if( fwrite(str,1,size,fo) < size)
    {
        printf("Write error\n");
        return(0);
    }
    fclose(fo);
    return(size);
}

/// @brief sum of a file with 16bit hex and integer results
/// @param[in] *name: file to sum
/// @retrun void
MEMSPACE
uint16_t sum(char *name)
{

    FILE *fi;
    uint16_t sum;
    int i,len;
    uint8_t buffer[256];

    fi=fopen(name,"rb");
    if(fi == NULL) 
    {
        printf("Can' open: %s\n", name);
        return(0);
    }
    sum = 0;
    while( (len = fread(buffer,1, 256, fi)) > 0) 
    {
        for(i=0;i<len;++i) 
            sum += (0xff & buffer[i]);
#ifdef ESP8266
            optimistic_yield(1000);
            wdt_reset();
#endif
    }
    fclose(fi);
    printf("Sum: %04Xh, %5u\n", (int) sum, (unsigned int) sum);
    return(sum);
}

/// FIXME TODO
/// @brief Capture an ASCII file to sdcard
/// First blank line exits capture
/// @param[in] *name: file to save on sdcard
/// @retrun void
MEMSPACE
long upload(char *name)
{
    int len,len2;
    long size = 0;
    FILE *fp;
    char buffer[256];

    fp = fopen(name, "wb");
    if( fp == NULL)
    {
        printf("Can' open: %s\n", name);
        return(0);
    }

    while(1)
    {
        if(fgets(buffer,254,stdin) == NULL)
            break;
        len = strlen(buffer);
        if(len < 1)
            break;
        strcat(buffer,"\n");
        len = strlen(buffer);
        len2 = fwrite(buffer, 1, len,fp);
        if(len != len2)
            break;
        size += len;
    }

    fclose(fp);
    sync();
    return(size);
}
