/**
 @file buffer.h

 @brief character read buffering wrappers for FatFS
 WHY? Character at a time operation is in FatFS are VERY slow

 @par Copyright &copy; 2017 Mike Gore, GPL License
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
#include "posix.h"
#include "buffer.h"

///@brief FatFS does not have a f_fgetc() function
/// Using f_read() of just 1 byte is VERY VERY SLOW

	
///@brief buffered read open
///@param[in] name: file name
///@param[in] buf:  buffer to use when reading
///@param[in] size: buffer size
///@return buffer_t pointer, or NULL on error
buffer_t *buffer_read_open(char *name, uint8_t *buf, int size)
{
	static buffer_t _buff;
	buffer_t *p = (buffer_t *) &_buff;

	memset(p,0,(int) sizeof(buffer_t));

	p->buf  = buf;

	///@brief Read and process into image file
	p->fp = fopen(name,"r");
	if(p->fp == NULL)
	{
		printf("buffer_read_open: Can not open:[%s]\n", name);
		buffer_read_close(p);
		return(0);
	}

	p->size = size;
	p->ind = 0;
	p->len = 0;
	p->ungetf = 0;
	p->ungetc = 0;
	return(p);
}


///@brief buffered read close
/// FatFS does not have a f_fgetc() and using f_read() of just 1 byte is VERY SLOW
///@param[in] p: buffer structure ppointer
///@return void
void buffer_read_close(buffer_t *p)
{
	if(p->fp)
		fclose(p->fp);
	p->size = 0;
	p->len = 0;
	p->ind = 0;
	p->ungetf = 0;
	p->ungetc = 0;
}

///@brief buffered ungetc
///@param[in] p: buffer structure ppointer
///@param[in] c: character to unget
///@return void
void buffer_ungetc(buffer_t *p, int c)
{
	p->ungetf = 1;
	p->ungetc = c;
}

	
///@brief buffered getc
///@param[in] p: buffer structure ppointer
///@return character or EOF 
int buffer_getc(buffer_t *p)
{
	int size;
	int c;

	if(p->ungetf)
	{
		p->ungetf = 0;
		return(p->ungetc);
	}
	// Read a new block
	if(p->len < 1)
	{
		p->ind = 0;
		size = fread(p->buf, 1, p->size, p->fp);
		if(size <= 0)
		{
			p->len = 0;
			return(EOF);
		}
		p->len = size;
	}
	c = p->buf[p->ind];
	p->len--;
	p->ind++;
	return( c );
}


///@brief buffered getc
///@param[out] str: string
///@param[int] size: maximum string size
///@param[in] p: buffer structure ppointer
///@return string or NULL on EOF
uint8_t *buffer_gets(uint8_t *str, int size, buffer_t *p)
{
	int c;
	int next;
	int ind = 0;

	// leave room for EOS
	while(ind < size)
	{
		c = buffer_getc(p);
        if(c == EOF)
		{
			if(ind == 0)
			{
				str[ind] = 0;
				return(NULL);
			}
			break;
		}
		// CR NL is end of line
		if(c == '\r')
		{
			next = buffer_getc(p);
			if(next == '\n')
				break;
			buffer_ungetc(p,next);
			break;
		}
		// NL is end of line
		if(c == '\n')
			break;
        str[ind++] = c;
    }
    str[ind] = 0;
    return(str);
}
