/**
 @file queue.h

 @brief Ring buffer code
 @par Copyright &copy; 2015 Mike Gore, GPL License
 @par You are free to use this code under the terms of GPL
  Please retain a copy of this notice in any code you use it in.

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


#ifndef _QUEUE_H_
#define _QUEUE_H_

/// @brief queue structure
typedef struct {
	char *buf;		/* Ring buffer */
	size_t in;		/* input offset */
	size_t out;		/* output offset */
	size_t bytes;	/* bytes used */
	size_t size;	/* Ring buffer size */
} queue_t;

/* queue.c */
queue_t *queue_new ( size_t size );
void queue_del ( queue_t *q );
void queue_flush ( queue_t *q );
size_t queue_used ( queue_t *q );
size_t queue_empty ( queue_t *q );
size_t queue_space ( queue_t *q );
size_t queue_full ( queue_t *q );
size_t queue_push_buffer ( queue_t *q , uint8_t *src , size_t size );
size_t queue_pop_buffer ( queue_t *q , uint8_t *dst , size_t size );
int queue_pushc ( queue_t *q , uint8_t c );
int queue_popc ( queue_t *q );

#endif

