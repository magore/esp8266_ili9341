#ifdef VFONTS 
/**
 @file vfont.c

 @brief Vector Font display for ili9341 driver

 @par Copyright &copy; 2017 Mike Gore, GPL License
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


#include <stdint.h>
#include <stdarg.h>
#include <string.h>
#include <math.h>


#ifdef USER_CONFIG
#include "user_config.h"
#endif

/// @brief save fonts in flash
#ifndef MEMSPACE
#define MEMSPACE_FONT MEMSPACE
#endif

#include "display/ili9341.h"

#include "display/vfont.h"
#include "display/vfonts.h"

#define BEZIER_STEPS 10

void drawSVG(window *win, int16_t x, int16_t y, int16_t c, float scale, uint16_t color, int16_t fill) 
{

	p2_int16_t Ctl,T;
	p2_int16_t Cur;
	p2_int16_t Move;
	int16_t Xsize,Ysize;

	int state;
	p2_int16_t p[3];
	p2_int16_t B[BEZIER_STEPS];

	int ind;
	int t;
	int i,j,count;

	int16_t *v;

	if( c >= 32 && c <= 127)
	{
		c -= 32;
		v = vfont[c]->v;
		Xsize = (scale * vfont[c]->xinc);
		Ysize = (scale * vfont[c]->yinc);
		// CLear
        tft_fillRectWH(win, x, y, Xsize, Ysize, win->bg);

	}
	else
	{
		printf("DrawSVG: invalid character\n");
		return;
	}
	

	printf("DrawSVG:(%d,%d) scale:%e)\n", x,y,(double)scale);

	ind = 0;
	state = 0;
	while(1)
	{
		char t = v[ind];
		if (t == 'M') 
		{
			// Move takes to move points
			Move.X = x + (scale * v[ind + 1]);
			Move.Y = y + (scale * v[ind + 2]);
			//printf("M: Move.X:%d,Move.Y:%d\n", (int)Move.X,(int)Move.Y);
			Cur.X = Move.X;
			Cur.Y = Move.Y;
			if(state < 3)
			{
				p[state].X = Cur.X;
				p[state].Y = Cur.Y;
				++state;
			}
			ind += 3;
		}
		else if (t == 'L' ) 
		{
			// Line takes Cur.X,Cur.Y and to target points
			T.X = x + (scale * v[ind + 1]);
			T.Y = y + (scale * v[ind + 2]);
			//printf("L: Curx:%d,Cury:%d,T.X:%d,T.Y:%d\n", (int)Cur.X,(int)Cur.Y,(int)T.X,(int)T.Y);
			if(T.X != Cur.X || T.Y != Cur.Y)
			{
				tft_drawLine(win, Cur.X, Cur.Y, T.X, T.Y, color);
				if(state < 3)
				{
					p[state].X = T.X;
					p[state].Y = T.Y;
					++state;
				}
			}
			Cur.X = T.X;
			Cur.Y = T.Y;
			ind += 3;

		}
		else if (t == 'Q' ) 
		{
			// Q Bezier takes Cur.X,Cur.Y, two control points and to target points
			Ctl.X =  x + (scale * v[ind + 1]);
			Ctl.Y =  y + (scale * v[ind + 2]);
			T.X =  x + (scale * v[ind + 3]);
			T.Y =  y + (scale * v[ind + 4]);
			//printf("Q: Cur.X:%d,Cury:%d,Ctl.X:%d,Ctl.Y:%d,T.X:%d,T.Y:%d\n", (int)Cur.X,(int)Cur.Y,(int)Ctl.X,(int)Ctl.Y,(int)T.X,(int)T.Y);
			if(T.X != Cur.X || T.Y != Cur.Y)
			{
#if 1
				count = tft_Bezier2(win, Cur, Ctl, T, BEZIER_STEPS, color);
#else
				if(T.X != Cur.X || T.Y != Cur.Y)
					tft_drawLine(win, Cur.X, Cur.Y, T.X, T.Y, color);
#endif
				if(state < 3)
				{
					p[state].X = T.X;
					p[state].Y = T.Y;
					++state;
				}
			}
			Cur.X = T.X;
			Cur.Y = T.Y;
			ind += 5;

		}
		else if (t == 'Z') 
		{
			// CLOSE takes no params
			//printf("Z: Curx:%d,Cury:%d,Move.X:%d,Move.Y:%d\n", (int)Cur.X,(int)Cur.Y,(int)Move.X,(int)Move.Y);
			if(Cur.X != Move.X || Cur.Y != Move.Y)
			{
				tft_drawLine(win, Cur.X, Cur.Y, Move.X, Move.Y, color);
				if(state < 3)
				{
					p[state].X = T.X;
					p[state].Y = T.Y;
					++state;
				}
			}
			Cur.X = Move.X;
			Cur.Y = Move.Y;
			ind += 1;
		}
		else if (t == '.') 
		{
			break;
		}
		else
		{
			printf("bad type:%c @ index:%d\n", (int)t, (int)ind);
			break;
		}
	}

#if 1

	if(state == 3)
	{
		int i;
		printf("Fill\n");
		for(i=0;i<3;++i)
		{
			printf("%d,%d\n", p[i].X, p[i].Y);
		}

	}
		
#else
	if(fill)
	{
		for(i=0;i<Ysize;++i)
		{
			count = tft_FillPolyLine( win , x, y + i, Xsize, color );
		   // optimistic_yield(1000);

		}
	}
#endif
}

#endif //ifdef VFONTS
