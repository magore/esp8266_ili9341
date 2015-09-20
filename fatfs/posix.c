/**
 @file fatfs/posix.c

 @brief POSIX wrapper for FatFS
   - Provides many of the common Posix/linux functions 
     - fdevopen 
     - isatty 
     - perror 
     - strerror 
     - fgetc 
     - fputc 
     - open 
     - fopen 
     - close 
     - syncfs 
     - sync 
     - fclose 
     - write 
     - fwrite 
     - read 
     - fread 
     - lseek 
     - fseek 
     - ftell 
     - rewind 
     - fgetpos 
     - fsetpos 
     - unlink 
     - rmdir 
     - ftruncate 
     - truncate 
     - fstat 
     - stat 
     - rename 

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


#include <user_config.h>

#include "ff.h"
#include "disk.h"
#include "diskio.h"

#include "posix.h"

#ifdef ESP8266
// FIXME ESP8266 library conflict
#undef strerror_r
#endif

///  Note: fdevopen assigns stdin,stdout,stderr

///@brief POSIX errno.
int errno;

///@brief Maximum number or POSIX file handels.
#define MAX_FILES 16

///@brief POSIX fileno to POSIX FILE stream table
///
/// - Note: the index of __iob[] is reffered to "fileno".
/// - Reference: libc/avr-libc-1.8.0/libc/stdio.
/// - stdin = __iob[0].
/// - __iob[1] = stdout.
/// - __iob[2] = stderr.
FILE *__iob[MAX_FILES];

/// @brief POSIX error messages for each errno value.
///
/// - man page errno (3)
const char *sys_errlist[] =
{
    "OK",
    "Operation not permitted",
    "No such file or directory",
    "No such process",
    "Interrupted system call",
    "I/O error",
    "No such device or address",
    "Argument list too long",
    "Exec format error",
    "Bad file number",
    "No child processes",
    "Try again",
    "Out of memory",
    "Permission denied",
    "Bad address",
    "Block device required",
    "Device or resource busy",
    "File exists",
    "Cross-device link",
    "No such device",
    "Not a directory",
    "Is a directory",
    "Invalid argument",
    "File table overflow",
    "Too many open files",
    "Not a typewriter",
    "Text file busy",
    "File too large",
    "No space left on device",
    "Illegal seek",
    "Read-only file system",
    "Too many links",
    "Broken pipe",
    "Math argument out of domain of func",
    "Math result not representable",
    "Bad Message",
    NULL
};

/// @brief Convert POSIX fileno to POSIX FILE stream pointer.
///
/// - inverse of POSIX fileno()
/// - man page fileno (3)
///
/// @param[in] fileno: POSIX fileno is the index of __iob[].
///
/// @see fileno()
/// @return FILE * on success
/// @return NULL on error with errno set,  NULL if fileno out of bounds
MEMSPACE
FILE *fileno_to_stream(int fileno)
{
    FILE *stream;
    if(fileno < 0 || fileno >= MAX_FILES)
    {
        errno = EBADF;
        return(NULL);
    }

    stream = __iob[fileno];
    if(stream == NULL)
    {
        errno = EBADF;
        return(NULL);
    }
    return(stream);
}


/// @brief Convert POSIX stream pointer to POSIX fileno (index of __iob[])
///
/// - man page fileno (3)
/// @param[in] stream: stream pointer
///
/// @return  int fileno on success
/// @return -1 with errno = EBAFD if stream is NULL or not found
MEMSPACE
int fileno(FILE *stream)
{
    int fileno;

    if(stream == NULL)
    {
        errno = EBADF;
        return(-1);
    }

    for(fileno=0; fileno<MAX_FILES; ++fileno)
    {
        if ( __iob[fileno] == stream)
            return(fileno);
    }
    return(-1);
}


/// @brief  Convert POSIX fileno to FatFS handle
///
/// - FatFS file handle is pointed to by the avr-libc stream->udata.
///
/// @param[in] fileno: fileno of file
///
/// @return FIL * FatFS file handle on success.
/// @return NULL if POSIX fileno is invalid NULL 
MEMSPACE
FIL *fileno_to_fatfs(int fileno)
{
    FILE *stream;
    FIL *fh;

    if(isatty( fileno ))
    {
        errno = EBADF;
        return(NULL);
    }

	// checks if fileno out of bounds
    stream = fileno_to_stream(fileno);
    if( stream == NULL )
        return(NULL);

    fh = fdev_get_udata(stream);
    if(fh == NULL)
    {
        errno = EBADF;
        return(NULL);
    }
    return(fh);
}


/// @brief Convert FatFS file handle to POSIX fileno.
///
/// @param[in] fh: FatFS file pointer.
///
/// @return fileno on success.
/// @return -1 on error with errno set to EBADF.
MEMSPACE
int fatfs_to_fileno(FIL *fh)
{
    int i;

    FILE *stream;

    if(fh == NULL)
    {
        errno = EBADF;
        return(-1);
    }

    for(i=0;i<MAX_FILES;++i)
    {
        stream = __iob[i];
        if(stream)
        {
            if( fh == (FIL *) fdev_get_udata(stream) )
                return(i);
        }
    }
    errno = EBADF;
    return(-1);
}

#ifdef NO_STDIO
/// @brief feof reports if the stream is at EOF 
/// - man page feof (3).
///
/// @param[in] stream: POSIX stream pointer.
/// @return 1 if EOF set, 0 otherwise.
MEMSPACE
int feof(FILE *stream)
{
	if(stream->flags & __SEOF)
		return(1);
	return(0);
}
#endif

#ifdef NO_STDIO
/// @brief ferror reports if the stream has an error flag set
/// - man page ferror (3).
///
/// @param[in] stream: POSIX stream pointer.
/// @return 1 if EOF set, 0 otherwise.
MEMSPACE
int ferror(FILE *stream)
{
	if(stream->flags & __SERR)
		return(1);
	return(0);
}
#endif

#ifdef NO_STDIO
/// @brief clrerror resets stream EOF and error flags
/// - man page clrerror(3).
///
/// @param[in] stream: POSIX stream pointer.
/// @return EOF on error with errno set.
MEMSPACE
void clrerror(FILE *stream)
{
	stream->flags &= ~__SEOF;
	stream->flags &= ~__SERR;
}
#endif


/// @brief Allocate a POSIX FILE descriptor.
///
/// @return fileno on success.
/// @return -1 on failure with errno set.
MEMSPACE
int new_file_descriptor( void )
{
    int i;
    FILE *stream;
    FIL *fh;

    for(i=0;i<MAX_FILES;++i)
    {
        if(isatty(i))
            continue;
        if( __iob[i] == NULL)
        {
            stream = (FILE *) calloc(sizeof(FILE),1);
            if(stream == NULL)
            {
                errno = ENOMEM;
                return(-1);
            }
            fh = (FIL *) calloc(sizeof(FIL),1);
            if(fh == NULL)
            {
                free(stream);
                errno = ENOMEM;
                return(-1);
            }

            __iob[i]  = stream;
            fdev_set_udata(stream, (void *) fh);
            return(i);
        }
    }
    errno = ENFILE;
    return(-1);
}


#ifdef NO_STDIO
/// @brief  Assign stdin,stdout,stderr
///
/// @param[in] *put: uart putc function pointer
/// @param[in] *get: uart gutc function pointer
MEMSPACE
FILE *
fdevopen(int (*put)(char, FILE *), int (*get)(FILE *))
{
    FILE *s;

    if (put == 0 && get == 0)
        return 0;

    if ((s = calloc(1, sizeof(FILE))) == 0)
        return 0;

    s->flags = __SMALLOC;

    if (get != 0) {
        s->get = get;
        s->flags |= __SRD;
		// Only assign once
        if (stdin == 0)
            stdin = s;
    }

    if (put != 0) {
        s->put = put;
        s->flags |= __SWR;
		// NOTE:
		// STDOUT and STDERR are the same here
		// Only assign once
        if (stdout == 0) 
            stdout = s;
		if (stderr == 0)
			stderr = s;
    }

    return s;
}
#endif


/// @brief  Free POSIX fileno FILE descriptor.
///
/// @param[in] fileno: POSIX file number __iob[] index.
///
/// @return fileno on success.
/// @return -1 on failure.
MEMSPACE
int free_file_descriptor(int fileno)
{
    FILE *stream;
    FIL *fh;

    if(isatty( fileno ))
    {
        errno = EBADF;
        return(-1);
    }

	// checks if fileno out of bounds
    stream = fileno_to_stream(fileno);
    if(stream == NULL)
    {
        return(-1);
    }

    fh = fdev_get_udata(stream);

    if(fh != NULL)
    {
        free(fh);
    }

    if(stream->buf != NULL && stream->flags & __SMALLOC)
    {
        free(stream->buf);
    }

    __iob[fileno]  = NULL;
    free(stream);
    return(fileno);
}


/// @brief  Test POSIX fileno if it is a Serial Console/TTY.
///
///  - man page isatty (3).
///
/// @param[in] fileno: POSIX fileno of open file.
///
/// @return 1 if fileno is a serial TTY/Console (uart in avr-libc terms).
/// @return 0 if POSIX fileno is NOT a Serial TTY.
MEMSPACE
int isatty(int fileno)
{
/// @todo  Perhaps we should verify console functions have been added ?
    if(fileno >= 0 && fileno <= 2)
        return(1);
    return 0;
}



/// @brief POSIX perror() -  convert POSIX errno to text with user message.
///
/// - man page errno (3).
///
/// @param[in] s: User message displayed before the error message
///
/// @see sys_errlist[].
/// @return  void.
MEMSPACE
void perror(const char *s)
{
    const char *ptr = NULL;


    if(errno >=0 && errno < EBADMSG)
        ptr = sys_errlist[errno];
    else
        ptr = sys_errlist[EBADMSG];

    if(s && *s)
        printf("%s: %s\n", s, ptr);
    else
        printf("%s\n", ptr);
}


/// @brief POSIX strerror() -  convert POSIX errno to text with user message.
///
/// - man page strerror (3).
///
/// @param[in] errnum: index for sys_errlist[]
///
/// @see sys_errlist[].
/// @return  char *
MEMSPACE
char *strerror(int errnum)
{
        return( (char *)sys_errlist[errnum] );
}

/// @brief POSIX strerror_r() -  convert POSIX errno to text with user message.
///
/// - man page strerror (3).
///
/// @param[in] errnum: index for sys_errlist[]
/// @param[in] buf: user buffer for error message
/// @param[in] buflen: length of user buffer for error message
///
/// @see sys_errlist[].
/// @return  char *
MEMSPACE
char *strerror_r(int errnum, char *buf, size_t buflen)
{
		strncpy(buf, sys_errlist[errnum], buflen);
		return(buf);
}

/// @brief Convert FafFs error result to POSIX errno.
///
/// - man page errno (3).
///
/// @param[in] Result: FatFs Result code.
///
/// @return POSIX errno.
/// @return EBADMSG if no conversion possible.
MEMSPACE
int fatfs_to_errno( FRESULT Result )
{
    switch( Result )
    {
        case FR_OK:              /* FatFS (0) Succeeded */
            return (0);          /* POSIX OK */
        case FR_DISK_ERR:        /* FatFS (1) A hard error occurred in the low level disk I/O layer */
            return (EIO);        /* POSIX Input/output error (POSIX.1) */

        case FR_INT_ERR:         /* FatFS (2) Assertion failed */
            return (EPERM);      /* POSIX Operation not permitted (POSIX.1) */

        case FR_NOT_READY:       /* FatFS (3) The physical drive cannot work */
            return (EBUSY);      /* POSIX Device or resource busy (POSIX.1) */

        case FR_NO_FILE:         /* FatFS (4) Could not find the file */
            return (ENOENT);     /* POSIX No such file or directory (POSIX.1) */

        case FR_NO_PATH:         /* FatFS (5) Could not find the path */
            return (ENOENT);     /* POSIX No such file or directory (POSIX.1) */

        case FR_INVALID_NAME:    /* FatFS (6) The path name format is invalid */
            return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

        case FR_DENIED:          /* FatFS (7) Access denied due to prohibited access or directory full */
            return (EACCES);     /* POSIX Permission denied (POSIX.1) */
        case FR_EXIST:           /* FatFS (8) Access denied due to prohibited access */
            return (EACCES);     /* POSIX Permission denied (POSIX.1) */

        case FR_INVALID_OBJECT:  /* FatFS (9) The file/directory object is invalid */
            return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

        case FR_WRITE_PROTECTED: /* FatFS (10) The physical drive is write protected */
            return(EROFS);       /* POSIX Read-only filesystem (POSIX.1) */

        case FR_INVALID_DRIVE:   /* FatFS (11) The logical drive number is invalid */
            return(ENXIO);       /* POSIX No such device or address (POSIX.1) */

        case FR_NOT_ENABLED:     /* FatFS (12) The volume has no work area */
            return (ENOSPC);     /* POSIX No space left on device (POSIX.1) */

        case FR_NO_FILESYSTEM:   /* FatFS (13) There is no valid FAT volume */
            return(ENXIO);       /* POSIX No such device or address (POSIX.1) */

        case FR_MKFS_ABORTED:    /* FatFS (14) The f_mkfs() aborted due to any parameter error */
            return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

        case FR_TIMEOUT:         /* FatFS (15) Could not get a grant to access the volume within defined period */
            return (EBUSY);      /* POSIX Device or resource busy (POSIX.1) */

        case FR_LOCKED:          /* FatFS (16) The operation is rejected according to the file sharing policy */
            return (EBUSY);		 /* POSIX Device or resource busy (POSIX.1) */


        case FR_NOT_ENOUGH_CORE: /* FatFS (17) LFN working buffer could not be allocated */
            return (ENOMEM);     /* POSIX Not enough space (POSIX.1) */

        case FR_TOO_MANY_OPEN_FILES:/* FatFS (18) Number of open files > _FS_SHARE */
            return (EMFILE);     /* POSIX Too many open files (POSIX.1) */

        case FR_INVALID_PARAMETER:/* FatFS (19) Given parameter is invalid */
            return (EINVAL);     /* POSIX Invalid argument (POSIX.1) */

    }
	return (EBADMSG);            /* POSIX Bad message (POSIX.1) */
}


