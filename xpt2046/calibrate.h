/**
 @file     calibrate.h
 @version V0.10
 @date     26 mar 2017
 
 @brief XPT2046 calibration code

 @par Copyright &copy; 2017 Mike Gore, GPL License
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
#ifndef _CALIBRATE_H_
#define _CALIBRATE_H_

/* calibrate.c */
MEMSPACE int tft_check_calibrated ( window *win );
MEMSPACE int tft_touch_calibrate ( window *win );
MEMSPACE int tft_touch_map ( window *win , int16_t *X , int16_t *Y );
MEMSPACE int tft_map_test ( window *win , int points );
MEMSPACE int tft_touch_xy_raw ( window *win , uint16_t *X , uint16_t *Y );
MEMSPACE int tft_touch_xy ( window *win , uint16_t *X , uint16_t *Y );
MEMSPACE int tft_touch_key ( window *win , uint16_t *X , uint16_t *Y );

#endif // _CALIBRATE_H_



