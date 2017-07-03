/**
  @file stringsup.h 

 @brief Various string and character functions

 @par Copyright &copy; 2016 Mike Gore, GPL License
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

#include <string.h>
#include "lib/stringsup.h"

// =============================================
///@brief print seperator
MEMSPACE
void sep()
{
    printf("==============================\n");
}

// =============================================
// Character functions
// =============================================

// =============================================
/// @brief test if a character is a digit
/// @param[in] c: character
/// @return true or false
MEMSPACE
int
WEAK_ATR
isdigit(int c) 
{
    if(c >= '0' && c <= '9')
        return(1);
    return(0);
}

// =============================================
///@brief Is a character upper case
///
/// @param[in] c: character.
///
/// @return 1 of upper case, else 0
MEMSPACE
int
WEAK_ATR
isupper(int c)
{
    if(c >= 'A' && c <= 'Z')
        return(1);
    return(0);
}

// =============================================
//@brief Is a character lower case
///
/// @param[in] c: character.
///
/// @return 1 of lower case, else 0
MEMSPACE
int
WEAK_ATR
islower(int c)
{
    if(c >= 'a' && c <= 'a')
        return(1);
    return(0);
}

// =============================================
///@brief Convert character to lower case, only if it is upper case
///
/// @param[in] c: character.
///
/// @return character or lowercase value or character
MEMSPACE
int
WEAK_ATR
tolower(int c)
{
    if(isupper(c))
        return(c - 'A' + 'a');
    return(c);
}

// =============================================
///@brief Convert character to upper case, only if it is lower case
///
/// @param[in] c: character.
///
/// @return character or upper case value or character
MEMSPACE
int
WEAK_ATR
toupper(int c)
{
    if(islower(c))
        return(c - 'a' + 'A');
    return(c);
}

/// @brief find a character in a string of maximum size
/// @param[in] str: string
/// @param[in] c: character
/// @param[in] size: string length to search
/// @return string length
MEMSPACE
void *memchr(const void *str, int c, size_t size)
{
	const uint8_t *ptr = str;
	while(size--)
	{
		if (*ptr++ == (uint8_t) c) 
			return (void *) (ptr - 1);
	} 
    return NULL;
}

// =============================================
// String functions
// =============================================
// =============================================
/// @brief String Length
/// @param[in] str: string
/// @return string length
MEMSPACE
size_t
WEAK_ATR
strlen(const char *str)
{
    int len=0;
    // String length
    while(*str++)
        ++len;
    return(len);
}

/// @brief copy a string 
/// @param[in] dest: destination string
/// @param[in] src: source string
/// @return destination string
MEMSPACE
 WEAK_ATR char *
strcpy(char *dest, const char *src)
{
	char *ptr = dest;
	while(*src)
	{
		*ptr++ = *src++;
	} 
	*ptr ++ = 0;
    return (ptr);
}

/// @brief copy a string of at most N characters
/// @param[in] dest: destination string
/// @param[in] src: source string
/// @param[in] size: maximum destination size
/// @return destination string
MEMSPACE
 WEAK_ATR
char * strncpy(char *dest, const char *src, size_t size)
{
	char *ptr = dest;
	while(*src && size)
	{
		*ptr++ = *src++;
		size--;
	} 
	while(size--)
		*ptr++ = 0;
    return (dest);
}


/// @brief Append string 
/// @param[in] dest: string
/// @param[in] src: string
/// @return string length
MEMSPACE
WEAK_ATR
char * strcat(char *dest, const char *src)
{
	char *ptr = dest;
	while(*ptr)
		++ptr;
    strcpy(ptr,src);
	return(dest);
}

/// @brief Append string of at most N bytes from src
/// @param[in] dest: string
/// @param[in] src: string
/// @return string length
MEMSPACE
WEAK_ATR
char * strncat(char *dest, const char *src, size_t max)
{
	char *ptr = dest;
	while(*ptr)
		++ptr;
    strncpy(ptr,src,max);
	return(dest);
}

// =============================================
/// @brief Reverse a string in place
///  Example: abcdef -> fedcba
/// @param[in] str: string
/// @return string length
MEMSPACE
void
WEAK_ATR
reverse(char *str)
{
    char temp;
    int i;
    int len = strlen(str);
    // Reverse
    // We only exchange up to half way
    for (i = 0; i < (len >> 1); i++)
    {
        temp = str[len - i - 1];
        str[len - i - 1] = str[i];
        str[i] = temp;
    }
}

// =============================================
/// @brief UPPERCASE a string
/// @param[in] str: string
/// @return void
MEMSPACE
void
WEAK_ATR
strupper(char *str)
{

    while(*str)
    {
        *str = toupper(*str);
        ++str;
    }
}


// =============================================
///@brief Trim White space and control characters from end of string.
///
///@param[in] str: string
///
///@return void
///@warning Overwrites White space and control characters with EOS.
MEMSPACE
void trim_tail(char *str)
{
    int len = strlen(str);
    while(len--)
    {
        if(str[len] > ' ')
            break;
        str[len] = 0;
    }
}

// =============================================
///@brief Skip white space in a string - tabs and spaces.
///
///@param[in] ptr: input string
///
///@return pointer to first non white space character
MEMSPACE
char *skipspaces(char *ptr)
{
    if(!ptr)
        return(ptr);

    while(*ptr == ' ' || *ptr == '\t')
        ++ptr;
    return(ptr);
}

// =============================================
///@brief Skip to first white space in a string - tabs and spaces.
///
///@param[in] ptr: input string
///
///@return pointer to first white space character
MEMSPACE
char *nextspace(char *ptr)
{
    if(!ptr)
        return(ptr);

    while(*ptr)
    {
        if(*ptr == ' ' || *ptr == '\t')
            break;
        ++ptr;
    }
    return(ptr);
}

// =============================================
///@brief Skip characters defined in user string.
///
///@param[in] str: string
///@param[in] pat: pattern string
///
///@return pointer to string after skipped characters.
MEMSPACE
char *skipchars(char *str, char *pat)
{
    char *base;
    if(!str)
        return(str);

    while(*str)
    {
        base = pat;
        while(*base)
        {
            if(*base == *str)
                break;
            ++base;
        }
        if(*base != *str)
            return(str);
        ++str;
    }
    return(str);
}

// =============================================
// String Matching
// =============================================
///@brief Compare two strings
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return 0 on match, < 0 implies str sorts before pat, > 0 implies str sorts after pat
MEMSPACE
int
WEAK_ATR
strcmp(const char *str, const char *pat)
{
    int ret = 0;
    int c1,c2;
    while (1)
    {
        c1 = *str++;
        c2 = *pat++;
        if ( (ret = c1 - c2) != 0 || c2 == 0)
            break;
    }
    return(ret);
}

// =============================================
///@brief Compare two strings maximum len bytes in size
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///@param[in] len: maximum string length for compare
///@return 0 on match, < 0 implies str sorts before pat, > 0 implies str sorts after pat
MEMSPACE
int
WEAK_ATR
strncmp(const char *str, const char *pat, size_t len)
{
    int ret = 0;
    int c1,c2;
    while (len--)
    {
        c1 = *str++;
        c2 = *pat++;
        if ( (ret = c1 - c2) != 0 || c2 == 0)
            break;
    }
    return(ret);
}

// =============================================
///@brief Compare two strings without case
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return 0 on match, < 0 implies str sorts before pat, > 0 implies str sorts after pat
MEMSPACE
int
WEAK_ATR
strcasecmp(const char *str, const char *pat)
{
    int ret = 0;
    int c1,c2;
    while (1)
    {
        c1 = toupper(*str++);
        c2 = toupper(*pat++);
        if ( (ret = c1 - c2) != 0 || c2 == 0)
            break;
    }
    return(ret);
}
///@brief Compare two strings without case maximum len bytes in size
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///@param[in] len: maximum string length for compare
///@return 0 on match, < 0 implies str sorts before pat, > 0 implies str sorts after pat
MEMSPACE
int
WEAK_ATR
strncasecmp(const char *str, const char *pat, size_t len)
{
    int ret = 0;
    int c1,c2;
    while (len--)
    {
        c1 = toupper(*str++);
        c2 = toupper(*pat++);
        if ( (ret = c1 - c2) != 0 || c2 == 0)
            break;
    }
    return(ret);
}

// =============================================
///@brief Compare two strings.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
MEMSPACE
int MATCH(char *str, char *pat)
{
    int len;
    len = strlen(pat);
    if(strcmp(str,pat) == 0 )
        return(len);
    return(0);
}
///@brief Match two strings and compare argument index
/// Display  message if the number of arguments is too few
///@param str: string to test
///@param pat: pattern to match
///@param min: minumum number or arguments
///@param argc: actual number of arguments
///@return 1 on match, 0 on no match or too few arguments
MEMSPACE
int MATCHARGS(char *str, char *pat, int min, int argc)
{
    if(MATCH(str,pat))
    {
        if(argc >= min)
            return(1);
        else
            printf("%s expected %d arguments only got %d\n", pat, min,argc);
    }
    return(0);
}


// =============================================
///@brief Compare two strings without case.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
MEMSPACE
int MATCHI(char *str, char *pat)
{
    int len;
    len = strlen(pat);
    if(strcasecmp(str,pat) == 0 )
        return(len);
    return(0);
}
// =============================================
///@brief Compare two strings limted to length of pattern.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
///@warning Matches sub strings so be caeful.
MEMSPACE
int MATCH_LEN(char *str, char *pat)
{
    int len;

    if(!str || !pat)
        return(0);
    len = strlen(pat);

    if( len )
    {
        if(strncmp(str,pat,len) == 0 )
            return(len);
    }
    return(0);
}

// =============================================
///@brief Compare two strings without case limted to length of pattern.
///
///@param[in] str: string to match.
///@param[in] pat: pattern to compare.
///
///@return string lenth on match.
///@return 0 on no match.
///@warning Matches sub strings so be caeful.
MEMSPACE
int MATCHI_LEN(char *str, char *pat)
{
    int len;

    if(!str || !pat)
        return(0);
    len = strlen(pat);

    if( len )
    {
        if(strncasecmp(str,pat,len) == 0 )
            return(len);
    }
    return(0);
}

// =============================================
///@brief Split string into arguments stored in argv[]
///   We split source string into arguments
/// Warning: source string is modified!
///   To save memory each gap in the source string is terminated with an EOS
///   This becomes the end of string for each argument returned
/// Warning: Do NOT modify the source string or argument contents while using them
///   You can reassign new pointers to the arguments if you like
///@param[in|out] str: string to break up into arguments
///@param[out] *argv[]: token array
///@param[in] max: maximum argument count
///@return count
MEMSPACE
int split_args(char *str, char *argv[], int max)
{
	int i;
	int count = 0;
    // NULL ?

	for(i=1;i<max;++i)
		argv[i] = NULL;	

	// You may replace argv[0]
	argv[count++] = "main";

	if(!max)
		return(0);

    if(!str)
        return(0);

	while(*str && count < max)
	{
		str = skipspaces(str);
		if(!*str)
			break;

		// string processing
		if(*str == '"')
		{
			++str;
			// Save string pointer
			argv[count++] = str;
			while(*str && *str != '"')
				++str;
			if(*str == '"')
				*str++ = 0;
			continue;
		}

		argv[count++] = str;
		// Find size of token
		while(*str > ' ' && *str <= 0x7e)
			++str;
		if(!*str)
			break;
		*str  = 0;
		++str;
	}
	return(count);
}

// =============================================
///@brief return next token
///
/// - Skips all non printable ASCII characters before token
/// - Token returns only printable ASCII
///
///@param[in] str: string to search.
///@param[out] token: token to return
///@param[in] max: maximum token size
///
///@return pointer past token on success .
///@return NULL if no token found
MEMSPACE
char *get_token(char *str, char *token, int max)
{
    int len;

    *token = 0;

    // NULL ?
    if(!str)
        return(NULL);

    str = skipspaces(str);

    // Find size of token
    len = 0;
    while(*str > ' ' && *str <= 0x7e && len < max)
    {
        // clip token to max length
        if(len < max)
        {
            *token++ = *str++;
            ++len;
        }
    }
    *token = 0;
    // str points past the token
    if(!len)
        return(NULL);
    return(str);
}


// =============================================
///@brief Search for token in a string matching user pattern.
///
/// - Skips all non printable ASCII characters before trying match.
///
///@param[in] str: string to search.
///@param[in] pat: pattern to search for.
///
///@return string lenth on match.
///@return 0 on no match.

MEMSPACE
int token(char *str, char *pat)
{
    int patlen;
    int len;
    char *ptr;

    ptr = skipspaces(str);
    len = 0;
    while(*ptr > ' ' && *ptr <= 0x7e )
    {
        ++len;
        ++ptr;
    }

    if(!len)
        return(0);

    patlen = strlen(pat);

    if(len != patlen)
        return(0);

    if(strncmp(str,pat,patlen) == 0)
        return(len);
    return(0);
}
// =============================================

/// @brief get a number
///
/// - Used only for debugging
/// @param[in] str: string to examine
///
/// @return  value
MEMSPACE
int32_t get_value(char *str)
{ 
	int base;
	int ret;
	char *ptr;
	char *endptr;


	ptr = skipspaces(str);
	base = 10;

	// convert number base 10, 16, 8 and 2
	if( (ret = MATCHI_LEN(ptr,"0x")) )
	{
		base = 16;
		ptr += ret;
	}
	else if( (ret = MATCHI_LEN(ptr,"0o")) )
	{
		base = 8;
		ptr += ret;
	}
	else if( (ret = MATCHI_LEN(ptr,"0b")) )
	{
		base = 2;
		ptr += ret;
	}
	return(strtol(ptr, (char **)&endptr, base));
}

// =============================================
// String memory allocation functions
// =============================================
// =============================================
///@brief Allocate space for string with maximum size.
///
/// - Copies tring into allocated space limited to maximum size.
///
///@param[in] str: user string.
///@param[in] len: maximum string length.
///
///@return pointer to alocated string.

MEMSPACE
char *strnalloc(char *str, int len)
{
    char *ptr;

    if(!str)
        return(NULL);
    ptr = safecalloc(len+1,1);
    if(!ptr)
        return(ptr);
    strncpy(ptr,str,len);
    return(ptr);

}

// =============================================
///@brief Allocate space for string.
///
/// - Copies tring into allocated space.
///
///@param[in] str: user string.
///
///@return pointer to alocated string.
///@return NULL on out of memory.
MEMSPACE
char *stralloc(char *str)
{
    char *ptr;
    int len;

    if(!str)
        return(str);;
    len  = strlen(str);
    ptr = safecalloc(len+1,1);
    if(!ptr)
        return(ptr);
    strcpy(ptr,str);
    return(ptr);
}



