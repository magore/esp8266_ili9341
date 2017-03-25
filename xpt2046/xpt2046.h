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
#define XPT2046_SAMPLES 12 	/* number of samples to take */
#define XPT2046_DEBOUNCE 10 /* Debound value in mS */
#define XPT2046_EVENTS 10 /* Number of queued touch events */

///@brief only need 4 commands for reading position or touch information
#define XPT2046_READ_Y  0x91    /* Read Y position*/
#define XPT2046_READ_Z1 0xb1	/* Read Z1 */
#define XPT2046_READ_Z2 0xc1    /* read Z2 */
#define XPT2046_READ_X  0xd1	/* Read X position */


/// @brief  initial calibration values for your display
/// Note: these values are not rotated 
/// - rotations are applied dynamically based on the display state and data structures
typedef struct xpt2046_win
{
	int xmin;
	int ymin;
	int xmax;
	int ymax;
} xpt2046_win_t;

typedef struct _xpt2046 
{
	// touch debounce state machine
    int state;  // Debounce state machine
    int ms;     // Debounce 1mS timer

	// rotation of touch screen
	int rotation;

	// raw uncorrected values
	xpt2046_win_t raw;

	// map calibration to this range
	xpt2046_win_t map;

	// touch input queue
    int ind;	// touch events
    int head;	// head of touch event queue
    int tail;	// tail of touch event queue
	uint16_t XQ[XPT2046_EVENTS+1];	// Debounced X result
	uint16_t YQ[XPT2046_EVENTS+1];	// Debounced Y result
} xpt2046_t;

typedef struct _sdev {
    uint16_t min,max;
    float mean, sdev;
} sdev_t;

/* xpt2046.c */
MEMSPACE void XPT2046_spi_init ( void );
MEMSPACE void XPT2046_key_flush ( void );
uint16_t XPT2046_read ( uint8_t cmd );
int XPT2046_xy_raw ( uint16_t *X , uint16_t *Y );
void XPT2046_rotate ( xpt2046_win_t *out , xpt2046_win_t *in , int rotate );
void XPT2046_map ( uint16_t *X , uint16_t *Y );
MEMSPACE int XPT2046_xy_raw_mapped ( uint16_t *X , uint16_t *Y );
MEMSPACE int XPT2046_xy_filtered ( uint16_t *X , uint16_t *Y );
MEMSPACE int nearest_run ( int *v , int size , int minsamples , int *count );
MEMSPACE int XPT2046_xy_filtered ( uint16_t *X , uint16_t *Y );
MEMSPACE int XPT2046_xy_filtered_mapped ( uint16_t *X , uint16_t *Y );
MEMSPACE int XPT2046_xy_filtered_test ( uint16_t *X , uint16_t *Y );
MEMSPACE void XPT2046_task ( void );
MEMSPACE int XPT2046_key_unmapped ( uint16_t *X , uint16_t *Y );
MEMSPACE void XPT2046_setcal ( uint16_t xmin , uint16_t ymin , uint16_t xmax , uint16_t ymax );
MEMSPACE void XPT2046_setmap ( uint16_t xmin , uint16_t ymin , uint16_t xmax , uint16_t ymax );
MEMSPACE int XPT2046_xy_mapped ( uint16_t *X , uint16_t *Y );
MEMSPACE int XPT2046_key ( uint16_t *X , uint16_t *Y );
MEMSPACE int sdev ( uint16_t *samples , int size , sdev_t *Z );

#endif // _XPT2046_H_



