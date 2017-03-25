/**
  @file sort.h

 @brief Integer sort

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

#ifndef _SORT_H_
#define _SORT_H_

/* sort.c */
MEMSPACE void heapify ( int *v , int size , int root );
MEMSPACE void heapsort ( int *v , int size );
MEMSPACE void insert_sort ( uint16_t *v , int size );

#endif // _SORT_H_
