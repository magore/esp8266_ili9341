/**
 @file fatfs/posix.h

 @brief POSIX wrapper for FatFS
   - Provides many of the common Posix/linux functions 
     - close
     - clrerror 
     - fclose
     - fdevopen
     - feof
     - ferror
     - fgetc
     - fgetpos
     - fileno
     - fopen
     - fputc
     - fread
     - fseek
     - fstat
     - ftell
     - ftruncate
     - fwrite
     - isatty
     - lseek
     - open
     - perror
     - read
     - rename
     - rewind
     - rmdir
     - stat
     - strerror
     - sync
     - syncfs
     - truncate
     - unlink
     - write
 @par Copyright &copy; 2015 Mike Gore, GPL License
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

#ifndef _POSIX_H_
#define _POSIX_H_

#define POSIX

#include <user_config.h>
#include "ff.h"
#include "disk.h"
#include "fatfs_utils.h"

#ifdef ESP8266
#undef strerror_r
#endif
// ====================================================================

#ifdef NO_STDIO
struct __file {
    char    *buf;       /* buffer pointer */
    unsigned char unget;    /* ungetc() buffer */
    uint8_t flags;      /* flags, see below */
#define __SRD   0x0001      /* OK to read */
#define __SWR   0x0002      /* OK to write */
#define __SSTR  0x0004      /* this is an sprintf/snprintf string */
#define __SPGM  0x0008      /* fmt string is in progmem */
#define __SERR  0x0010      /* found error */
#define __SEOF  0x0020      /* found EOF */
#define __SUNGET 0x040      /* ungetc() happened */
#define __SMALLOC 0x80      /* handle is malloc()ed */
#if 0
/* possible future extensions, will require uint16_t flags */
#define __SRW   0x0100      /* open for reading & writing */
#define __SLBF  0x0200      /* line buffered */
#define __SNBF  0x0400      /* unbuffered */
#define __SMBF  0x0800      /* buf is from malloc */
#endif
    int size;       /* size of buffer */
    int len;        /* characters read or written so far */
    int (*put)(char, struct __file *);  				/* write one char to device */
    int (*get)(struct __file *);    					/* read one char from device */
// FIXME add all low level functions here like _open, _close, ... like newlib does
    void    *udata;     /* User defined and accessible data. */
};

typedef struct __file FILE;

// Hard coded stdin,stdout and stderr locations
#define stdin (__iob[0])
#define stdout (__iob[1])
#define stderr (__iob[2])


// Define putc and getc in terms of the posix fgetc() and fputc() interface
// MAKE SURE that fdevopen() has been called BEFORE any input/output is processed
// @see uart.c for the fdevopen() call
#define putc(__c, __stream) fputc(__c, __stream)
#define getc(__stream) fgetc(__stream)
/**
   The macro \c putchar sends character \c c to \c stdout.
*/
#define putchar(__c) fputc(__c,stdout)

// Device IO udata
#define fdev_set_udata(stream, u) do { (stream)->udata = u; } while(0)
#define fdev_get_udata(stream) ((stream)->udata)
#endif
// =============================================================

#ifdef ESP8266
#undef strerror_r
#endif

#define EOF (-1)

// Seek
#define SEEK_SET 0
#define SEEK_CUR 1
#define SEEK_END 2

#undef EDOM
#undef ERANGE

///@brief Maximum number of POSIX file handles.
#define MAX_FILES 16
extern FILE *__iob[MAX_FILES];
extern int errno;

// device status flags
#define _FDEV_EOF (-1)
#define _FDEV_ERR (-2)
// device read/write flags
#define _FDEV_SETUP_READ  __SRD /**< fdev_setup_stream() with read intent */
#define _FDEV_SETUP_WRITE __SWR /**< fdev_setup_stream() with write intent */
#define _FDEV_SETUP_RW    (__SRD|__SWR) /**< fdev_setup_stream() with read/write intent */

///@brief compare two strings, 1 = match, 0 mismatch.
#define modecmp(str, pat) (strcmp(str, pat) == 0 ? 1: 0)