/// @brief Convert POSIX fopen mode to POSIX open mode flags.
///
/// - man page fopen (3).
/// - man page open (2).
/// - Valid modes.
/// - Read
///  - "r", "rb"
/// - Read and Write
///  - "r+", "r+b", "rb+"
/// - Write
///  - "w", "wb"
/// - Write and Read.
///  - "w+", "w+b", "wb+"
///  - "w+" implies write/read access.
/// - Append
///  - "a", "ab"
/// - Append and Read
///  - "a+", "a+b", "ab+"
/// - Note: ORDER IS IMPORTANT! so w+ is NOT the same as r+.
/// - ALWAYS do a fflush or fseek between rear write operations if + is used..
///
/// @param[in] mode: POSIX file mode string.
///
/// @return open mode flags.
/// @return -1 on error.
/// @warning read and write BOTH share the same stream buffer and buffer index pointers.
MEMSPACE
int posix_fopen_modes_to_open(const char *mode)
{
    int flag = 0;

    if(modecmp(mode,"r") || modecmp(mode,"rb"))
    {
        flag = O_RDONLY;
        return(flag);
    }
    if(modecmp(mode,"r+") || modecmp(mode, "r+b" ) || modecmp(mode, "rb+" ))
    {
        flag = O_RDWR | O_TRUNC;
        return(flag);
    }
    if(modecmp(mode,"w") || modecmp(mode,"wb"))
    {
        flag = O_WRONLY | O_CREAT | O_TRUNC;
        return(flag);
    }
    if(modecmp(mode,"w+") || modecmp(mode, "w+b" ) || modecmp(mode, "wb+" ))
    {
        flag = O_RDWR | O_CREAT | O_TRUNC;
        return(flag);
    }
    if(modecmp(mode,"a") || modecmp(mode,"ab"))
    {
        flag = O_WRONLY | O_CREAT | O_APPEND;
        return(flag);
    }
    if(modecmp(mode,"a+") || modecmp(mode, "a+b" ) || modecmp(mode, "ab+" ))
    {
        flag = O_RDWR | O_CREAT | O_APPEND;
        return(-1);
    }
    return(-1);                                   // nvalid mode
}

