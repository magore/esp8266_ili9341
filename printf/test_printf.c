/**
 @file test_printf.c

 @brief Test routines for Small printf

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

// only used when testing standalone on linux
#ifdef PRINTF_TEST

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

#include <stdarg.h>
#include <stdint.h>
#include <math.h>

#include "mathio.h"


/// @brief compare significant digits and exponennt of floating point numbers
/// We match:
///   - String lengths
///   - Signs +/-
///   - Leading spaces 
/// Skip all matching leading zeros 
/// Skip optional matching decimal point
///   If these step succeeed
///   Then we collect at most a user specified number of significant digits
////    for each string;
///   Then we match optional exponent
///   Then we convert significant digits for each string to long long values
///   Then we return the absolute value of the difference of these numbers
/// @param[in] *str1: known good number string
/// @param[in] *str2: testing number string
/// return -1LL if a match step fails - steps listed above with the word match
/// return absolute value of significant digits as long long
long long numcmp(uint8_t *str1, uint8_t *str2, int max)
{

    int nzflag = 0;
    uint8_t n1[64+2],n2[64+2];
    uint8_t c1,c2;
    int ind1,ind2;
    int len1,len2;
    int count = 0;
    int offset = 0;
    long long diff, l1,l2;

    uint8_t *save1 = str1;
    uint8_t *save2 = str2;

    ind1 = 0;
    ind2 = 0;

    if(max > 64)
        max = 64;

    len1 = strlen(save1);   
    len2 = strlen(save2);   
    if(len1 != len2)
    {
        printf("WARN: length mismatch (%d) != (%d)\n", 
            len1,len2);
        printf("      str1:[%s]\n", save1);
        printf("      str2:[%s]\n", save2);
        return(-1LL);   
    }

    // discard leading space,,+/- characters 
    while( (c1 = *str1) && (c2 = *str2) )
    {
        if( isdigit(c1) || isdigit(c2) )
            break;

        if(c1 != c2)
        {
            n1[ind1] = 0;
            n2[ind2] = 0;
            printf("WARN: mismatch at (%d)\n", count);
            printf("      str1 offset:%d\n", (int)(str1 - save1));
            printf("      str2 offset:%d\n", (int)(str2 - save2));
            printf("      str1:[%s]\n", save1);
            printf("      str2:[%s]\n", save2);
            printf("      n1:[%s]\n", n1);
            printf("      n2:[%s]\n", n2);
            return(-1LL);   
        }
            
        // sign an leading zeros
        if(c1 == '.' || c1 == ' ' || c1 == '-' || c1 == '+')
        {
            ++str1;
            ++str2;
            ++count;
            continue;
        }
        break;
    }

    while( (c1 = *str1) && (c2 = *str2) )
    {
        if( c1 == '.' && c2 == '.' )
        {
            ++str1;
            ++str2;
            ++count;
            continue;
        }

        if( !isdigit(c1) || !isdigit(c2) )
            break;

        if(!nzflag && (c1 != '0' || c2 != '0'))
            nzflag = 1;
        
        if(nzflag)
        {
            // no more then 16 digits
            if(ind1 < max)
            {
                n1[ind1++] = c1;
                n2[ind2++] = c2;
            }
        }
        ++count;
        ++str1;
        ++str2;
    }
    n1[ind1] = 0;
    n2[ind2] = 0;

    c1 = *str1;
    c2 = *str2;

    if(c1 != c2)
    {
        printf("WARN: mismatch at (%d) %02X != %02X\n", 
            (int) count, (int) c1, (int) c2);
        printf("      str1 offset:%d\n", (int)(str1 - save1));
        printf("      str2 offset:%d\n", (int)(str2 - save2));
        printf("      str1:[%s]\n", save1);
        printf("      str2:[%s]\n", save2);
        printf("      n1:[%s]\n", n1);
        printf("      n2:[%s]\n", n2);
        return(-1LL);   
    }
    if(c1 == 'e' || c1 == 'E')
    {
        if(strcmp(str1,str2) != 0)
        {
            printf("WARN: exponent mismatch at offset:(%d)\n", count);
            printf("      str1:[%s]\n", save1);
            printf("      str2:[%s]\n", save2);
            printf("      n1:[%s]\n", n1);
            printf("      n2:[%s]\n", n2);
            return(-1LL);   
        }
    }

    l1 = strtoll(n1,NULL,10);
    l2 = strtoll(n2,NULL,10);
    diff = l1 - l2;
    if(diff < 0)
        diff = -diff;

    return(diff);
}


// =============================================
/// @brief _putc_fn low level function that writes a character with putchar()
/// @param[in] *p: structure with pointers to track number of bytes written
/// @param[in] ch: character to write
/// @return void
static void _putc_fn(struct _printf_t *p, char ch)
{
    p->sent++;
    putchar(ch);
}

/// @brief Our printf function for testing
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


// =============================================
/// @brief Our vsnprintf function for testing
/// @param[out] str: string buffer for result
/// @param[in] size: maximum length of converted string
/// @param[in] format: printf forat string
/// @param[in] va: va_list list of arguments
/// @return string size
MEMSPACE
int t_vsnprintf(char* str, size_t size, const char *format, va_list va)
{

    int len;
    char *save = str;
    printf_t fn;

    *str = 0;

    fn.put = _putc_buffer_fn;
    fn.len = size;
    fn.sent = 0;
    fn.buffer = (void *) str;

    _printf_fn(&fn, format, va);

    // FIXME check size should == fn.size on exit
    len = strlen(save);
    return( len );
}


// =============================================
int display_good = 0;
long tp_good = 0;   //@brief total good tests
long tp_bad = 0;    //@brief total bad  tests
long tp_fmt = 0;    //@brief total empty format string errors

/// @brief Manual test of glibc printf vs ours
/// We test for a worst case error of 1 LSB error at 15digits
/// @param[in] format: printf format string
/// @param[in] ...: list of arguments
/// @return void
MEMSPACE
void tp(const char *format, ...)
{
    char str0[1024];
    char fmt[1024];
    char str1[1024];
    char str2[1024];
    int f;
    int find, ind, len;
    int matched;
    long long error;
    va_list va;
    int digits;

    memset(str0,sizeof(str0)-1,0);
    memset(str1,sizeof(str1)-1,0);
    memset(str2,sizeof(str2)-1,0);
    memset(fmt,sizeof(fmt)-1,0);

    // We want to save only type and type size specifiers)
    find = 0;
    fmt[find++] = '%';

    len = strlen(format);
    if(len > 0)
    {
        ind = len - 1;
    }
    else 
    {
        printf("ERROR: empty format\n");
        printf("    G[%s]\n", str1);
        printf("    B[%s]\n", str2);
        printf("\n");
        ++tp_fmt;
        return;
    }
    
    // We may have a size adjustment specifier
    //FIXME add more as printf gains more type size conversion specifiers
    if(ind >= 2)
    {
        f = format[ind-2];
        if(f == 'l')    
            fmt[find++] = f;
    }
    f = format[ind-1];
    if(f == 'l' || f == 'h')    
        fmt[find++] = f;
    
    // This should be the primary conversion type specifier
    if(ind)
    {
        f = format[ind];
        fmt[find++] = f;
    }
    fmt[find++] = 0;

    // GLIBC printf in str0 without extra specifiers
    va_start(va, format);
    len = vsnprintf(str0, sizeof(str0)-1, fmt, va);
    va_end(va);
    fflush(stdout);
        
    // GLIBC printf in str1
    va_start(va, format);
    len = vsnprintf(str1, sizeof(str1)-1, format, va);
    va_end(va);
    fflush(stdout);

    // Our Printf in str2
    va_start(va, format);
    len = t_vsnprintf(str2, sizeof(str2)-1, format, va);
    va_end(va);
    fflush(stdout);



    //FIXME add more as printf gains more conversion functions
    if(f == 'g' || f == 'G' || f == 'e' || f == 'E' || f == 'f' || f == 'F')
    {
        /*
         * Single mantissa 24bits   base10 digits   7.22     
         *      exponent 8bits   base10 exponent 37
         * Double mantissa 53bits   base10 digits   15.95   
         *      exponent 11bits  base10 exponent 307
        */
        if(sizeof(double) == 8)
            digits = 16;
        else if(sizeof(double) == 4)
            digits = 7;
        else
        {
            fprintf(stderr,"Unexpected size of double:%d\n", (int)sizeof(double));
            exit(1);
        }

        // Compare results to N digit window
        error = numcmp(str1,str2,digits);

        // A double 1 LSB error would be 10LL
        // Remember the numbers may be rounded so we use less then 15LL
        if(error < 0 || error > 14LL)
        {
            printf("ERROR: [%s], [%s]\n", format, str0);
            printf("    G[%s]\n", str1);
            printf("    B[%s]\n", str2);
            printf("    error:%lld\n", error);
            ++tp_bad;
        }
        else
        {
            ++tp_good;
            if(display_good)
            {
                printf("OK:    [%s], [%s]\n", format, str0);
                printf("    G[%s]\n", str1);
            }
        }
    }
    else 
    {
        if(strcmp(str1,str2) != 0)
        {
            printf("ERROR: [%s], [%s]\n", format, str0);
            printf("    G[%s]\n", str1);
            printf("    B[%s]\n", str2);
            ++tp_bad;
        }
        else
        {
            ++tp_good;
            if(display_good)
            {
                printf("OK:    [%s], [%s]\n", format, str0);
                printf("    G[%s]\n", str1);
            }
        }
    }
    fflush(stdout); 
        return;
    fflush(stdout);
}



