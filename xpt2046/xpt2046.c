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

#include "display/ili9341.h"

/// =============================================================
/// =============================================================
/// HAL

extern window *tft;
extern window *winmsg;

#ifndef XPT2046_CS
#error You must define the XPT2046 GPIO pin
#endif

/// Start SPI Hardware Abstraction Layer
/// Keep all hardware dependent SPI code in this section

/// @brief cache of SPI clock devisor
uint32_t XPT2046_clock = -1;

///@breif touch event queue
xpt2046_t xpt2046;

///@brief ADC range or xpt2046
xpt2046_win_t xpt2046_device = {0, 0, 4095, 4095};

///@brief Example map range
xpt2046_win_t xpt2046_map = {0, 0, 4095, 4095};

///@brief measured ADC values for our ili9341 touch overlay - will be in the range 0 .. 4095
xpt2046_win_t xpt2046_ili9341_cal = {210, 184, 3366, 3250};

///@brief Map to range we want for the ili9341  display
xpt2046_win_t xpt2046_ili9341_map = {0, 0, 319, 239};

/// @brief Obtain SPI bus for XPT2046, raises LE
/// return: void
MEMSPACE
void XPT2046_spi_init(void)
{
	XPT2046_clock = 40;
	chip_select_init(XPT2046_CS);
	// reset calibration to NONE
	xpt2046.raw = xpt2046_ili9341_cal;
	// xpt2046.map = xpt2046_device; // maps to 0 .. 4095 - used to measure noise
	// FIXME get these values from display code so we do rotations
	xpt2046.map = xpt2046_ili9341_map;	// map to ili9341
	XPT2046_key_flush();
	xpt2046.rotation = tft->rotation;
}