/// ==================================================================



/// @brief Private FatFs function called by fgetc() to get a byte from file stream
/// open() assigns stream->get = fatfs_getc() 
///
/// - man page fgetc (3).
/// - Notes: fgetc does all tests prior to caling us, including ungetc.
///
/// @param[in] stream: POSIX stream pointer.
///
/// @return character.
/// @return EOF on error with errno set.
MEMSPACE
static int  fatfs_getc(FILE *stream)
{
    FIL *fh;
    UINT size;
    int res;
    uint8_t c;

    errno = 0;

    if(stream == NULL)
    {
        errno = EBADF;                            // Bad File Number
        return(EOF);
    }

    fh = (FIL *) fdev_get_udata(stream);
    if(fh == NULL)
    {
        errno = EBADF;                            // Bad File Number
        return(EOF);
    }

    res = f_read(fh, &c, 1, (UINT *) &size);
    if( res != FR_OK || size != 1)
    {
        errno = fatfs_to_errno(res);
        stream->flags |= __SEOF;
        return(EOF);
    }
    return(c & 0xff);
}

/// @brief Private FatFs function called by fputc() to put a byte from file stream
/// open() assigns stream->put = fatfs_putc() 
///
/// - man page fputc (3).
/// - Notes: fputc does all tests prior to caling us.
///
/// @param[in] c: character.
/// @param[in] stream: POSIX stream pointer.
///
/// @return character 
/// @return EOF on error with errno set.
MEMSPACE
static 
int fatfs_putc(char c, FILE *stream)
{
    int res;
    FIL *fh;
    UINT size;

    errno = 0;
    if(stream == NULL)
    {
        errno = EBADF;                            // Bad File Number
        return(EOF);
    }

    fh = (FIL *) fdev_get_udata(stream);
    if(fh == NULL)
    {
        errno = EBADF;                            // Bad File Number
        return(EOF);
    }

    res = f_write(fh, &c, 1, (UINT *)  &size);
    if( res != FR_OK || size != 1)
    {
        errno = fatfs_to_errno(res);
        stream->flags |= __SEOF;
        return(EOF);
    }
    return(c);
}