// @brief posix errno values
enum POSIX_errno
{
    EOK,        /*< 	0   NO ERROR */
    EPERM,     /*< 	1   Operation not permitted */
    ENOENT,    /*< 	2   No such file or directory */
    ESRCH,     /*< 	3   No such process */
    EINTR,     /*< 	4   Interrupted system call */
    EIO,       /*< 	5   I/O error */
    ENXIO,     /*< 	6   No such device or address */
    E2BIG,     /*< 	7   Argument list too long */
    ENOEXEC,   /*< 	8   Exec format error */
    EBADF,     /*< 	9   Bad file number */
    ECHILD,    /*< 	10  No child processes */
    EAGAIN,    /*< 	11  Try again */
    ENOMEM,    /*< 	12  Out of memory */
    EACCES,    /*< 	13  Permission denied */
    EFAULT,    /*< 	14  Bad address */
    ENOTBLK,   /*< 	15  Block device required */
    EBUSY,     /*< 	16  Device or resource busy */
    EEXIST,    /*< 	17  File exists */
    EXDEV,     /*< 	18  Cross-device link */
    ENODEV,    /*< 	19  No such device */
    ENOTDIR,   /*< 	20  Not a directory */
    EISDIR,    /*< 	21  Is a directory */
    EINVAL,    /*< 	22  Invalid argument */
    ENFILE,    /*< 	23  File table overflow */
    EMFILE,    /*< 	24  Too many open files */
    ENOTTY,    /*< 	25  Not a typewriter */
    ETXTBSY,   /*< 	26  Text file busy */
    EFBIG,     /*< 	27  File too large */
    ENOSPC,    /*< 	28  No space left on device */
    ESPIPE,    /*< 	29  Illegal seek */
    EROFS,     /*< 	30  Read-only file system */
    EMLINK,    /*< 	31  Too many links */
    EPIPE,     /*< 	32  Broken pipe */
    EDOM,      /*< 	33  Math argument out of domain of func */
    ERANGE,    /*< 	34  Math result not representable */
    EBADMSG    /*< 	35  Bad Message */
};
///@brief POSIX open modes  - no other combination are allowed.
/// - man page open(2)
/// - Note: The POSIX correct test of O_RDONLY is: (mode & O_ACCMODE) == O_RDONLY.
#define O_ACCMODE  00000003 /*< read, write, read-write modes */
#define O_RDONLY   00000000 /*< Read only */
#define O_WRONLY   00000001 /*< Write only */
#define O_RDWR     00000002 /*< Read/Write */
#define O_CREAT    00000100 /*< Create file only if it does not exist */
#define O_EXCL     00000200 /*< O_CREAT option, Create fails if file exists 
*/
#define O_NOCTTY   00000400 /*< @todo */
#define O_TRUNC    00001000 /*< Truncate if exists */
#define O_APPEND   00002000 /*< All writes are to EOF */
#define O_NONBLOCK 00004000 /*< @todo */
#define O_BINARY   00000004 /*< Binary */
#define O_TEXT     00000004 /*< Text End Of Line translation */


///@brief POSIX File types, see fstat and stat.
#define S_IFMT     0170000  /*< These bits determine file type.  */
#define S_IFDIR    0040000  /*< Directory.  */
#define S_IFCHR    0020000  /*< Character device.  */
#define S_IFBLK    0060000  /*< Block device.  */
#define S_IFREG    0100000  /*< Regular file.  */
#define S_IFIFO    0010000  /*< FIFO.  */
#define S_IFLNK    0120000  /*< Symbolic link.  */
#define S_IFSOCK   0140000  /*< Socket.  */
#define S_IREAD    0400     /*< Read by owner.  */
#define S_IWRITE   0200     /*< Write by owner.  */
#define S_IEXEC    0100     /*< Execute by owner.  */

///@brief POSIX Fiel type test macros.
#define S_ISTYPE(mode, mask)  (((mode) & S_IFMT) == (mask))
#define S_ISDIR(mode)    S_ISTYPE((mode), S_IFDIR)
#define S_ISCHR(mode)    S_ISTYPE((mode), S_IFCHR)
#define S_ISBLK(mode)    S_ISTYPE((mode), S_IFBLK)
#define S_ISREG(mode)    S_ISTYPE((mode), S_IFREG)

//@brief POSIX File permissions, see fstat and stat  
#define S_IRUSR S_IREAD                     /*< Read by owner.  */
#define S_IWUSR S_IWRITE                    /*< Write by owner.  */
#define S_IXUSR S_IEXEC                     /*< Execute by owner.  */
#define S_IRWXU (S_IREAD|S_IWRITE|S_IEXEC)	/*< Read,Write,Execute by owner */

#define S_IRGRP (S_IRUSR >> 3)              /*< Read by group.  */
#define S_IWGRP (S_IWUSR >> 3)              /*< Write by group.  */
#define S_IXGRP (S_IXUSR >> 3)              /*< Execute by group.  */
#define S_IRWXG (S_IRWXU >> 3)				/*< Read,Write,Execute by user */

#define S_IROTH (S_IRGRP >> 3)              /*< Read by others.  */
#define S_IWOTH (S_IWGRP >> 3)              /*< Write by others.  */
#define S_IXOTH (S_IXGRP >> 3)              /*< Execute by others.  */
#define S_IRWXO (S_IRWXG >> 3)				/*< Read,Write,Execute by other */

#define FATFS_R (S_IRUSR | S_IRGRP | S_IROTH)	/*< FatFs Read perms */
#define FATFS_W (S_IWUSR | S_IWGRP | S_IWOTH)	/*< FatFs Write perms */
#define FATFS_X (S_IXUSR | S_IXGRP | S_IXOTH)	/*< FatFs Execute perms */