// =============================================
/// @brief Do random printf tests - glibc vc ours
/// We use random width, an random optional precision
/// @param[in] flag: 'f' or 'F' or 'e' or 'E'
/// @param[in] longf: add 'l' to format string
/// @return void
MEMSPACE
void random_tests(int flag, char *size)
{
    int snum;
    int inum;
    long lnum;
    long long llnum;
    double dnum, scale;
    
    int width,prec;
    int exp10;
    int shift;
    int precf;
    double sign;
    int digits;
    int dotf;
    int signind;
    char *signop = "+- 0";
    char format[1024];
    char tmp[1024];

    memset(format,0,128);


    if(drand48() >= .5)
        sign = 1.0;
    else
        sign = -1.0;

    if(drand48() >= .5)
        precf = 1;
    else
        precf = 0;
    if(drand48() >= .5)
        dotf = 1;
    else
        dotf = 0;
    signind = drand48() * 3.99999;  

    //printf("num:%ld\n",num);

    // With f we limit the exponent to +/-2 ** sizeof(long long)
    if(flag == 'e' || flag == 'f' )
    {
        /*
         * Single mantissa 24bits   base10 digits   7.22     
         *      exponent 8bits   base10 exponent 37
         * Double mantissa 53bits   base10 digits   15.95   
         *      exponent 11bits  base10 exponent 307
        */

        // We only test exponent to +/- digits * 2 
        // If we test numbers greater then (10 ** digits) we start getting
        // roundoff/truncation errors
        // 
        // Glibc printf uses extended precision functions (ie. > double size)

        if(sizeof(double) == 8)
        {
            digits = 16;
        }
        else if(sizeof(double) == 4)
        {
            digits = 7;
        }
        else
        {
            fprintf(stderr,"Unexpected size of double:%d\n",(int)sizeof(double));
            exit(1);
        }
        width = drand48() * 2.0 * digits;
        prec = drand48() * 2.0 * digits;
        if(precf || dotf)
            snprintf(format,sizeof(format)-1, "%%%c%d.%d%c", signop[signind], width, prec, flag);
        else
            snprintf(format,sizeof(format)-1, "%%%c%d%c", signop[signind], width, flag);
        

        exp10 = ((drand48() * 2.0) - 1.0) * (double) (digits * 2);
        scale = iexp(10.0, exp10);
        dnum = ( sign * drand48() * scale);
        tp(format, dnum);
    }
    else    /* ASSUME integer or long arguments */
    {
        if(strcmp(size,"short") == 0)
        {
            width = drand48() * (double)(sizeof(short) * 8)/3.321928095;
            prec = drand48() * (double)(sizeof(short) * 8)/3.321928095;
            shift = drand48() * (double) sizeof(int) * 8;
            scale = pow(2.0, shift);
            inum = (int) ( sign * drand48() * scale);
            if(precf || dotf)
                snprintf(format,sizeof(format)-1, "%%%c%d.%dh%c", signop[signind], width, prec, flag);
            else
                snprintf(format,sizeof(format)-1, "%%%c%dh%c", signop[signind], width, flag);
            tp(format, inum);
        }
        else if(strcmp(size,"int") == 0)
        {
            width = drand48() * (double)(sizeof(int) * 8)/3.321928095;
            prec = drand48() * (double)(sizeof(int) * 8)/3.321928095;
            shift = drand48() * (double) sizeof(int) * 8;
            scale = pow(2.0, shift);
            inum = (int) ( sign * drand48() * scale);
            if(precf || dotf)
                snprintf(format,sizeof(format)-1, "%%%c%d.%d%c", signop[signind], width, prec, flag);
            else
                snprintf(format,sizeof(format)-1, "%%%c%d%c", signop[signind], width, flag);
            tp(format, inum);
        }
        else if(strcmp(size,"long") == 0)
        {
            width = drand48() * (double)(sizeof(long) * 8)/3.321928095;
            prec = drand48() * (double)(sizeof(long) * 8)/3.321928095;
            shift = drand48() * (double) sizeof(long) * 8;
            scale = pow(2.0, shift);
            lnum = (long) ( sign * drand48() * scale);
            if(precf || dotf)
                snprintf(format,sizeof(format)-1, "%%%c%d.%dl%c", signop[signind], width, prec, flag);
            else
                snprintf(format,sizeof(format)-1, "%%%c%dl%c", signop[signind], width, flag);
            tp(format, lnum);
        }
        else if(strcmp(size,"long long") == 0)
        {
            width = drand48() * (double)(sizeof(long long) * 8)/3.321928095;
            prec = drand48() * (double)(sizeof(long long) * 8)/3.321928095;
            shift = drand48() * (double) sizeof(long long) * 8;
            scale = pow(2.0, shift);
            llnum = (long) ( sign * drand48() * scale);
            if(precf || dotf)
                snprintf(format,sizeof(format)-1, "%%%c%d.%dll%c", signop[signind], width, prec, flag);
            else
                snprintf(format,sizeof(format)-1, "%%%c%dll%c", signop[signind], width, flag);
            tp(format, llnum);
        }
        else
        {
            fprintf(stderr,"random_tests: bad size[%s]\n", size);
            exit(1);
        }
    }
}