/// ==================================================================
#ifdef NO_STDIO
/// @brief Get byte from a TTY device or FatFs file stream
/// open() or fopen() sets stream->get = fatfs_getc() for FatFs functions
/// fdevopen()        sets stream->get for TTY devices
///
/// - man page fgetc (3).
///
/// @param[in] stream: POSIX stream pointer.
///
/// @return character.
/// @return EOF on error with errno set.
MEMSPACE
int
fgetc(FILE *stream)
{
    int c;

    if ((stream->flags & __SRD) == 0)
        return EOF;

    if ((stream->flags & __SUNGET) != 0) {
        stream->flags &= ~__SUNGET;
        stream->len++;
        return stream->unget;
    }

    if (stream->flags & __SSTR) {
        c = *stream->buf;
        if (c == '\0') {
            stream->flags |= __SEOF;
            return EOF;
        } else {
            stream->buf++;
        }
    } else {
		if(!stream->get)
		{
			printf("fgetc stream->get NULL\n");
			return(EOF);
		}
        c = stream->get(stream);
        if (c < 0) {
            /* if != _FDEV_ERR, assume it's _FDEV_EOF */
            stream->flags |= (c == _FDEV_ERR)? __SERR: __SEOF;
            return EOF;
        }
    }

    stream->len++;
    return (unsigned char)c;
}
#endif

#ifdef NO_STDIO
/// @brief Put a byte to TTY device or FatFs file stream
/// open() or fopen() sets stream->put = fatfs_outc() for FatFs functions
/// fdevopen()        sets stream->put get for TTY devices
///
/// - man page fputc (3).
///
/// @param[in] stream: POSIX stream pointer.
///
/// @return character.
MEMSPACE
int
fputc(int c, FILE *stream)
{
    errno = 0;

    if(stream == NULL)
    {
        errno = EBADF;                            // Bad File Number
        return(EOF);
    }

	if(stream != stdout && stream != stderr)
	{
        return(fatfs_putc(c,stream));
    }

	// TTY outputs

    if ((stream->flags & __SWR) == 0)
        return EOF;

    if (stream->flags & __SSTR) {
        if (stream->len < stream->size)
            *stream->buf++ = c;
        stream->len++;
        return c;
    } else {
		if(!stream->put)
		{
			printf("fputc stream->put NULL\n");
			return(EOF);
		}
        if (stream->put(c, stream) == 0) {
            stream->len++;
            return c;
        } else
            return EOF;
    }
}
#endif


