#include <stdio.h>
#include <stdlib.h>



/**
 @file     testflash.c
 @date     1 Nov 2016 
 @brief    Create or read a flash test pattern file - used for testing esp8266 flash

 @par Copyright &copy; 2015-2016 Mike Gore, GPL License
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

/**
 @brief    Create or read a flash test pattern file - used for testing esp8266 flash
  Depends on esptool from https://github.com/themadinventor/esptool
  Example - test 1 megabyte of flash - we write and verify a pattern
	ESPTOOL=/usr/local/bin/esptool.py
	ESPPORT=/dev/ttyUSB0
    gcc testflash.c -o testflash
    -mkdir tmp
    @echo testing first megabyte
    @echo
    @echo Create megabyte size test file
    ./testflash -s 0x100000 -w tmp/test1w.bin
    @echo Write file to ESP8266
    $(ESPTOOL) -p $(ESPPORT) -b $(BAUD) write_flash \
        $(flashimageoptions) \
        0x000000 tmp/test1w.bin
    @echo read flash back from ESP8266
    $(ESPTOOL) -p $(ESPPORT) -b $(BAUD) read_flash \
        0x000000 0x100000 tmp/test1r.bin
    @echo Verify data read back matches what we wrote
    ./testflash -s 0x100000 -r tmp/test1r.bin
*/

int main(int argc, char *argv[])
{
	int i;
	char *p;
	char *name;
	int wf = 0;
	int rf = 0;
	int size = 0;
	int count = 0;
	int errors = 0;
	FILE *FP;
	unsigned int *buffer;

	for(i=1;i<argc;++i)
	{
		p = argv[i];
		if(p == NULL)
			break;
		if(*p == '-')
		{
			++p;
			if(*p == 'w')
			{
				name = argv[++i];
				wf = 1;
				continue;
			}
			if(*p == 'r')
			{
				name = argv[++i];
				rf = 1;
				continue;
			}
			if(*p == 's')
			{
				++p;
				if(*p)
					size = strtol(p, NULL,16);
				else
					size = strtol(argv[++i], NULL,16);
				continue;
			}
		}
	}

	if(!size || (!rf && !wf))
	{
		fprintf(stderr,"Usage: %s [-w filename ]|[-r filename] -s size\n", argv[0]);
		exit(1);
	}

	buffer = (unsigned int *) calloc(size,1);
	if(!buffer)
	{
		fprintf(stderr,"Can not alocate %d bytes\n", size);
		exit(1);
	}

	if(wf)
	{
		FP = fopen(name,"w");
		if(!FP)
		{
			fprintf(stderr,"Can not open %s for writting\n", name);
			free((void *) buffer);
			exit(1);
		}
		for(i=0;i<size/4;++i)
		{
			buffer[i] = i;
		}
		count = fwrite(buffer, 1, size,FP);
		fclose(FP);
		free((void *) buffer);
		if(count != size)
		{
			fprintf(stderr,"wrote:%08x bytes, expected:%08x bytes\n", count, size);
		}
	}
	if(rf)
	{
		FP = fopen(name,"r");
		if(!FP)
		{
			fprintf(stderr,"Can not open %s for writting\n", name);
			free((void *) buffer);
			exit(1);
		}
		count = fread(buffer, 1, size,FP);
		fclose(FP);
		if(count != size)
		{
			fprintf(stderr,"read:%08x bytes, expected:%08x bytes\n", count, size);
			free((void *) buffer);
			exit(1);
		}
		for(i=0;i<size/4;++i)
		{
			if(buffer[i] != i) 
			{
				fprintf(stderr,"addr:%08x, expected:%08x, got:%08x\n",
					i * 4, i, buffer[i]);
				++errors;
			}
		}
		free((void *) buffer);
	}
	if(errors)
		printf("%d errors\n",errors);
	else
		printf("No errors\n");
	return(errors);
}