///@brief Standard POSIX typedefs.
///
/// - Using these makes code portable accross many acrchitectures
typedef uint32_t dev_t;		/*< dev_t for this architecture */
typedef uint32_t ino_t;		/*< ino_t for this architecture */
typedef uint32_t mode_t;    /*< mode_t for this architecture */
typedef uint32_t nlink_t;  	/*< nlink_t for this architecture */ 
typedef uint16_t uid_t;     /*< uid_t for this architecture */
typedef uint16_t gid_t;     /*< gid_t for this architecture */
typedef uint32_t off_t;     /*< off_t for this architecture */
typedef uint32_t blkcnt_t;  /*< blkcnt_t for this architecture */
typedef uint32_t blksize_t; /*< blksize_t for this architecture */
typedef uint32_t time_t;    /*< time_t for this architecture */
typedef int32_t  ssize_t;	/*< ssize_t for this architecture */

///@brief POSIX stat structure
///@see stat()
///@see fstat()
struct stat
{
    dev_t     st_dev;    /*<  ID of device containing file */
    ino_t     st_ino;    /*<  inode number */
    mode_t    st_mode;   /*<  protection */
    nlink_t   st_nlink;  /*<  number of hard links */
    uid_t     st_uid;    /*<  user ID of owner */
    gid_t     st_gid;    /*<  group ID of owner */
    dev_t     st_rdev;   /*<  device ID (if special file) */
    off_t     st_size;   /*<  total size, in bytes */
    blksize_t st_blksize;/*<  blocksize for filesystem I/O */
    blkcnt_t  st_blocks; /*<  number of 512B blocks allocated */
    time_t    st_atime;  /*<  time of last access */
    time_t    st_mtime;  /*<  time of last modification */
    time_t    st_ctime;  /*<  time of last status change */
};

///@brief POSIX lstat()
///@see stat()
#define lstat stat

// ============================================================================
/* posix.c */
MEMSPACE FILE *fileno_to_stream ( int fileno );
MEMSPACE int fileno ( FILE *stream );
MEMSPACE FIL *fileno_to_fatfs ( int fileno );
MEMSPACE int fatfs_to_fileno ( FIL *fh );
#ifdef NO_STDIO
	MEMSPACE int feof ( FILE *stream );
#endif
#ifdef NO_STDIO
	MEMSPACE int ferror ( FILE *stream );
#endif
#ifdef NO_STDIO
	MEMSPACE void clrerror ( FILE *stream );
#endif
MEMSPACE int new_file_descriptor ( void );
#ifdef NO_STDIO
MEMSPACE FILE *fdevopen ( int (*put )(char ,FILE *), int (*get )(FILE *));
#endif
MEMSPACE int free_file_descriptor ( int fileno );
MEMSPACE int isatty ( int fileno );
MEMSPACE void perror ( const char *s );
MEMSPACE char *strerror ( int errnum );
MEMSPACE char *strerror_r ( int errnum , char *buf , size_t buflen );
MEMSPACE int fatfs_to_errno ( FRESULT Result );
MEMSPACE int posix_fopen_modes_to_open ( const char *mode );
#ifdef NO_STDIO
	MEMSPACE int fgetc ( FILE *stream );
#endif
#ifdef NO_STDIO
	MEMSPACE int fputc ( int c , FILE *stream );
#endif
MEMSPACE int open ( const char *pathname , int flags );
MEMSPACE FILE *fopen ( const char *path , const char *mode );
MEMSPACE int close ( int fileno );
MEMSPACE int syncfs ( int fd );
MEMSPACE void sync ( void );
MEMSPACE int fclose ( FILE *stream );
MEMSPACE ssize_t write ( int fd , const void *buf , size_t count );
MEMSPACE size_t fwrite ( const void *ptr , size_t size , size_t nmemb , FILE *stream );
MEMSPACE ssize_t read ( int fd , const void *buf , size_t count );
MEMSPACE size_t fread ( void *ptr , size_t size , size_t nmemb , FILE *stream );
MEMSPACE size_t lseek ( int fileno , size_t position , int whence );
MEMSPACE int fseek ( FILE *stream , long offset , int whence );
MEMSPACE long ftell ( FILE *stream );
MEMSPACE void rewind ( FILE *stream );
MEMSPACE int fgetpos ( FILE *stream , size_t *pos );
MEMSPACE int fsetpos ( FILE *stream , size_t *pos );
MEMSPACE int unlink ( const char *pathname );
MEMSPACE int rmdir ( const char *pathname );
MEMSPACE time_t fat_time_to_unix ( uint16_t date , uint16_t time );
MEMSPACE int ftruncate ( int fd , off_t length );
MEMSPACE int truncate ( const char *path , off_t length );
MEMSPACE int fstat ( int fd , struct stat *buf );
MEMSPACE int stat ( char *name , struct stat *buf );
MEMSPACE char *mctime ( time_t timev );
MEMSPACE void dump_stat ( struct stat *sp );
MEMSPACE int rename ( const char *oldpath , const char *newpath );
MEMSPACE int dirname ( char *str );
MEMSPACE char *basename ( char *str );
MEMSPACE char *baseext ( char *str );


// ============================================================================
#endif                                            //_POSIX_H_
