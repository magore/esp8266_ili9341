/**
 @file test_printf.c

 @brief Test routines for Small printf

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

// only used when testing standalone on linux
#ifdef PRINTF_TEST

#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>

#include <stdarg.h>
#include <stdint.h>
#include <math.h>

#include "mathio.h"

/// @brief _putc_fn low level function that writes a character with putchar()
/// @param[in] *p: structure with pointers to track number of bytes written
/// @param[in] ch: character to write
/// @return void
static void _putc_fn(struct _printf_t *p, char ch)
{
	p->sent++;
	putchar(ch);
}

/// @brief printf function
/// @param[in] fmt: printf forat string
/// @param[in] va_list: vararg list or arguments
/// @return size of printed string
int t_printf(const char *format, ...)
{
    int len;
    int i;
    printf_t fn;
	va_list va;

    fn.put = _putc_fn;
    fn.sent = 0;

    va_start(va, format);
    _printf_fn(&fn, format, va);
    va_end(va);

    len = fn.sent;
    return (len);
}

/// @brief printf tests
/// Compare printf results from gcc printf and this printf
/// @return void
void tests()
{
puts("[%c]\\n, 'a'");
 t_printf("    [%c]\n", 'a');
   printf("    [%c]\n", 'a');
puts("\n");

puts("[%-5c]\\n, 'a'");
 t_printf("    [%-5c]\n", 'a');
   printf("    [%-5c]\n", 'a');
puts("\n");

puts("[%5c]\\n, 'a'");
 t_printf("    [%5c]\n", 'a');
   printf("    [%5c]\n", 'a');
puts("\n");


puts("[%-20.2s]\\n, abc");
 t_printf("    [%-20.2s]\n", "abc");
   printf("    [%-20.2s]\n", "abc");
puts("\n");

puts("[%10.5s]\\n, abcdefg");
 t_printf("    [%10.5s]\n", "abcdefg");
   printf("    [%10.5s]\n", "abcdefg");
puts("\n");

puts("[%e]\\n, 314.159265358979");
 t_printf("    [%e]\n", 314.159265358979);
   printf("    [%e]\n", 314.159265358979);
puts("\n");

puts("[%e]\\n, -314.159265358979");
 t_printf("    [%e]\n", -314.159265358979);
   printf("    [%e]\n", -314.159265358979);
puts("\n");

puts("[%-+e]\\n, 314.159265358979");
 t_printf("    [%-+e]\n", 314.159265358979);
   printf("    [%-+e]\n", 314.159265358979);
puts("\n");

puts("[%-+e]\\n, -314.159265358979");
 t_printf("    [%-+e]\n", -314.159265358979);
   printf("    [%-+e]\n", -314.159265358979);
puts("\n");


puts("[%-+15.4e]\\n, 314.159265358979");
 t_printf("    [%-+15.4e]\n", 314.159265358979);
   printf("    [%-+15.4e]\n", 314.159265358979);
puts("\n");

puts("[%-+15.4e]\\n, -314.159265358979");
 t_printf("    [%-+15.4e]\n", -314.159265358979);
   printf("    [%-+15.4e]\n", -314.159265358979);
puts("\n");


puts("[%-+15e]\\n, 314.159265358979");
 t_printf("    [%-+15e]\n", 314.159265358979);
   printf("    [%-+15e]\n", 314.159265358979);
puts("\n");

puts("[%-+15e]\\n, -314.159265358979");
 t_printf("    [%-+15e]\n", 314.159265358979);
   printf("    [%-+15e]\n", -314.159265358979);
puts("\n");

puts("[%15e]\\n, 314.159265358979");
 t_printf("    [%15e]\n", 314.159265358979);
   printf("    [%15e]\n", 314.159265358979);
puts("\n");

puts("[%15e]\\n, -314.159265358979");
 t_printf("    [%15e]\n", -314.159265358979);
   printf("    [%15e]\n", -314.159265358979);
puts("\n");

puts("[%20.5e]\\n, 314.159265358979");
 t_printf("    [%20.5e]\n", 314.159265358979);
   printf("    [%20.5e]\n", 314.159265358979);
puts("\n");

puts("[%20.5e]\\n, -314.159265358979");
 t_printf("    [%20.5e]\n", -314.159265358979);
   printf("    [%20.5e]\n", -314.159265358979);
puts("\n");

puts("[%08.0f], 1.0");
 t_printf("    [%08.0f]\n", 1.0);
   printf("    [%08.0f]\n", 1.0);
puts("\n");

puts("[%08.0f], -1.0");
 t_printf("    [%08.0f]\n", -1.0);
   printf("    [%08.0f]\n", -1.0);
puts("\n");

puts("[%08.4f], 0.0");
 t_printf("    [%08.4f]\n", 0.0);
   printf("    [%08.4f]\n", 0.0);
puts("\n");

puts("[%08.4f], 314.159265358979");
 t_printf("    [%08.4f]\n", 314.159265358979);
   printf("    [%08.4f]\n", 314.159265358979);
puts("\n");

puts("[%08.4f], -314.159265358979");
 t_printf("    [%08.4f]\n", -314.159265358979);
   printf("    [%08.4f]\n", -314.159265358979);
puts("\n");

puts("[%-+08.4f], 314.159265358979");
 t_printf("    [%08.4f]\n", 314.159265358979);
   printf("    [%08.4f]\n", 314.159265358979);
puts("\n");

puts("[%-+08.4f], -314.159265358979");
 t_printf("    [%08.4f]\n", -314.159265358979);
   printf("    [%08.4f]\n", -314.159265358979);
puts("\n");

puts("[%f], 0.0");
 t_printf("    [%f]\n", 0.0);
   printf("    [%f]\n", 0.0);
puts("\n");

puts("[%8.2f], 0.0");
 t_printf("    [%8.2f]\n", 0.0);
   printf("    [%8.2f]\n", 0.0);
puts("\n");

puts("[%08.4f], 12.89");
 t_printf("    [%08.4f]\n", 12.89);
   printf("    [%08.4f]\n", 12.89);
puts("\n");

puts("[%.2f], 1234567.89");
 t_printf("    [%.2f]\n", 1234567.89);
   printf("    [%.2f]\n", 1234567.89);
puts("\n");

puts("[%10.5f]\\n, 314.159265358979");
 t_printf("    [%10.5f]\n", 314.159265358979);
   printf("    [%10.5f]\n", 314.159265358979);
puts("\n");

puts("[%10.5f]\\n, 123456789012345678901234567890.159265358979");
 t_printf("    [%10.5f]\n", 123456789012345678901234567890.159265358979);
   printf("    [%10.5f]\n", 123456789012345678901234567890.159265358979);
puts("\n");

puts("[%+014.8f]\\n, 3.14159265358979");
 t_printf("    [%+014.8f]\n", 3.14159265358979);
   printf("    [%+014.8f]\n", 3.14159265358979);
puts("\n");

puts("[%14.8f]\\n, 3.141");
 t_printf("    [%14.8f]\n", 3.141);
   printf("    [%14.8f]\n", 3.141);
puts("\n");

puts("[%-+15d]\\n, 1234");
 t_printf("    [%-+15d]\n", 1234);
   printf("    [%-+15d]\n", 1234);
puts("\n");

puts("[%+15.8d]\\n, 1234");
 t_printf("    [%+15.8d]\n", 1234);
   printf("    [%+15.8d]\n", 1234);
puts("\n");

puts("[% -15.8d]\\n, 1234");
 t_printf("    [% -15.8d]\n", 1234);
   printf("    [% -15.8d]\n", 1234);
puts("\n");

puts("[%-+15.8d]\\n, 1234");
 t_printf("    [%-+15.8d]\n", 1234);
   printf("    [%-+15.8d]\n", 1234);
puts("\n");

puts("[%.3d]\\n, 12345");
 t_printf("    [%.3d]\n", 12345);
   printf("    [%.3d]\n", 12345);
puts("\n");

puts("[% 15.8d]\\n, 1234");
 t_printf("    [% 15.8d]\n", 1234);
   printf("    [% 15.8d]\n", 1234);
puts("\n");

puts("[%+015d]\\n, 1234567890");
 t_printf("    [%+015d]\n", 1234567890);
   printf("    [%+015d]\n", 1234567890);
puts("\n");

puts("[%+020d]\\n, 1234567890");
 t_printf("    [%+020d]\n", 1234567890);
   printf("    [%+020d]\n", 1234567890);
puts("\n");

puts("[%020d]\\n, 1234567890");
 t_printf("    [%020d]\n", 1234567890);
   printf("    [%020d]\n", 1234567890);
puts("\n");

puts("[% -20d]\\n, 1234567890");
 t_printf("    [% -20d]\n", 1234567890);
   printf("    [% -20d]\n", 1234567890);
puts("\n");

puts("[%- 20d]\\n, 1234567890");
 t_printf("    [%- 20d]\n", 1234567890);
   printf("    [%- 20d]\n", 1234567890);
puts("\n");

puts("[%-+20d]\\n, 1234567890");
 t_printf("    [%-+20d]\n", 1234567890);
   printf("    [%-+20d]\n", 1234567890);
puts("\n");

puts("[%-20d]\\n, 1234567890");
 t_printf("    [%-20d]\n", 1234567890);
   printf("    [%-20d]\n", 1234567890);
puts("\n");

puts("[%+-20d]\\n, 1234567890");
 t_printf("    [%+-20d]\n", 1234567890);
   printf("    [%+-20d]\n", 1234567890);
puts("\n");

puts("[%+20d]\\n, 1234567890");
 t_printf("    [%+20d]\n", 1234567890);
   printf("    [%+20d]\n", 1234567890);
puts("\n");

puts("[%20d]\\n, 1234567890");
 t_printf("    [%20d]\n", 1234567890);
   printf("    [%20d]\n", 1234567890);
puts("\n");

puts("[%-20.5d]\\n, 123");
 t_printf("    [%- 20.5d]\n", 123);
   printf("    [%- 20.5d]\n", 123);
puts("\n");

puts("[%- 10.5d]\\n, 123");
 t_printf("    [%- 10.5d]\n", 123);
   printf("    [%- 10.5d]\n", 123);
puts("\n");

puts("[%10.5d]\\n, 123");
 t_printf("    [% 10.5d]\n", 123);
   printf("    [% 10.5d]\n", 123);
puts("\n");

puts("[% - .5d]\\n, 123");
 t_printf("    [% -.5d]\n", 123);
   printf("    [% -.5d]\n", 123);
puts("\n");

puts("[% -5d]\\n, 123");
 t_printf("    [% -5d]\n", 123);
   printf("    [% -5d]\n", 123);
puts("\n");

puts("[%+-6d]\\n, 123");
 t_printf("    [%+06d]\n", 123);
   printf("    [%+06d]\n", 123);
puts("\n");

}

/// @brief main printf test programe
/// Run a number of conversion tests
/// @return 0
#define MAXSTR 256
int main(int argc, char *argv[])
{

	uint8_t str[MAXSTR+1];
	long num, num2, mask;
	int i;
	mask = 1;
	num = 0;


	tests();

	printf("1's\n");
	num = ~num;
	for(i=0;i<64;++i)
	{
		num &= ~mask;
		mask <<= 1;

		t_printf("[%+020ld]\n", num);
		printf("[%+020ld]\n", num);
		sprintf(str,"%+020ld", num);

		num2 = aton(str,10);
		if(num2 != num)
			printf("**:%ld\n",num2);

		p_ntoa(num, str, sizeof(str), 16, 16);
		printf("[%s]\n",str);
		p_ntoa(num, str, sizeof(str), 8, 22);
		printf("[%s]\n",str);
		p_ntoa(num, str, sizeof(str), 2, 64);
		printf("[%s]\n",str);

		printf("\n");
		
	}
	printf("=================================\n");
	printf("1's\n");
	num = 0;
	mask = 1;
	for(i=0;i<64;++i)
	{
		num |= mask;
		mask <<= 1;
		t_printf("[%+020ld]\n", num);
		printf("[%+020ld]\n", num);

		sprintf(str,"%+020ld", num);
		num2 = aton(str,10);
		if(num2 != num)
			printf("***:%ld\n",num2);

		p_ntoa(num, str, sizeof(str), 16, 16);
		printf("[%s]\n",str);
		p_ntoa(num, str, sizeof(str), 8, 22);
		printf("[%s]\n",str);
		p_ntoa(num, str, sizeof(str), 2, 64);
		printf("[%s]\n",str);

		printf("\n");
	}

	printf("=================================\n");
	printf("9's\n");
	num = 9;
	for(i=0;i<32;++i)
	{
		t_printf("[%+020ld]\n", num);
		printf("[%+020ld]\n", num);

		sprintf(str,"%+020ld", num);
		num2 = aton(str,10);
		if(num2 != num)
			printf("***:%ld\n",num2);

		p_ntoa(num, str, sizeof(str), 16, 16);
		printf("[%s]\n",str);
		p_ntoa(num, str, sizeof(str), 8, 22);
		printf("[%s]\n",str);
		p_ntoa(num, str, sizeof(str), 2, 64);
		printf("[%s]\n",str);

		printf("\n");

		if(num &  (1L << ((sizeof(num)*8)-1)))
			break;
		num *= 10;
		num += 9;
		
	}
	printf("\n");
	printf("=================================\n");


	printf("-1\n");
	num = -1;
	t_printf("[%+020ld]\n", num);
	printf("[%+020ld]\n", num);
	sprintf(str,"%+020ld", num);

	num2 = aton(str,10);
	if(num2 != num)
		printf("***:%ld\n",num2);

	p_ntoa(num, str, sizeof(str), 16, 0);
	printf("[%s]\n",str);
	p_ntoa(num, str, sizeof(str), 8, 0);
	printf("[%s]\n",str);
	p_ntoa(num, str, sizeof(str), 2, 0);
	printf("[%s]\n",str);

	printf("\n");
	printf("=================================\n");

	printf("1 << 63\n");
	num = 1UL;
	num <<= 63;

	t_printf("[%020lu]\n", num);
	printf("[%020lu]\n", num);
	sprintf(str,"%+020ld", num);

	num2 = aton(str,10);
	if(num2 != num)
		printf("***:%ld\n",num2);

	p_ntoa(num, str, sizeof(str), 16, 0);
	printf("[%s]\n",str);
	p_ntoa(num, str, sizeof(str), 8, 0);
	printf("[%s]\n",str);
	p_ntoa(num, str, sizeof(str), 2, 0);
	printf("[%s]\n",str);

	printf("\n");
	printf("=================================\n");

	printf("0\n");
	num = 0;
	t_printf("[%+020ld]\n", num);
	printf("[%+020ld]\n", num);
	sprintf(str,"%+020ld", num);

	num2 = aton(str,10);
	if(num2 != num)
		printf("***:%ld\n",num2);

	p_ntoa(num, str, sizeof(str), 16, 0);
	printf("[%s]\n",str);
	p_ntoa(num, str, sizeof(str), 8, 0);
	printf("[%s]\n",str);
	p_ntoa(num, str, sizeof(str), 2, 0);
	printf("[%s]\n",str);

	printf("\n");
	printf("=================================\n");

	printf("0\n");
	num = 0;
	t_printf("[%+ld]\n", num);
	printf("[%+ld]\n", num);
	sprintf(str,"%+020ld", num);

	num2 = aton(str,10);
	if(num2 != num)
		printf("***:%ld\n",num2);

	p_ntoa(num, str, sizeof(str), 16, 0);
	printf("[%s]\n",str);
	p_ntoa(num, str, sizeof(str), 8, 0);
	printf("[%s]\n",str);
	p_ntoa(num, str, sizeof(str), 2, 0);
	printf("[%s]\n",str);

	printf("\n");
	printf("=================================\n");

	return(0);
}
#endif	