// =============================================
/// @brief Manual printf tests - glibc vc ours
/// Compare printf results from gcc printf and this printf
/// @return void
void tests()
{
    int i1 = -1;
    int l1 = -1L;

    printf("=======================\n");
    printf("Start of Manual tests\n");
    tp("%-0d", 0);
    tp("%-0u", 0);
    tp("%-0u", 1);
    tp("%-0u", -1);
    tp("%00u", 0);
    tp("%00u", 1);
    tp("%00u", -1);
    tp("% -.5u", 123);
    tp("% - .5u", -123);
    tp("% 1.5u", -6);
    tp("%+1.4u", 372);
    tp("%-0lu", 0);
    tp("%-0lu", 1);
    tp("%-0lu", -1);
    tp("%00lu", 0);
    tp("%00lu", 1);
    tp("%00lu", -1);
    tp("% -.5lu", 123);
    tp("% - .5lu", -123);
    tp("% 1.5lu", -6);
    tp("%+1.4lu", 372);
    tp("%-0ld", 0);
    tp("%-0ld", 1);
    tp("%-0ld", -1);
    tp("%00ld", 0);
    tp("%00ld", 1);
    tp("%00ld", -1);
    tp("% -.5ld", 123);
    tp("% - .5ld", -123);
    tp("% 1.5ld", -6);
    if(display_good)
        printf("\n");

    tp("%-0d", 0);
    tp("%-0d", 1);
    tp("%-0d", -1);
    tp("%00d", 0);
    tp("%00d", 1);
    tp("%00d", -1);
    tp("% -.5d", 123);
    tp("% - .5d", -123);
    tp("% 1.5d", -6);
    tp("%+1.4d", 372);
    tp("%+03d", 0);
    tp("%+03d", -1);
    tp("%+03d", 1);
    tp("%+03.0d", 0);
    tp("%+03.0d", -1);
    tp("%+03.0d", 1);
    tp("%-+03d", 0);
    tp("%-+03d", -1);
    tp("%-+03d", 1);
    tp("%-+03.0d", 0);
    tp("%-+03.0d", -1);
    tp("%-+03.0d", 1);
    if(display_good)
        printf("\n");

    tp("%+0f", -0.196764);
    tp("%08.0f", 1.5);
    tp("%08.0f", -1.5);
    tp("%08.4f", 0.0);
    tp("%30.2f", 0.123456789012345678901234567890);
    tp("%30.2f", 0.00000000000123456789012345678901234567890);
    tp("%+30.15f", 0.00000000000123456789012345678901234567890);
    tp("%+030.15f", 0.00000000000123456789012345678901234567890);
    tp("% 30.15f", 0.00000000000123456789012345678901234567890);
    tp("%+30.15f",   123456.789012345678901234567890);
    tp("%+030.15f",  123456.789012345678901234567890);
    tp("%+30.15f",  -123456.789012345678901234567890);
    tp("%+030.15f", -123456.789012345678901234567890);
    tp("%+15.13f", -0.0085833);
    tp("%+15.13f", 0.0085833);
    tp("%+15.13f", 0.085833);
    tp("%+15.13f", 0.85833);
    tp("%+15.13f", 8.5833);
    tp("%+15.13f", 85.833);
    tp("%+15.13f", 123456789012345678901234567890.159265358979); 
    tp("%+15.2f", 123456789012345678901234567890.159265358979); 
    tp("%15.2f", 123456789012345678901234567890.159265358979); 
    tp("%f", 123456789012345678901234567890.159265358979); 
    tp("%08.0f", 1.0);
    tp("%08.0f", -1.0);
    tp("%f", 0.0);
    tp("%8.2f", 0.0);
    tp("%08.4f", 12.89);
    tp("%08.4f", -12.89);
    tp("%.2f", 1234567.89);
    tp("%.2f", -1234567.89);
    tp("%+.2f", 1234567.89);
    tp("%+.2f", -1234567.89);
    tp("% .2f", 1234567.89);
    tp("%08.4f", 0.0);
    tp("%+014.8f", 3.14159265358979);
    tp("%+14.8f", 3.141);
    tp("% .2f", -1234567.89);
    if(display_good)
        printf("\n");

    tp("%015e", -314.159265358979);
    tp("%020.5e", 314.159265358979);
    tp("%020.5e", -314.159265358979);
    tp("%-+015.4e", 314.159265358979);
    tp("%-+015.4e", -314.159265358979);
    tp("%-+025.10e", 314.159265358979);
    tp("%-+025.10e", -314.159265358979);
    tp("%-+25.10e", 314.159265358979);
    tp("%-+25.10e", -314.159265358979);
    tp("%+25.10e", 314.159265358979);
    tp("%+25.10e", -314.159265358979);
    tp("% 015.4e", 314.159265358979);
    tp("% 015.4e", -314.159265358979);
    tp("%015e", -314.159265358979);
    tp("%020.5e", 314.159265358979e-17);
    tp("%020.5e", -314.159265358979e-17);
    tp("%-+015.4e", 314.159265358979e-17);
    tp("%-+015.4e", -314.159265358979e-17);
    tp("%-+025.10e", 314.159265358979e-17);
    tp("%-+025.10e", -314.159265358979e-17);
    tp("%-+25.10e", 314.159265358979e-17);
    tp("%-+25.10e", -314.159265358979e-17);
    tp("%+25.10e", 314.159265358979e-17);
    tp("%+25.10e", -314.159265358979e-17);
    tp("% 015.4e", 314.159265358979e-17);
    tp("%010.5e", 314.159265358979e-17);
    tp("%010.5e", -314.159265358979e-17);
    tp("%010.5e", 123456789012345678901234567890.159265358979); 
    if(display_good)
        printf("\n");

    tp("%e", 314.159265358979);
    tp("%e", -314.159265358979);
    tp("%+e", -314.159265358979);
    tp("% e", 314.159265358979);
    tp("%-+e", 314.159265358979);
    tp("%-+e", -314.159265358979);
    tp("%-+15.4e", 314.159265358979);
    tp("%-+15.4e", -314.159265358979);
    tp("%-+15e", 314.159265358979);
    tp("%-+15e", -314.159265358979);
    tp("%15e", 314.159265358979);
    tp("%15e", -314.159265358979);
    tp("%20.5e", 314.159265358979);
    tp("%20.5e", -314.159265358979);
    tp("%08.4e", 314.159265358979);
    tp("%08.4e", -314.159265358979);
    tp("%-+08.4e", 314.159265358979);
    tp("%-+08.4e", -314.159265358979);
    tp("% 08.4e", 314.159265358979);
    tp("% 08.4e", -314.159265358979);
    tp("%10.5e", 314.159265358979);
    tp("%10.5e", -314.159265358979);
    tp("%10.5e", 123456789012345678901234567890.159265358979); 
    tp("%10.10e", 123456789012345678901234567890.159265358979); 
    tp("%10.15e", 123456789012345678901234567890.159265358979); 
    tp("%10.20e", 123456789012345678901234567890.159265358979); 
    tp("%10.20e", 123456789012345678901234567890.159265358979e-50); 
    tp("%10.20e", 123456789012345678901234567890.159265358979e+50); 
    tp("%10.20e", 123456789012345678901234567890.159265358979e-100); 
    tp("%10.20e", 123456789012345678901234567890.159265358979e+100); 
    if(display_good)
        printf("\n");


    tp("%c", 'a');
    tp("%-5c", 'a');
    tp("%5c", 'a');
    if(display_good)
        printf("\n");
    tp("-20.2s", "abc");
    tp("10.5s", "abc");
    if(display_good)
        printf("\n");
    printf("End of Manual tests\n");
    printf("=======================\n");
    printf("\n");
}

