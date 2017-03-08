/**
 @file     xpt2046.c
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

#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>

#include "user_config.h"

#ifdef XPT2046
#include "xpt2046.h"


/// =============================================================
/// =============================================================
/// HAL

#ifndef XPT2046_CS
#error You must define the XPT2046 GPIO pin
#endif

///@brief z press status
int zstat = 0;

/// Start SPI Hardware Abstraction Layer
/// Keep all hardware dependent SPI code in this section

/// @brief cache of SPI clock devisor
uint32_t XPT2046_clock = -1;

///@breif touch event queue
static xpt2046_t xpt2046;

/// @brief Obtain SPI bus for XPT2046, raises LE
/// return: void
MEMSPACE
void XPT2046_spi_init(void)
{
	XPT2046_clock = 16;
	chip_select_init(XPT2046_CS);
	XPT2046_key_flush();
}


/// @brief  reset touch queue
void XPT2046_key_flush()
{
	xpt2046.state = 0;
	xpt2046.ms = 0;
	xpt2046.count = 0;
	xpt2046.head = 0;
	xpt2046.tail = 0;
}

/// @brief  Read a touch event
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 on touch event in queue
int XPT2046_key(uint16_t *X, uint16_t *Y)
{
	XPT2046_task();

	if(xpt2046.count > 0)
	{
		*X = (uint16_t) xpt2046.XQ[xpt2046.tail];
		*Y = (uint16_t) xpt2046.YQ[xpt2046.tail];
		if(++xpt2046.tail >= XPT2046_EVENTS)
			xpt2046.tail = 0;
		xpt2046.count--;
		return(1);
	}
	return(0);
}


/**
Command byte mode bits used in this project

Lower Command Nibble (4 bits) for this project
Note:
     /DFR = 0 is differential mode
         Ratiometric conversion - has lowest noise for ADC readings
     PD1 = 0 turns off reference, required for this mode (/DFR=0)
     PD0 = 1 enable ADC
     PD0 = 0 power down device
     MODE = 0 12 bits
     Summary for lower nibble, we only use:
         MODE SER//DFR PD1 PD0
            0        0   0   0	  12bit, /DFR, REF off, ADC off
            0        0   0   1     12bit, /DFR, REF off, ADC on

Upper Command Nibble (4 bits) for this project
Note:
     S = 1 Command byte
     S = 0 read data used to clock out data after first command byte
     Summary for upper nibble, we only use
         S  A2 A1 A0     ADC+IN    MEASUREMENT    DRIVERS
         1   0  0  0      TEMP0
         1   0  0  1         XP          Y-POS      YN YP
         1   0  1  0       VBAT
         1   0  1  1         XP         Z1-POS      XN YP
         1   1  0  0         YN         Z2-POS      XN YP
         1   1  0  1         YP          X-POS      XN XP
         1   1  1  0      AUXIN
         1   1  1  1       TEMP

    We only need 4 commands for reading position or touch information
        0x91 Read Y position
        0xb1 Read Z1
        0xc1 Read Z2
        0xd1 Read X position
*/

/// @brief  Send command and read ADC result
/// @param[in] cmd: command byte
/// return: ADC value
uint16_t XPT2046_read(uint8_t cmd)
{
	uint8_t  buf[3];
	uint16_t val;
	
	// First byte to send is the command
    buf[0] = cmd;
	// 0 is ignored as a command NOOP, used here to read the reply from the ADC
    buf[1] = 0;
	buf[2] = 0;

    spi_begin(XPT2046_clock, XPT2046_CS);
	// Send three bytes and read the result
    spi_TXRX_buffer(buf,3);   
	spi_end(XPT2046_CS);

	// buf[0] - Ignore data read back from the first command byte - is has no valid data

	// Extract the ADC 12 bit reply - ADC result starts one bit AFTER the MSB bit position
	val = buf[1];	// MSB
	val <<= 8;
	val |= buf[2];	// LSB

	// Align 12 bit result to LSB bit position
	val >>= 3;		

	if(val < 0)
		val = 0;
	if(val >4095)
		val = 4095;
    return(val);
}