/// ==================================================================


/// @brief POSIX Open a file with integer mode flags.
///
/// - man page open (2).
///
/// @param[in] pathname: filename string.
/// @param[in] flags: POSIX open modes.
///
/// @return fileno on success.
/// @return -1 on error with errno set.
MEMSPACE
int open(const char *pathname, int flags)
{
    int fileno;
    int fatfs_modes;
    FILE *stream;
    FIL *fh;
    int res;

    errno = 0;

// FIXME Assume that mmc_init was already called 
#if 0
// Checks Disk status
    res = mmc_init(0);

    if(res != RES_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
#endif

    if((flags & O_ACCMODE) == O_RDWR)
        fatfs_modes = FA_READ | FA_WRITE;
    else if((flags & O_ACCMODE) == O_RDONLY)
        fatfs_modes = FA_READ;
    else
        fatfs_modes = FA_WRITE;

    if(flags & O_CREAT)
    {
        if(flags & O_TRUNC)
            fatfs_modes |= FA_CREATE_ALWAYS;
        else
            fatfs_modes |= FA_OPEN_ALWAYS;
    }

    fileno = new_file_descriptor();

	// checks if fileno out of bounds
    stream = fileno_to_stream(fileno);
    if(stream == NULL)
    {
        free_file_descriptor(fileno);
        return(-1);
    }

	// fileno_to_fatfs checks for fileno out of bounds
    fh = fileno_to_fatfs(fileno);
    if(fh == NULL)
    {
        free_file_descriptor(fileno);
        errno = EBADF;
        return(-1);
    }
    res = f_open(fh, pathname, (BYTE) (fatfs_modes & 0xff));
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        free_file_descriptor(fileno);
        return(-1);
    }
	if(flags & O_APPEND)
    {
///  Seek to end of the file
        res = f_lseek(fh, fh->fsize);
        if (res != FR_OK)
        {
            errno = fatfs_to_errno(res);
            f_close(fh);
            free_file_descriptor(fileno);
            return(-1);
        }
    }

    if((flags & O_ACCMODE) == O_RDWR)
    {
        stream->put = fatfs_putc;
        stream->get = fatfs_getc;
        stream->flags = _FDEV_SETUP_RW;
    }
    else if((flags & O_ACCMODE) == O_RDONLY)
    {
        stream->put = NULL;
        stream->get = fatfs_getc;
        stream->flags = _FDEV_SETUP_READ;
    }
    else
    {
        stream->put = fatfs_putc;
        stream->get = NULL;
        stream->flags = _FDEV_SETUP_WRITE;
    }

    return(fileno);
}


///@brief POSIX Open a file with path name and ascii file mode string.
///
/// - man page fopen(3).
///
/// @param[in] path: filename string.
/// @param[in] mode: POSIX open mode strings.
///
/// @return stream * on success.
/// @return NULL on error with errno set.
MEMSPACE
FILE *fopen(const char *path, const char *mode)
{
    int flags = posix_fopen_modes_to_open(mode);
    int fileno = open(path, flags);

	// checks if fileno out of bounds
    return( fileno_to_stream(fileno) );
}


