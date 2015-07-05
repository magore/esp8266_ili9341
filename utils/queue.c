/**
 @file queue.c

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


#include "user_config.h"
/**
  @brief Create a ring buffer of a given size
  @param[in] size: size of rin buffer
  @return popinter to ring buffer structure
*/
queue_t *queue_new(size_t size)
{
	queue_t *q = calloc( sizeof(queue_t),1);
	if(!q)
		return(NULL);
	q->buf = calloc(size+1,1);
	if(!q->buf)
	{
		free(q);
		return(NULL);
	}
	q->in = 0;
	q->out = 0;
	q->bytes = 0;
	q->size = size;
	return(q);
}

/**
  @brief Delete a ring buffer and free memory
  @param[in] *q: ring buffer pointer
  @return void 
*/
void queue_del(queue_t *q)
{
	if(!q)
		return;
	if(q->buf)
	{
		free(q->buf);
		// This clear help prevents a freed pointer from being used by mistake
		// can be removed in production
		q->buf = NULL;
		q->in = 0;
		q->out = 0;
		q->bytes = 0;
		q->size = 0;
	}
	free(q);
}


/**
  @brief Flush ring buffer
  @param[in] *q: ring buffer pointer
  @return void 
*/
void queue_flush(queue_t *q)
{
	q->in = 0;
	q->out = 0;
	q->bytes = 0;
}

/**
  @brief Find the number of bytes used by the ring buffer
  @param[in] *q: ring buffer pointer
  @return the number of bytes used in the ring buffer
*/
size_t queue_used(queue_t *q)
{
	if(!q || !q->buf)
		return(0);
	return(q->bytes);
}

/**
  @brief Is the ring buffer empty ?
  @param[in] *q: ring buffer pointer
  @return 1 if empty, 0 otherwise
*/
size_t queue_empty(queue_t *q)
{
	if(!q || !q->buf)
		return(1);
	if(!q->bytes)
		return(1);
	return(0);
}

/**
  @brief Find the amount of free space remaining in the ring buffer 
  @param[in] *q: ring buffer pointer
  @return bytes remining in ring buffer 
*/
size_t queue_space(queue_t *q)
{
	if(!q || !q->buf)
		return(0);
	return(q->size - q->bytes);
}

/**
  @brief Is the ring buffer full ?
  @param[in] *q: ring buffer pointer
  @return 1 if full, 0 otherwise
*/
size_t queue_full(queue_t *q)
{
	if(!q || !q->buf)
		return(0);
	return((q->size - q->bytes) ? 0 : 1);
}

/**
  @brief Add a data buffer to the ring buffer
	 Note: This function does not wait/block util there is enough free space 
	 to meet the request.
	 So you must check that the return value matches the size.
  @param[in] *q: ring buffer pointer
  @param[in] *src: input buffer
  @param[in] size: size of input buffer
  @return number of bytes actually added to the buffer - may not be size!
*/
size_t queue_push_buffer(queue_t *q, uint8_t *src, size_t size)
{
    size_t bytes = 0;
	int free;

    if(!q || !q->buf)
        return(0);

	while(size && (q->size - q->bytes) > 0)
	{
		q->buf[q->in++] = *src++;
		if(q->in >= q->size)
			q->in = 0;
		++q->bytes;
		--size;
		++bytes;
	}
	return(bytes);
}

/**
  @brief Get a data buffer from the ring buffer.
	 Note: This function does not wait/block until there is enough data
	 to fill the request.
	 So you must check that the return value matches the size.
  @param[in] *q: ring buffer pointer
  @param[in] *src: input buffer
  @param[in] size: size of input buffer
  @return number of bytes actually added to the buffer - may not be size!
*/
size_t queue_pop_buffer(queue_t *q, uint8_t *dst, size_t size)
{
	size_t bytes = 0;

	if(!q || !q->buf)
		return(0);

	while(size && q->bytes)
	{
		*dst++ = q->buf[q->out++];
		if(q->out >= q->size)
			q->out = 0;
		--q->bytes;
		--size;
		++bytes;
	}
	return(bytes);
}


/**
  @brief Add a byte to the ring buffer
	 Note: This function does not wait/block util there is enough free space 
	 to meet the request.
	 We assume you check queue_full() before calling this function!
	 Otherwise you must check that the return value matches 1
  @param[in] *q: ring buffer pointer
  @param[in] c: vyte to add
  @return number of bytes actually added to the buffer - may not be 1!
*/
int queue_pushc(queue_t *q, uint8_t c)
{
	if(!q || !q->buf)
		return(0);

	if(q->bytes >= q->size)
	{
		return(0);
	}

	q->buf[q->in] = c;
	if(++q->in >= q->size)
		q->in = 0;
	++q->bytes;
	return(1);
}

/**
  @brief Remove a byte from the ring buffer
	Note: This function does not wait/block util there is data to 
	meet the request.
	We assume you check queue_empty() before calling this function!
  @param[in] *q: ring buffer pointer
  @return byte , or 0 if ring buffer was empty (user error)
*/
int queue_popc(queue_t *q)
{
	uint8_t c;
	if(!q || !q->buf)
		return(0);

	if(q->bytes)
	{
		c = q->buf[q->out];
		if(++q->out >= q->size)
			q->out = 0;
		q->bytes--;
		return(c);
	}
	return(0);
}
