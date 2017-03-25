/**
  @file heapsort.c

 @brief Integer heapsort

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

#include "sort.h"

 
MEMSPACE
void heapify(int *v, int size, int root)
{
    int largest = root;
    int left  = (root<<1) + 1;  
    int right = (root<<1) + 2; 
	int tmp;
 
    // If left child is larger than root
    if (left < size && v[left] > v[largest])
        largest = left;
 
    // If right child is larger than largest so far
    if (right < size && v[right] > v[largest])
        largest = right;
 
    // If largest is not root
    if (largest != root)
    {
        // swap
		tmp = v[root];
		v[root] = v[largest];
		v[largest] = tmp;
        heapify(v, size, largest);
    }
}
 
MEMSPACE
void heapsort(int *v, int size)
{
	int root;
	int tmp;

    for(root = size / 2 - 1; root >= 0; --root)
	{
        heapify(v, size, root);
	}
 
    // One by one extract an element from heap
    for (root=size-1; root>=0; --root)
    {
        // swap
		tmp = v[0];
		v[0] = v[root];
		v[root] = tmp;
        heapify(v, root, 0);
    }
}

MEMSPACE
void insert_sort(uint16_t *v, int size) 
{
  int i,j;
  uint16_t tmp;
  
  for (i = 1; i < size; ++i) 
  {
    tmp = v[i];
    for (j = i; j >= 1 && tmp < v[j - 1]; --j)
      v[j] = v[j - 1];
    v[j] = tmp; 
  }
}