/// @brief  Read current averaged X andd Y state
/// If  touch state changes while averaging we restart
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 if touch state is true
int XPT2046_read_filtered(uint16_t *X, uint16_t *Y)
{
	int Z1,Z2,Z;
	long Xavg = 0;		// Average of count X samples
	long Yavg = 0;		// Average of count Y samples
	int T = 0;			// Current touch state, 1 of touch, 0 if not
	int TL = 2;			// Last touch state - set to invalid
	int count = 0;		// Sample counter - to compute average 

	do
	{
		Z1 = XPT2046_read(XPT2046_READ_Z1);
		Z2 = XPT2046_read(XPT2046_READ_Z2);
		// of the touch pressure
		Z = (4095 - Z2) + Z1;
		if(Z < 0)
			Z = -Z;
		T = (Z > 200) ? 1 : 0;
		if(T != TL) 
		{
			count = 0;
			TL = T;
			continue;
		}
		Xavg += XPT2046_read(XPT2046_READ_X);
		Yavg += XPT2046_read(XPT2046_READ_Y);
		TL = T;
	}	while(++count < XPT2046_SAMPLES);
	Xavg /= XPT2046_SAMPLES;
	Yavg /= XPT2046_SAMPLES;
	*X = (uint16_t) Xavg;
	*Y = (uint16_t) Yavg;
	return(T);	// 1 = touch event, 0 is no touch event
}


/// @brief Task to collect debounced touch screen key press events 
/// return: void
void XPT2046_task()
{
	int i;
	uint16_t X,Y;
	int T;
	long Xavg, Yavg;

	T = XPT2046_read_filtered((uint16_t *)&X, (uint16_t *)&Y);
	
	// Key debounce state machine
	switch(xpt2046.state) 
	{
		// Full init
		case 0:                 
			xpt2046.state = 1;
			xpt2046.ms = 0;
			xpt2046.count = 0;
			xpt2046.head = 0;
			xpt2046.tail = 0;
			// Fall through to state 1
		case 1:
			if(T)	// key is down
			{
				// wait for key down debounce time before taking a sample
				if(++xpt2046.ms < XPT2046_DEBOUNCE) 	
					break;

				if(xpt2046.count < XPT2046_EVENTS )
				{
					xpt2046.XQ[xpt2046.head] = X;
					xpt2046.YQ[xpt2046.head] = Y;
					if(++xpt2046.head >= XPT2046_EVENTS)
						xpt2046.head = 0;
					xpt2046.count++;
					xpt2046.state = 2;
					xpt2046.ms = 0;
				}
				else
				{
					xpt2046.ms = 0;
				}
			}	// if (T)
			else	// Restart timer and wait for touch debounce time
			{
				xpt2046.ms = 0;
			}
			break;

			// Wait for touch release debounce time
        case 2:                 
			// Released ?
			if(!T)
			{
				// Debounce release time - valid key depress cycle done
				if(++xpt2046.ms >= XPT2046_DEBOUNCE) 
				{
					xpt2046.ms = 0;
					xpt2046.state = 1;
				}
			}
			else	// Still depressed
			{
				xpt2046.ms = 0;
			}
			break;


			// Error INIT
		default:
			xpt2046.state = 0;
			break;
	}
}

///TODO add filtering
#if 0
///@brief For noise filtering we can use the standard deviation of a set of samples
///       Then we can reject samples outside of say +/- 1 standard deviation in the set
///       Sample size MUST be 2 or greater, this is by definition of the term standard deviation
/// @param[in] *sample: array of uint16_t samples
/// @param[in] size:    number of samples
/// @param[in] min:     lowest sample value permitted
/// @param[in] max:     highest sample value permitted
/// @param[out] *Z:      sdev_t * structure pointer to computer results into
/// @return 1 on success, 0 on parameter or calculation error
int sdev(uint16_t *samples, int size, uint16_t min, uint16_t max, sdev_t *Z) 
{
int i;
float val, delta, sum, sqsum;

Z->min = ~0;
Z->max = 0;

// error in sample size ?
	if(size < 2)
		return(0);

	sum = 0.0;
	for(i=0; i<size ; i++) {
		val = samples[i];
		if(val < 0 || val > 4095)
			continue;	// reject
		if(Z->min > val)
			Z->min = val;
		if(Z->max < val)
			Z->max = val;
		sum += val;
	}
	Z->mean = sum / (float) size;
	sqsum = 0.0;
	for(i=0;i<size;++i) {
		val = samples[i];
		if(val < 0 || val > 4095)
			continue;	// reject
		delta = val - Z->mean;
		sqsum += (delta * delta);
	}
	val = sqsum / (float) (i - 1);
	Z->sdev = sqrt(val);
	return(1);
}

#endif

#endif // XPT2046