/// @brief reset key press touch queue
MEMSPACE
void XPT2046_key_flush()
{
	xpt2046.state = 0;
	xpt2046.ms = 0;
	xpt2046.ind = 0;
	xpt2046.head = 0;
	xpt2046.tail = 0;
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

/// @brief  Check touch state and return the X,Y value if true
/// NO filtereing is done - use XPT2046_xy_filtered if you need filtering
/// @param[out] *X: X value 
/// @param[out] *Y: Y value 
/// return: Touch state 1 = touch, 0 = no touch 
int XPT2046_xy_raw(uint16_t *X, uint16_t *Y)
{
	int Z1,Z2,Z;

	Z1 = XPT2046_read(XPT2046_READ_Z1);
	Z2 = XPT2046_read(XPT2046_READ_Z2);
	// of the touch pressure
	Z = (4095 - Z2) + Z1;
	if(Z < 0)
		Z = -Z;
	if(Z > 300)
	{
		switch (tft->rotation)
		{
			case 0:
				// reverse X
				xpt2046.rotation = 0;
				*X = 4095 - XPT2046_read(XPT2046_READ_X);
				*Y = XPT2046_read(XPT2046_READ_Y);
				break;

			case 1:
				// swap X and Y
				xpt2046.rotation = 1;
				*X = XPT2046_read(XPT2046_READ_Y);
				*Y = XPT2046_read(XPT2046_READ_X);
				break;

			case 2:
				// reverse Y
				xpt2046.rotation = 2;
				*X = XPT2046_read(XPT2046_READ_X);
				*Y = 4095 - XPT2046_read(XPT2046_READ_Y);
				break;

			case 3:
				xpt2046.rotation = 3;
				// swap X and Y and reverse X and Y
				*X = 4095 - XPT2046_read(XPT2046_READ_Y);
				*Y = 4095 - XPT2046_read(XPT2046_READ_X);
				break;
		}
		return(1);	// 1 = touch event
	}
	return(0);		// no touch event
}


/// @brief  rotate map or raw limits to match screen rotation
/// @param[out] *out: rotated refernce frame
/// @param[out] *in: original reference frame
void XPT2046_rotate(xpt2046_win_t *out, xpt2046_win_t *in, int rotate)
{
    switch (rotate & 3)
    {
        case 0:
			// reverse X
			out->xmin = 4095 - in->xmax;
			out->xmax = 4095 - in->xmin;
			out->ymin = in->ymin;
			out->ymax = in->ymax;
            break;

        case 1:
			// swap X and Y
			out->xmin = in->ymin;
			out->xmax = in->ymax;
			out->ymin = in->xmin;
			out->ymax = in->xmax;
            break;

        case 2:
			// reverse Y
			out->xmin = in->xmin;
			out->xmax = in->xmax;
			out->ymin = 4095 - in->ymin;
			out->ymax = 4095 - in->ymax;
            break;

        case 3:
			// swap X and Y and reverse X and Y
			out->xmin = 4095 - in->ymin;
			out->xmax = 4095 - in->ymax;
			out->ymin = 4095 - in->xmin;
			out->ymax = 4095 - in->xmax;
            break;
	}
}



/// @brief  Apply calibration to X and Y results 
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 if values is in range or 0 on error
void XPT2046_map(uint16_t *X, uint16_t *Y)
{
	int XO,YO,XW,YW;
	int MX,MY;
	xpt2046_win_t raw,map;

	// rotate raw limits
	XPT2046_rotate((xpt2046_win_t *) &raw, (xpt2046_win_t *) &xpt2046.raw, tft->rotation);
	// rotate mapped limits limits
	XPT2046_rotate((xpt2046_win_t *) &map, (xpt2046_win_t *) &xpt2046.map, tft->rotation);

	// If we have a xpt2046 that reads 200,300 3600,3800 
	// We want to scale this to the range 0,0 4095,4095

	// Calibrate to 0 .. 4095;
	// XC = (X - 200) * 4096 / (3600.0 - 200.0);
	// YC = (Y - 300) * 4096 / (3800.0 - 300.0);

	// ====================================================================
	// MAP X 
	MX = (((raw.xmax-raw.xmin+1) * ((int)*X-map.xmin)) / (map.xmax-map.xmin+1)) 
		+ raw.xmin;
	// Map Y
	MY = (((raw.ymax-raw.ymin+1) * ((int)*Y-map.ymin)) / (map.ymax-map.ymin+1)) 
		+ raw.ymin;

	// Limits
	if(MX < raw.xmin)
		MX = raw.xmin;
	if(MX > raw.xmax)
		MX = raw.xmax;

	// Limits
	if(MY < raw.ymin)
		MY = raw.ymin;
	if(MY > raw.ymax)
		MY = raw.ymax;

	*X = (uint16_t) MX;
	*Y = (uint16_t) MY;
}

/// @brief  Check touch state and return the X,Y value if true
/// NO filtereing is done - use XPT2046_xy_filtered if you need filtering
/// @param[out] *X: X value 
/// @param[out] *Y: Y value 
/// return: Touch state 1 = touch, 0 = no touch 
MEMSPACE
int XPT2046_xy_raw_mapped(uint16_t *X, uint16_t *Y)
{
	int ret = XPT2046_xy_raw(X, Y);
	if(ret)
		XPT2046_map(X, Y);
	return(ret);
}
	

#if 0
/// @brief  Check Touch state - if touched then average a set of X and Y readings 
/// Check at most XPT2046_SAMPLES that are within noise limits
/// @param[out] *X: X position - ONLY if touched
/// @param[out] *Y: Y position - ONLY if touched
/// return: count of consecutive samples within noise limits - or 0 if not touched.
///         Ideally we want count to be at least >= XPT2046_SAMPLES/2 
MEMSPACE
int XPT2046_xy_filtered(uint16_t *X, uint16_t *Y)
{
	int XC,YC, XL,YL, XD, YD;
	long Xavg,Yavg;
	int count;		// Sample counter - to compute average 

	Xavg = 0; // X average
	Yavg = 0; // Y average

	for(count=0;count<XPT2046_SAMPLES;++count)
	{
		// X and Y if touched - otherwise 0,0
		if(!XPT2046_xy_raw(X, Y))
			break;
		XC = *X;
		YC = *Y;
		// Has the permitted peak to peak NOISE maximum been exceeded ?
		if(count)
		{
			XD = XC - XL;
			if(XD < 0)
				XD = -XD;
			YD = YC - YL;
			if(YD < 0)
				YD = -YD;
			if(XD > 20 || YD > 20)
				break;
		}

		// Update average
		Xavg += XC;
		Yavg += YC;
		XL = XC;
		YL = YC;
	}	

	// FIXME testing shows we need at least 2 samples in a row to be ok
	if(count)
	{
		Xavg /= count;
		Yavg /= count;
	}
	*X = (uint16_t) Xavg;
	*Y = (uint16_t) Yavg;
	return(count);
}
#else


/// @brief  Calculates best average of longest run 
/// Where longest run is calculated using best running average of absolute differences (noise)
/// @param[int] *v: integer array
/// @param[int] size: size of array
/// @param[int] minsamples: minimum number samples required for valid result
/// return: average of longest run or -1 if limits not met
int XPT2046_loop;
MEMSPACE
int nearest_run(int *v, int size, int minsamples, int *count)
{

	int i,j;
	int val;
	int diff,diffavg;
	int run;
	int sum;
	int noise;
	int min_run = 1;
	int average = 0;
	int min_noise = 65536;

	for(i=0;i<size;++i)
	{
		val = v[i];
		sum = val;
		run = 0;
		noise = 0;

		for(j=i+1;j<size;++j)
		{
			// compute differences
			diff = v[j] - val;
			if(diff < 0)
				diff = -diff;

			++run;

			// this is run+1 sample we have averaged
			sum += v[j];

			noise += diff;

			// FIXME we want average signal to noise ratio
			// always favor longer runs if better or equal noise
			if(run >= minsamples)
			{
				if((noise/run) <= min_noise/min_run)
				{
					min_noise = noise;;
					min_run = run;
					// we have run + 1 elements
					average = sum/(run+1);
				}
			}
		}
	}

	// we have run + 1 elements
	*count = min_run+1;
	if(min_run >= minsamples)
	{
		return(average);
	}
	return(-1);
}

/// @brief  Check Touch state - if touched then average a set of X and Y readings 
/// Check at most XPT2046_SAMPLES that are within noise limits
/// @param[out] *X: X position - ONLY if touched
/// @param[out] *Y: Y position - ONLY if touched
/// return: count of consecutive samples within noise limits - or 0 if not touched.
///         Ideally we want count to be at least >= XPT2046_SAMPLES/2 
MEMSPACE
int XPT2046_xy_filtered(uint16_t *X, uint16_t *Y)
{
	int XS[XPT2046_SAMPLES+1];
	int YS[XPT2046_SAMPLES+1];
	int Xavg,Yavg;
	int xcount,ycount;

	int i;
	for(i=0;i<XPT2046_SAMPLES;++i)
	{
		// X and Y if touched - otherwise 0,0
		if(!XPT2046_xy_raw(X, Y))
			break;
		XS[i] = *X;
		YS[i] = *Y;
	}

	Xavg = nearest_run((int *) &XS, i, 4, &xcount);
	Yavg = nearest_run((int *) &YS, i, 4, &ycount);

	if(Xavg >= 0 && Yavg >= 0)
	{	
		*X = (uint16_t) Xavg;
		*Y = (uint16_t) Yavg;
        printf("X:%4d, Y:%4d, XN:%2d, YN:%d\n",
            (int)Xavg, (int)Yavg, (int)xcount,(int)ycount);
		return(i);
    }
	return(0);
}


#endif

/// @brief  Check Touch state - if touched then average a set of X and Y readings 
/// Check at most XPT2046_SAMPLES that are within noise limits
/// @param[out] *X: X position - ONLY if touched
/// @param[out] *Y: Y position - ONLY if touched
/// return: count of consecutive samples within noise limits - or 0 if not touched.
///         Ideally we want count to be at least >= XPT2046_SAMPLES/2 
MEMSPACE
int XPT2046_xy_filtered_mapped(uint16_t *X, uint16_t *Y)
{
	if(XPT2046_xy_filtered(X, Y))
	{
		XPT2046_map(X,Y);	
        printf("XM:%4d, YM:%4d\n", (int)*X, (int)*Y);
		return(1);
	}
	return(0);
}

/// @brief  Test XPT2046_xy_filtered() to determine how long it takes a good read of X and Y within noise limits
/// Warning: This is only used for testing because it can block for up to 100ms
/// @param[out] *X: X position - ONLY if touched
/// @param[out] *Y: Y position - ONLY if touched
/// return: 1 if touch state is true, or 0
int XPT2046_loop;
MEMSPACE
int XPT2046_xy_filtered_test(uint16_t *X, uint16_t *Y)
{

	int T;
	XPT2046_loop = 0;
	while(1)
	{
		T = XPT2046_xy_filtered(X, Y);
		XPT2046_loop += T;
		if(!T)
			return(0);
		if(T >= XPT2046_SAMPLES/2)
			return(1);
	}
}


/// @brief Task to collect debounced KEY press style touch events
/// We treat the touch screen as a keyboard with debounce processing
/// return: void
MEMSPACE
void XPT2046_task()
{
	uint16_t X,Y;
	int T;

	T = XPT2046_xy_filtered((uint16_t *)&X, (uint16_t *)&Y);
	
	// Key debounce state machine
	switch(xpt2046.state) 
	{
		// Full init
		case 0:                 
			xpt2046.state = 1;
			xpt2046.ms = 0;
			xpt2046.ind = 0;
			xpt2046.head = 0;
			xpt2046.tail = 0;
			// Fall through to state 1
		case 1:
			// FIXME this can be a lower count for a button as we are not drawing here
			if(T)	// key is down and noise level is in range
			{
				// wait for key down debounce time before taking a sample
				if(++xpt2046.ms < XPT2046_DEBOUNCE) 	
					break;

				if(xpt2046.ind < XPT2046_EVENTS )
				{
					xpt2046.XQ[xpt2046.head] = X;
					xpt2046.YQ[xpt2046.head] = Y;
					if(++xpt2046.head >= XPT2046_EVENTS)
						xpt2046.head = 0;
					xpt2046.ind++;
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
			if(T == 0)
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


/// @brief  return uncalibrated key press
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 on touch event in queue
MEMSPACE
int XPT2046_key_unmapped(uint16_t *X, uint16_t *Y)
{
	XPT2046_task();

	if(xpt2046.ind > 0)
	{
		*X = (uint16_t) xpt2046.XQ[xpt2046.tail];
		*Y = (uint16_t) xpt2046.YQ[xpt2046.tail];
		if(++xpt2046.tail >= XPT2046_EVENTS)
			xpt2046.tail = 0;
		xpt2046.ind--;
		return(1);
	}
	*X = 0;
	*Y = 0;
	return(0);
}



// ===========================================================================
/// #brief Calibration code

/// @brief  Set raw calibration values
/// @param[in] XL: lowest X
/// @param[in] YL: lowest Y
/// @param[in] XM: highest X
/// @param[in] YM: highest Y
/// return: void
MEMSPACE
void XPT2046_setcal(uint16_t xmin, uint16_t ymin, uint16_t xmax, uint16_t ymax)
{
	xpt2046.raw.xmin = xmin;
	xpt2046.raw.ymin = ymin;
	xpt2046.raw.xmax = xmax;
	xpt2046.raw.ymax = ymax;
}

/// @brief  Set range to map to
/// @param[in] XL: lowest X
/// @param[in] YL: lowest Y
/// @param[in] XM: highest X
/// @param[in] YM: highest Y
/// return: void
MEMSPACE
void XPT2046_setmap(uint16_t xmin, uint16_t ymin, uint16_t xmax, uint16_t ymax)
{
	xpt2046.map.xmin = xmin;
	xpt2046.map.ymin = ymin;
	xpt2046.map.xmax = xmax;
	xpt2046.map.ymax = ymax;
}


/// @brief  Read current calibrated averaged X and Y state
/// We want T to be more then 2 - ideally XPT2046_SAMPLES/2 for low noise measurements of X and Y
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 if touch state is true
MEMSPACE
int XPT2046_xy_mapped(uint16_t *X, uint16_t *Y)
{
	int T;
	T = XPT2046_xy_filtered(X, Y);
	XPT2046_map(X,Y);
	return(T);
}
	

/// @brief  return calibrated KEY press touch position 
/// We want T to be more then 2 - ideally XPT2046_SAMPLES/2 for low noise measurements of X and Y
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 on touch event in queue
MEMSPACE
int XPT2046_key(uint16_t *X, uint16_t *Y)
{

	int T;
	T = XPT2046_key_unmapped(X,Y);
	XPT2046_map(X,Y);
	return(T);
}
// ===========================================================================

// 
#ifdef ESP8266
int __errno;
#endif

///@brief For noise filtering we can use the standard deviation of a set of samples
///       Then we can reject samples outside of say +/- 1 standard deviation in the set
///       Sample size MUST be 2 or greater, this is by definition of the term standard deviation
/// @param[in] *sample: array of uint16_t samples
/// @param[in] size:    number of samples
/// @param[out] *Z:      sdev_t * structure pointer to computer results into
/// @return 1 on success, 0 on parameter or calculation error
MEMSPACE
int sdev(uint16_t *samples, int size, sdev_t *Z) 
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

#endif // XPT2046
