/**
 @file     xpd2046.h
 @version V0.10
 @date     22 Sept 2016
 
 @brief XPT2046 drivers

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
#ifndef _XPT2046_H_
#define _XPT2046_H_

///@brief number of time to read and average results
#define XPT2046_SAMPLES 8 	/* make this a power of 2 */
#define XPT2046_DEBOUNCE 10 /* Debound value in mS */

///@brief only need 4 commands for reading position or touch information
#define XPT2046_READ_Y  0x91    /* Read Y position*/
#define XPT2046_READ_Z1 0xb1	/* Read Z1 */
#define XPT2046_READ_Z2 0xc1    /* read Z2 */
#define XPT2046_READ_X  0xd1	/* Read X position */

typedef struct _xpt2046 {
	int Z1;	// Z1 driving XN and YP, reading XP
	int Z2;	// Z2 driving XN and YP, reading YN
	int X;	// X Position, driving XP and YN, reading YP
	int Y;	// Y Position, driving YP and YN, reading XP
    int state;  // Debounce state machine
    int ms;     // Debounce 1mS timer
	int key_X;	// Debounced X
	int key_Y;	// Debounced Y
} xpt2046_t;

typedef struct _sdev {
    uint16_t Min,Max;
    float mean, sdev;
} sdev_t;

/* xpt2046.c */
MEMSPACE void XPT2046_spi_init ( void );
uint16_t XPT2046_read ( uint8_t cmd );
int XPT2046_task ( xpt2046_t *m );
int XPT2046_key ( xpt2046_t *m , int *X , int *Y );

#endif // _XPT2046_H_