/// @brief POSIX Close a file with fileno handel.
///
/// - man page close (2).
///
/// @param[in] fileno: fileno of file.
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
MEMSPACE
int close(int fileno)
{
    FILE *stream;
    FIL *fh;
    int res;

    errno = 0;

	// checks if fileno out of bounds
    stream = fileno_to_stream(fileno);
    if(stream == NULL)
    {
        return(-1);
    }

	// fileno_to_fatfs checks for fileno out of bounds
    fh = fileno_to_fatfs(fileno);
    if(fh == NULL)
    {
        return(-1);
    }
    res = f_close(fh);
    free_file_descriptor(fileno);
    if (res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}


/// @brief POSIX Sync pending file changes and metadata for specified fileno.
///
/// - man page syncfs (2).
///
/// @param[in] fd: POSIX fileno to sync.
/// @return 0.
/// @return -1 on error witrh errno set.
MEMSPACE
int syncfs(int fd)
{
    FIL *fh;
    FRESULT res;

    errno = 0;

    if(isatty(fd))
    {
        errno = EBADF;
        return(-1);
    }

	// fileno_to_fatfs checks for fd out of bounds
    fh = fileno_to_fatfs(fd);
    if(fh == NULL)
    {
        errno = EBADF;
        return(-1);
    }

    res  = f_sync ( fh );
    if (res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}


/// @brief POSIX Sync all pending file changes and metadata on ALL files.
///
/// - man page sync (2).
///
/// @return  void.
MEMSPACE
void sync(void)
{
    FIL *fh;
    int i;

    for(i=0;i<MAX_FILES;++i)
    {
        if(isatty(i))
            continue;

		// fileno_to_fatfs checks for i out of bounds
        fh = fileno_to_fatfs(i);
        if(fh == NULL)
            continue;

        (void ) syncfs(i);
    }
}


/// @brief POSIX close a file stream.
///
/// - man page flose (3).
///
/// @param[in] stream: POSIX stream pointer.

/// @return  0 on sucess.
/// @return  -1 on error witrh errno set.
MEMSPACE
int fclose(FILE *stream)
{
    int fn = fileno(stream);
	if(fn < 0)
		return(EOF);

    return( close(fn) );
}


/// @brief POSIX Write count bytes from *buf to fileno fd.
///
/// - man page write (2).
///
/// @param[in] fd: POSIX fileno.
/// @param[in] buf: buffer.
/// @param[in] count: number of bytes to write.
/// @return count on sucess.
/// @return -1 on error with errno set.
MEMSPACE
ssize_t write(int fd, const void *buf, size_t count)
{
    UINT size;
	UINT bytes = count;
    FRESULT res;
    FIL *fh;

    errno = 0;
#if 0
	// FIXME TTY write
	// TODO check for fd out of bounds fileno_to_fatfs does this
	if(__iob[fd] == stdin || __iob[fd] == stderr)
	{
	}
#endif

	// fileno_to_fatfs checks for fd out of bounds
    fh = fileno_to_fatfs(fd);
    if ( fh == NULL )
    {
        errno = EBADF;
        return(-1);
    }

    res = f_write(fh, buf, bytes, &size);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return ((ssize_t) size);
}


/// @brief POSIX write nmemb elements from buf, size bytes each, to the stream fd.
///
/// - man page write (2).
///
/// @param[in] ptr: buffer.
/// @param[in] nmemb: number of items to write.
/// @param[in] size: size of each item in bytes.
/// @param[in] stream: POSIX file stream.
///
/// @return count written on sucess.
/// @return 0 or < size on error with errno set.
MEMSPACE
size_t fwrite(const void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t count = size * nmemb;
    int fn = fileno(stream);
	ssize_t ret;
	
	// write () checks for fn out of bounds
	ret =  write(fn, ptr, count);

	if(ret < 0)
		return(0);

	return((size_t) ret);
}


/// @brief POSIX read count bytes from *buf to fileno fd.
///
/// - man page read (2).
///
/// @param[in] fd: POSIX fileno.
/// @param[in] buf: buffer.
/// @param[in] count: number of bytes to write.
///
/// @return count on sucess.
/// @return -1 on error with errno set.
MEMSPACE
ssize_t read(int fd, const void *buf, size_t count)
{
    UINT size;
	UINT bytes = count;
    int res;
    FIL *fh;

	*(char *) buf = 0;

    errno = 0;
#if 0
	// FIXME TTY read
	// TODO check for fd out of bounds fileno_to_fatfs does this
	if(__iob[fd] == stdin)
	{
	}
#endif

	// fileno_to_fatfs checks for fd out of bounds
    fh = fileno_to_fatfs(fd);
    if ( fh == NULL )
    {
        errno = EBADF;
        return(-1);
    }

    res = f_read(fh, (void *) buf, bytes, &size);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return ((ssize_t) size);
}


/// @brief POSIX read nmemb elements from buf, size bytes each, to the stream fd.
///
/// - man page fread (3).
///
/// @param[in] ptr: buffer.
/// @param[in] nmemb: number of items to read.
/// @param[in] size: size of each item in bytes.
/// @param[in] stream: POSIX file stream.
///
/// @return count on sucess.
/// @return 0 or < size on error with errno set.
MEMSPACE
size_t fread(void *ptr, size_t size, size_t nmemb, FILE *stream)
{
    size_t count = size * nmemb;
    int fn = fileno(stream);
	ssize_t ret;

	// read() checks for fn out of bounds
	ret = read(fn, ptr, count);
	if(ret < 0)
		return(0);

	return((size_t) ret);
}


/// @brief POSIX seek to file position.
///
/// - man page lseek (2).
///
/// @param[in] fileno: POSIX fileno of open file.
/// @param[in] position: offset to seek to.
/// @param[in] whence
///  - SEEK_SET The offset is set to offset bytes.
///  - SEEK_CUR The offset is set to its current location plus offset bytes.
///  - SEEK_END The offset is set to the size of the file plus offset bytes.
///
/// @return file position on sucess.
/// @return -1 on error.
MEMSPACE
size_t lseek(int fileno, size_t position, int whence)
{
    FRESULT res;
    FIL *fh;
    errno = 0;

	// fileno_to_fatfs checks for fd out of bounds
    fh = fileno_to_fatfs(fileno);
    if(fh == NULL)
    {
        errno = EMFILE;
        return(-1);
    }

    if(whence == SEEK_END)
        position += fh->fsize;
    else if(whence==SEEK_CUR)
        position += fh->fptr;

    res = f_lseek(fh, position);
    if(res)
    {
        errno = fatfs_to_errno(res);
        return -1;
    }
    return (fh->fptr);
}


/// @brief POSIX seek to file possition.
///
/// - man page fseek (3).
///
/// @param[in] stream: POSIX file stream.
/// @param[in] offset: offset to seek to.
/// @param[in] whence:
///  - SEEK_SET The offset is set to offset bytes.
///  - SEEK_CUR The offset is set to its current location plus offset bytes.
///  - SEEK_END The offset is set to the size of the file plus offset bytes.
///
/// @return file position on sucess.
/// @return -1 on error.
MEMSPACE
int fseek(FILE *stream, long offset, int whence)
{
    long ret;

    int fn = fileno(stream);
	if(fn < 0)
		return(-1);

    ret  = lseek(fn, offset, whence);

    if(ret == -1)
        return(-1);

    return(0);
}


/// @brief POSIX file position of open stream.
///
/// - man page fteel (3).
///
/// @param[in] stream: POSIX file stream.
///
/// @return file position on sucess.
/// @return -1 on error with errno set.
MEMSPACE
long ftell(FILE *stream)
{
    errno = 0;

    int fn = fileno(stream);
	// fileno_to_fatfs checks for fd out of bounds
    FIL *fh = fileno_to_fatfs(fn);
    if ( fh == NULL )
    {
        errno = EBADF;
        return(-1);
    }

    return( fh->fptr );
}


/// @brief POSIX  rewind file to the beginning.
///
/// - man page rewind (3).
///
/// @param[in] stream: POSIX file stream.
///
/// @return  void.
MEMSPACE
void rewind( FILE *stream)
{
    fseek(stream, 0L, SEEK_SET);
}


/// @brief POSIX get position of file stream.
///
/// - man page fgetpos (3).
///
/// @param[in] stream: POSIX file stream.
/// @param[in] pos: position pointer for return.
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
MEMSPACE
int fgetpos(FILE *stream, size_t *pos)
{
    long offset = ftell(stream);
    *pos = offset;
    if(offset == -1)
        return(-1);
    return( 0 );
}


/// @brief POSIX set position of file stream.
///
/// - man page fsetpos (3).
///
/// @param[in] stream: POSIX file stream.
/// @param[in] pos: position pointer.
///
/// @return 0 with *pos set to position on sucess.
/// @return -1 on error with errno set.
MEMSPACE
int fsetpos(FILE *stream, size_t *pos)
{
    return (fseek(stream, (size_t) *pos, SEEK_SET) );
}


/// @brief POSIX delete a file.
///
/// - man page unlink (2).
///
/// @param[in] pathname: filename to delete.
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
MEMSPACE
int unlink(const char *pathname)
{
    errno = 0;
    int res = f_unlink(pathname);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}


/// @brief POSIX delete a directory.
///
/// - man page rmdir (2).
///
/// @param[in] pathname: directory to delete.
///
/// @return 0 on sucess.
/// @return -1 on error with errno set.
MEMSPACE
int rmdir(const char *pathname)
{
    errno = 0;
    int res = f_unlink(pathname);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }
    return(0);
}



/// @brief Convert FatFs file date and time to POSIX epoch seconds.
///
/// - man page timegm (3).
///
/// @param[in] date: FatFs date.
/// @param[in] time: FatFs time.
///
/// @see timegm()
///
/// @return epoch seconds 
MEMSPACE
time_t fat_time_to_unix(uint16_t date, uint16_t time)
{
    struct tm tp;
    time_t unix;

    memset(&tp, 0, sizeof(struct tm));

    tp.tm_sec = (time << 1) & 0x3e;               // 2 second resolution
    tp.tm_min = ((time >> 5) & 0x3f);
    tp.tm_hour = ((time >> 11) & 0x1f);
    tp.tm_mday = (date & 0x1f);
    tp.tm_mon = ((date >> 5) & 0x0f) - 1;
    tp.tm_year = ((date >> 9) & 0x7f) + 80;
    unix = timegm( &tp );
    return( unix );
}


/// @brief POSIX truncate open file to length.
///
/// - man page ftruncate (3).
///
/// @param[in] fd: open file number.
/// @param[in] length: length to truncate to.
///
/// @return 0 on success.
/// @return -1 on fail.
MEMSPACE
int ftruncate(int fd, off_t length)
{
    errno = 0;
    FIL *fh;
    FRESULT rc;

	// fileno_to_fatfs checks for fd out of bounds
    fh = fileno_to_fatfs(fd);
    if(fh == NULL)
    {
        return(-1);
    }
    rc = f_lseek(fh, length);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    rc = f_truncate(fh);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    return(0);
}


/// @brief POSIX truncate named file to length.
///
/// - man page truncate (2).
///
/// @param[in] path: file name to truncate.
/// @param[in] length: length to truncate to.
///
/// @return 0 on sucess.
/// @return -1 n fail.
MEMSPACE
int truncate(const char *path, off_t length)
{
    errno = 0;
    FIL fh;
    FRESULT rc;

    rc = f_open(&fh , path, FA_OPEN_EXISTING | FA_READ | FA_WRITE);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    rc = f_lseek(&fh, length);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    rc = f_truncate(&fh);
    if (rc != FR_OK)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    return(0);
}


#if 0
/// @brief POSIX fstat of open file.
///
/// - man page (2).
///
/// @param[in] fd: POSIX fileno of open file.
/// @param[in] buf: struct stat buffer to return results in.
///
/// @return 0 on success.
/// @return -1 on error with errno set.
///
/// @todo needs fileno to filename lookup in order to work.
/// - We may be able to work out the directory pointer from the FatFS data?
MEMSPACE
int fstat(int fd, struct stat *buf)
{
    FIL *fh;
    FRESULT rc;

	// fileno_to_fatfs checks for fd out of bounds
    fh = fileno_to_fatfs(fileno);
    if(fh == NULL)
    {
        return(-1);
    }
    rc = f_lseek(fh, length);
#if _USE_LFN
#endif
}
#endif

/// @brief POSIX stat - get file status of named file.
///
/// - man page (2).
///
/// @param[in] name: file name.
/// @param[in] buf: struct stat buffer to return results in.
///
/// @return 0 on success.
/// @return -1 on error with errno set.
MEMSPACE
int stat(char *name, struct stat *buf)
{
    FILINFO *info = fatfs_alloc_finfo(0);
    int res;
    time_t epoch;
    uint16_t mode;
    errno = 0;

    if(info == NULL)
    {
        errno = ENOMEM;
        return(-1);
    }

    res = f_stat(name, info);
    if(res != FR_OK)
    {
        errno = fatfs_to_errno(res);
        return(-1);
    }


    buf->st_size = info->fsize;
    epoch = fat_time_to_unix(info->fdate, info->ftime);
    buf->st_atime = epoch;                        // Access time
    buf->st_mtime = epoch;                        // Modification time
    buf->st_ctime = epoch;                        // Creation time

    mode = (FATFS_R | FATFS_X);
    if( !(info->fattrib & AM_RDO))
        mode |= (FATFS_W);                        // enable write if NOT read only

    if(info->fattrib & AM_SYS)
    {
        buf->st_uid= 0;
        buf->st_gid= 0;
    }
    {
        buf->st_uid=1000;
        buf->st_gid=1000;
    }

    if(info->fattrib & AM_DIR)
        mode |= S_IFDIR;
    else
        mode |= S_IFREG;
    buf->st_mode = mode;

    return(0);
}


/// @brief Display Ascii formatted time from timev seconds 
///
/// - Assumes no timezone offset.
/// - man page ctime (3).
///
/// @param[in] timev: epoch time in seconds
/// @return  ascii string pointer of POSIX ctime()
/// @see ctime()
MEMSPACE
char *mctime(time_t timev)
{
    errno = 0;
    char *ptr = (char *)ctime_gm(&timev);
    int len;
    len = strlen(ptr);
    if(len && ptr[len-1] == '\n')
        ptr[len-1] = 0;
    return(ptr);
}


/// @brief Display struct stat, from POSIX stat(0 or fstat(), in ASCII.
///
/// @param[in] sp: struct stat pointer.
///
/// @return  void.
MEMSPACE
void dump_stat(struct stat *sp)
{
    mode_t mode = sp->st_mode;

    printf("\tSize:  %lu\n", (uint32_t)sp->st_size);

    printf("\tType:  ");
    if(S_ISDIR(mode))
        printf("DIR\n");
    else if(S_ISREG(mode))
        printf("File\n");
    else
        printf("Unknown\n");


    printf("\tMode:  %lo\n", (uint32_t)sp->st_mode);
    printf("\tUID:   %lu\n", (uint32_t)sp->st_uid);
    printf("\tGID:   %lu\n", (uint32_t)sp->st_gid);
    printf("\tatime: %s\n",mctime((time_t)sp->st_atime));
    printf("\tmtime: %s\n",mctime((time_t)sp->st_mtime));
    printf("\tctime: %s\n",mctime((time_t)sp->st_ctime));
}


/// @brief POSIX rename a file by name.
///
/// - man page (2).
///
/// @param[in] oldpath: original name.
/// @param[in] newpath: new name.
///
/// @return 0 on success.
/// @return -1 on error with errno set.
MEMSPACE
int rename(const char *oldpath, const char *newpath)
{
/* Rename an object */
    int rc;
    errno = 0;
    rc = f_rename(oldpath, newpath);
    if(rc)
    {
        errno = fatfs_to_errno(rc);
        return(-1);
    }
    return(0);
}



/// @brief Return the index of the last '/' character.
///
/// - Example:
/// @code
///  dir[0] = 0;
///  ret = dirname(path)
///  if(ret)
///   strncpy(dir,path,ret);
/// @endcode
///
/// @param[in] str: string to examine.
/// @return  0 if no directory part.
/// @return index of last '/' character.
///
MEMSPACE
int dirname(char *str)
{
    int end = 0;
    int ind = 0;

    if(!str)
        return(0);

    while(*str)
    {
        if(*str == '/')
            end = ind;
        ++str;
        ++ind;
    }
    return(end);
}


/// @brief POSIX Basename of filename.
///
/// - man page (3).
///
/// @param[in] str: string to find basename in.
///
/// @return pointer to basename of string.
MEMSPACE
char *basename(char *str)
{
    char *base = str;
    if(!str)
        return("");
    while(*str)
    {
        if(*str++ == '/')
            base = str;
    }
    return(base);
}


/// @brief File extention of a file name.
///
/// @param[in] str: string to find extension in.
///
/// @return  pointer to basename extension.
MEMSPACE
char *baseext(char *str)
{
    char *ext = "";

    while(*str)
    {
        if(*str++ == '.')
            ext = str;
    }
    return(ext);
}

