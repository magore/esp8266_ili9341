// =============================================
// Copyright © 2003 - 2016 Mike Gore, Waterloo, ON N2L 5N4, Canada
//
// Permission is hereby granted to use this Software for any purpose
// including combining with commercial products, creating derivative
// works, and redistribution of source or binary code, without
// limitation or consideration. Any redistributed copies of this
// Software must include the above Copyright Notice.
//
// THIS SOFTWARE IS PROVIDED "AS IS". THE AUTHOR MAKES NO
// WARRANTIES REGARDING THIS SOFTWARE, EXPRESS OR IMPLIED, AS TO ITS
// SUITABILITY OR FITNESS FOR A PARTICULAR PURPOSE.
// =============================================
// History
// Written:  2 April 2003	Mike Gore <magore@icr1.uwater.loo.ca>
// April 2 - April 8th extesive edits
// April 8 Alpha Release
// 12 Oct 2016
// =============================================
// =============================================
// Note:!!!!
// Use TAB STOPS to 4 or this document will look wrong
// =============================================

#include "user_config.h"

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "mathio.h"

#ifdef SMALL_SSCANF

// =============================================
//
//
// WRITTEN by MIKE GORE JUN 1999
//
// SCANF without multiplies!
// Work in progress

int sscanf(const char *strp, const char *fmt, ...)
{
	va_list	ap;
	int SIGN = 0;
	uint8_t SIZE;
	int args = 0; // Arguments
	uint8_t width;
	uint8_t base;
	uint8_t	shift;
	uint8_t ch;
	uint8_t spec;
	unsigned long num;

	va_start(ap,fmt);

	while ( (spec = *fmt++) )
	{


// FIXME match all non format spec components with input string HERE
//	- Of course the format spec includes the width spec, etc
//  - For now we discard any non % qualified format characters 
//    up to the % character and then the rest of the code eats whats 
//	  left in processing the spec later on. In other words we KNOW that 
//	  all characters up to a % are unused in the spec itself but really 
//    should be matched with the input to be correct - othersise could cause 
//    parse errors - if present. For now we skip spaces and commas
//	  to avoid the most common issues
//	 

//
// Skip non format characters - for now
		if (spec != '%') 
		{
			if(spec == *strp)
				++strp;
			continue;
		}

		// we had a '%'
		spec = *fmt;


// Sync up input string to current argument we want in format spec
// Skip white space and commas in input string - for now KLUDGE
//
		while ( (ch = *strp) ) {
			if(ch == '\t' || ch == ' ' || ch == ',') {
				++strp;
				continue;
			}
			break;
		}

		width = 0;		// Init width

		while( (spec = *fmt) ) {	// spec and fmt point past %
			if(!(spec >= '0' && spec <= '9'))
				break;
			base = width;		// width *= 10, base is just a temp here
			width <<=2;
			width += base;
			width <<=1;

			width += (spec - '0');
			++fmt;
		}

// Init numeric control flags in case we have a number to process

		SIZE = sizeof(int);		// Long/Short/Tiny flag, int is default
		base = 0;		// If base gets set below we have a number!
		shift = 0;		// Used for fast multiply

// Handle LARGE numbers
//

		if (spec == 'l') {	// Large
			SIZE = sizeof(long);
			spec = *fmt++;
		}
		if (spec == 't') {	// tiny
			SIZE = sizeof(char);
			spec = *fmt++;
		}
		
// spec has our format specifier!
		switch (spec)
		{
			case '%':
				break;
			case 'c':
				{
					unsigned char *c;
					c = va_arg(ap,unsigned char *);
					*c = *strp;
					++strp;
				}
				break;
			case 's':
				{
					unsigned char *p;
					unsigned char c;
					p = va_arg(ap,unsigned char *);
					while (width) 
					{
						c = *strp;
						if (!c || c == ' ' || c == '\t')
							break;
						*p = c;
						++p;
						++strp;
						--width;
					}
					*p = 0;
				}
				break;
			case 'B':
					SIZE = sizeof(long);
			case 'b':
				base = 2;
				shift = 1;
				break;
			case 'O':
					SIZE = sizeof(long);
			case 'o':
				base = 8;
				shift = 3;
				break;
			case 'X':
					SIZE = sizeof(long);
			case 'x':
				base = 16;
				shift = 4;
				break;
			case 'U':
			case 'D':
					SIZE = sizeof(long);
			case 'u':
			case 'd':
				base = 10;
//printf("sscanf: sizeof(long) = %d\n", sizeof(long));
//printf("sscanf: sizeof(int) = %d\n", sizeof(int));
//printf("sscanf: size = %d, base = %d\n", (int)SIZE, (int)base);
//printf("sscanf strp:%s\n", strp);
				break;
			case 'f':
					SIZE = sizeof(double);
				break;
			default:
				break;
		}
// IF base is non zero we have some kind of number to process

		if(base) {
			num = 0;
			SIGN = 0;
			if (!width)
				width = strlen(strp);

			ch = *strp;
			if (ch == '-')
			{
				SIGN = 1;
				++strp;
			}
			else if(ch == '+')
			{
				++strp;
			}

			while (width && (ch = *strp)) {
				if (ch < '0')
					break;

				if (ch >= 'a') 
					ch -= ('a' - 10);
				else if (ch >= 'A')
					ch -= ('A' - 10);
				else
					ch -= '0';

				if (ch >= base)
					break;

				if(base == 10) {
					unsigned long temp;
					num <<= 1;
					temp = num;
					num <<= 2;
					num += temp;
				}
				else {
					num <<= shift;
				}
				num += ch;
				++strp;
				--width;
//printf("sscanf: %ld\n",num);
			}				// END WHILE
			if(SIZE == sizeof(long)) {
				unsigned long *c;
				c = va_arg(ap,unsigned long *);
				if(SIGN)
					*c = (unsigned long) -num;
				else
					*c = (unsigned long) num;
			}
			else if(SIZE == sizeof(int)) {
				unsigned int *c;
				c = va_arg(ap,unsigned int *);
				if(SIGN)
					*c = (unsigned int) -num;
				else
					*c = (unsigned int) num;
			}
			else {
				unsigned char *c;
				c = va_arg(ap,uint8_t *);
				if(SIGN)
					*c = (unsigned char) -num;
				else
					*c = (unsigned char) num;
			}
		}				// END IF(base && width)
		else if(SIZE = sizeof(double))
		{
			// FIXME width
			double *d = va_arg(ap,double *);
			char *ptr,*endp;
			int len = strlen(strp);
			if(!width)
				width = len;
			if(width > len)
				width = len;
			ptr = stralloc((char *)strp);
			*d = strtod(ptr, &endp);
			free(ptr);
			strp += (endp - ptr);
		}
		++args;
	}					// END WHILE
	return (args);
}

#endif // ifdef SMALL_SSCANF
