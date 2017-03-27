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

#include "matrix.h"

#ifdef DISPLAY

///TODO this is very little XPT2046 specific code in this file
/// move specific code to to a HAL interface
///FYI: this code does not assume anything about the touch scale resolution
#ifndef XPT2046
#error XPT2046 is not defined
#endif

#include "xpt2046.h"
#include "display/ili9341.h"

///TODO save to SD card, flash or EEROM
mat_t tft_calX;
mat_t tft_calY;

///@brief has calibration been doen yet ?
int tft_is_calibrated = 0;


/** 
  @brief  Test if screen is calibrated
  @param[in] *win: window structure
  return: 1 if calibarted
*/
MEMSPACE
int tft_check_calibrated(window *win)
{
	if(!tft_is_calibrated)
	{
		tft_touch_calibrate(win);
		return(0);
	}
	return(1);

}

/** 
  @brief  Run screen calibration on window
          Most useful when done on the master winodow
  NOTE: Caller must redraw all windows after calling this function
  @param[in] *win: window structure
  return: 1 if calibarted
*/
MEMSPACE
int tft_touch_calibrate(window *win)
{
	int i;
	uint16_t w,h,X1,X2,Y1,Y2;
	mat_t MatAI;
	mat_t MatX = MatAlloc(5,1);
	mat_t MatY = MatAlloc(5,1);
	mat_t MatA = MatAlloc(5,3);

	w = win->w;
	h = win->h;

	if(tft_is_calibrated)
	{
		MatFree(tft_calX);
		MatFree(tft_calY);
		tft_is_calibrated = 0;
	}

	tft_fillWin(win, win->bg);
	if(win->rotation & 1)
		tft_set_font(win, 2);
	else
		tft_set_font(win, 1);
	tft_printf("Please Calibrate");


	MatX.data[0][0] = w / 4;
	MatY.data[0][0] = h / 4;

	MatX.data[1][0] = w * 3 / 4;
	MatY.data[1][0] = h / 4;

	MatX.data[2][0] = w / 4;
	MatY.data[2][0] = h * 3 / 4;

	MatX.data[3][0] = w * 3 / 4;
	MatY.data[3][0] = h * 3 / 4;

	MatX.data[4][0] = w / 2;
	MatY.data[4][0] = h / 2;

#if MATDEBUG & 2
	printf("X\n");
	MatPrint(MatX);
	printf("Y\n");
	MatPrint(MatY);
#endif

	for(i=0;i<5;++i)
	{
		X1 = MatX.data[i][0];
		Y1 = MatY.data[i][0];
		tft_fillCircle (win , X1,Y1, 5 , ILI9341_WHITE);
		tft_set_textpos(win, 0,0);
		if(win->rotation & 1)
			tft_set_font(win, 1);
		else
			tft_set_font(win, 0);

		tft_printf(win,"touch point %3d,%3d", (int)X1, (int)Y1);
		tft_cleareol(win);

		// This is the only XPT2046 specific bit of code in the file
		while( XPT2046_key((uint16_t *)&X2, (uint16_t *)&Y2) == 0 )
			optimistic_yield(1000);

		MatA.data[i][0] = (float)X2;
		MatA.data[i][1] = (float)Y2;
		MatA.data[i][2] = 1.0;
		// reset pixel
		tft_fillCircle (win , X1,Y1, 5 , win->bg);
		tft_drawPixel(win, X1, Y1, win->bg);
	}


	/// FYI: Least squares result is automatic result of PseudoInvert for overdetermined matrix
	/// FIXME! if the determinant is 0 we want to return 0
	MatAI = PseudoInvert(MatA);

	// Calibration values
	tft_calX = MatMul(MatAI,MatX);
	tft_calY = MatMul(MatAI,MatY);

#if MATDEBUG & 2
	printf("A\n");
	MatPrint(MatA);

	printf("Invert\n");
	MatPrint(MatAI);

	printf("tft_calX\n");
	MatPrint(tft_calX);

	printf("tft_calY\n");
	MatPrint(tft_calY);
#endif

	MatFree(MatA);
	MatFree(MatX);
	MatFree(MatY);
	MatFree(MatAI);

	return( (tft_is_calibrated = 1) );
}