/// @brief main printf test programe
/// Run a number of conversion tests and display good and bad result totals
/// @return 0
#define MAXSTR 256
int main(int argc, char *argv[])
{

    uint8_t str[MAXSTR+1];
    long lnum, mask;
    int i;
    int k;
    int size;
#ifdef __SIZEOF_INT128__
    __uint128_t num128;
#endif

    f_t f;

    char *intops = "duxXo";
    char *sizeops[] = { "short", "int", "long", "long long", NULL };
    char *floatops = "fe";

    printf("=======================\n");
    printf("Start of Manual tests\n");

// Test basic type sizes
    t_printf("sizeof (double) = %d\n", sizeof (double ) );
    t_printf("sizeof (float) = %d\n", sizeof (float ) );
#ifdef __SIZEOF_INT128__
    t_printf("sizeof (__uint128_t) = %d\n", sizeof (__uint128_t) );
#endif
    t_printf("sizeof (long long) = %d\n", sizeof (long long ) );
    t_printf("sizeof (long) = %d\n", sizeof (long ) );
    t_printf("sizeof (short) = %d\n", sizeof (short) );
    t_printf("sizeof (int) = %d\n", sizeof (int ) );
    t_printf("sizeof (char) = %d\n", sizeof (char ) );
    printf("=======================\n");
    printf("\n");


#ifdef __SIZEOF_INT128__
    printf("=======================\n");
    printf("Start of 128 bit int tests\n");
    // There are no 128bit int constants in gcc - sigh
    num128 = 1;
    for(i=0;i<128;++i)
    {
        t_printf("2**%03d = [%I128d]\n", i, num128);
        fflush(stdout);
        num128 <<= 1;
    }
    printf("=======================\n");
    printf("\n");
    printf("=======================\n");
    printf("Start of 128 bit int tests - 40 field width\n");
    num128 = 1;
    for(i=0;i<128;++i)
    {
        t_printf("2**%03d = [%40I128d]\n", i, num128);
        fflush(stdout);
        num128 <<= 1;
    }
    printf("=======================\n");
    printf("\n");
    printf("=======================\n");
    printf("Start of 128 bit int tests - 40 field width and leading 0's\n");
    num128 = 1;
    for(i=0;i<128;++i)
    {
        t_printf("2**%03d = [%+040I128d]\n", i, num128);
        fflush(stdout);
        num128 <<= 1;
    }
    printf("=======================\n");
    printf("\n");
#endif

    display_good = 0;
    tests();

    printf("\n\n");
    printf("Start of random tests\n");


    display_good = 0;
    for(size=0;sizeops[size];++size)
    {
        for(k=0;intops[k];++k)
        {
            tp_good = 0;
            tp_bad = 0;
            printf("=======================\n");
            printf("Start:(%c:%s)\n", intops[k], sizeops[size]);
            for(i=0;i<1000000;++i)
                random_tests(intops[k], sizeops[size]);
            printf("End:  (%c:%s)\n", intops[k], sizeops[size]);
            printf("Good:%ld, Bad:%ld, fmt:%ld\n", tp_good, tp_bad, tp_fmt);    
            printf("=======================\n");
        }
    }

    display_good = 0;
    for(k=0;floatops[k];++k)
    {
        tp_good = 0;
        tp_bad = 0;
        printf("=======================\n");
        printf("Start:(%c)\n", floatops[k]);
        for(i=0;i<1000000;++i)
            random_tests(floatops[k], "");
        printf("End:  (%c)\n", floatops[k]);
        printf("Good:%ld, Bad:%ld, fmt:%ld\n", tp_good, tp_bad, tp_fmt);    
        printf("=======================\n");
    }
    printf("\n");
    printf("Random done\n");

    printf("=======================\n");
    printf("testing binary leading 1's\n");
    lnum = 0;
    lnum = ~lnum;
    mask = 1;
    while(mask)
    {
        lnum &= ~mask;
        mask <<= 1;
        tp("%016lx", lnum);
        tp("%019ld", lnum);
        tp("%022lo", lnum);
        
    }
    printf("=================================\n");
    printf("testing binary trailing 1's\n");
    lnum = 0;
    mask = 1;
    while(mask)
    {
        lnum |= mask;
        mask <<= 1;
        tp("%016lx", lnum);
        tp("%019ld", lnum);
        tp("%022lo", lnum);
    }

    printf("=================================\n");
    printf("testing base 10 9's\n");
    lnum = 9;
    while(1)
    {
        if(lnum &  (1L << ((sizeof(lnum)*8)-1)))
            break;
        tp("%016lx", lnum);
        tp("%019ld", lnum);
        tp("%022lo", lnum);
        lnum *= 10;
        lnum += 9;
        
    }
    printf("\n");
    printf("=================================\n");
    return(0);
}
#endif  
