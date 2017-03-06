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

/// @brief Obtain SPI bus for XPT2046, raises LE
/// return: void
MEMSPACE
void XPT2046_spi_init(void)
{
	XPT2046_clock = 16;
	chip_select_init(XPT2046_CS);
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


int iabs(int a)
{
	if(a < 0)
		return(-a);
	return(a);
}

/// @brief  Check for a touch event 
/// Average several samples
/// @param[in] *m: touch data structure
/// return: Z as non zero implies touch event

#define XPT2046_NOISE 8

int XPT2046_task(xpt2046_t *m)
{
	int i;

	int X,Y,Z1,Z2,Z;
	int Xsum,Ysum,Z1sum,Z2sum,Zsum;

	Xsum=0;
	Ysum=0;
	Z1sum=0;
	Z2sum=0;
	Zsum=0;

	// To reduce noise we compute the average of a number of samples
	// Must have at least XPT2046_SAMPLES for a valid test
	i = 0;
	do
	{
		X = XPT2046_read(XPT2046_READ_X);
		Y = XPT2046_read(XPT2046_READ_Y);
		Z1 = XPT2046_read(XPT2046_READ_Z1);
		Z2 = XPT2046_read(XPT2046_READ_Z2);

		// The touch voltage drop is a rough approximation
		// of the touch pressure
		Z = (4095 - Z2) + Z1;
		if(Z < 0)
			Z = -Z;
		if(Z < 200)
			break;
		Xsum += X;
		Ysum += Y;
		Z1sum += Z1;
		Z2sum += Z2;
		Zsum += Z;
	}
	while(++i < XPT2046_SAMPLES);

	// Do we have a full set of samples ?
	if(i == XPT2046_SAMPLES)
	{
		// compute average to reduce noise
		m->X = (Xsum / XPT2046_SAMPLES);
		m->Y = (Ysum / XPT2046_SAMPLES);
		m->Z1 = (Z1sum / XPT2046_SAMPLES);
		m->Z2 = (Z2sum / XPT2046_SAMPLES);
		Z = (Zsum / XPT2046_SAMPLES);
		if(Z < 0)
			Z = -Z;
	}
	else
	{
		m->X = 0;
		m->Y = 0;
		m->Z1 = 0;
		m->Z2 = 0;
		Z = 0;
	}
	// not used yet
	m->XR = 0;
	m->YR = 0;

	return(Z);
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