/** 
  @brief  Clip mapped value to window limits
  @param[in] *win: window structure
  @param[in] *X: X position in window
  @param[in] *Y: Y position is window
  return: 1 if calibrated
*/
MEMSPACE
int tft_touch_map(window *win, int16_t *X, int16_t *Y)
{
	int16_t X2,Y2;
	float XF,YF;

	/// TODO tft_check_calibrated need to check for pseudoinvert error states
	if(!tft_check_calibrated(win))
	{
		return(0);
	}

	XF = (float)*X;
	YF = (float)*Y;
#if MATDEBUG & 2
	printf("tft_touch_map: raw: X:%.0f,Y:%.0f\n", (double)XF, (double)YF);
#endif
	X2 = (uint16_t)(tft_calX.data[0][0] * XF + tft_calX.data[1][0] * YF + tft_calX.data[2][0]);
	Y2 = (uint16_t)(tft_calY.data[0][0] * XF + tft_calY.data[1][0] * YF + tft_calY.data[2][0]);
#if MATDEBUG & 2
	printf("tft_touch_map: cal: X:%3d,Y:%3d\n", (int)X2, (int)Y2);
#endif

	// force result to fix in window limits
	tft_clip_xy(win,X,Y);
		
	*X = X2;
	*Y = Y2;
	return(1);
}


/** 
  @brief  Test setting specified number of screen points
  The user can see visually if the point doest not match the displayed point
  @param[in] *win: window structure
  @param[in] points: pints to test
*/
MEMSPACE
int tft_map_test(window *win, int points)
{
	int i;
	int16_t X1,Y1;

	tft_fillWin(win, win->bg);
	if(win->rotation & 1)
		tft_set_font(win, 1);
	else
		tft_set_font(win, 0);
	tft_printf(win,"press %d points", points);

	/// TODO tft_check_calibrated need to check for pseudoinvert error states
	if(!tft_check_calibrated(win))
	{
		return(0);
	}

	for(i=0;i<points;++i)
	{
		while( XPT2046_key((uint16_t *)&X1, (uint16_t *)&Y1) == 0 )
			optimistic_yield(1000);

		tft_touch_map(win,&X1,&Y1);
		tft_fillCircle (win , X1,Y1, 5 , ILI9341_WHITE);
	}
	return(1);
}

/// @brief  Read calibrated unfilterd X and Y values
/// @param[in] win*: TFT Window
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 if touch state is true
MEMSPACE
int tft_touch_xy_raw(window *win, uint16_t *X, uint16_t *Y)
{
	int T;
	T = XPT2046_xy_raw(X, Y);
	tft_touch_map(win,X,Y);
	return(T);
}

/// @brief  Read calibrated filtered X and Y values
/// We want T to be more then 2 - ideally XPT2046_SAMPLES/2 for low noise measurements of X and Y
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 if touch state is true
MEMSPACE
int tft_touch_xy(window *win, uint16_t *X, uint16_t *Y)
{
	int T;
	T = XPT2046_xy_filtered(X, Y);
	tft_touch_map(win,X,Y);
	return(T);
}
	

/// @brief  Read calibrated KEY press touch position 
/// We want T to be more then 2 - ideally XPT2046_SAMPLES/2 for low noise measurements of X and Y
/// @param[in] *X: X position
/// @param[in] *Y: Y position
/// return: 1 on touch event in queue
MEMSPACE
int tft_touch_key(window *win, uint16_t *X, uint16_t *Y)
{

	int T;
	T = XPT2046_key(X,Y);
	tft_touch_map(win,X,Y);
	return(T);
}
#endif	//DISPLAY
