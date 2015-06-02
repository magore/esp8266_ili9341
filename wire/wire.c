/**
 @file wireframe.c

 @par wireframe viewer
 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief Display wireframe object 
 The code handles fixed, proportional and bounding box format fonts
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option)
 any later version.

 wireframe.c is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "user_config.h"

#include "ili9341.h"
#include <math.h>
#include "cordic.h"
#include "wire.h"


void wire2fp(wire_p *in, point *out)
{
	// Point
	out->x = WIRE_2FP(in->x);
	out->y = WIRE_2FP(in->y);
	out->z = WIRE_2FP(in->z);
}

void scale2display(point *p, int *x, int *y)
{
	*x = (p->x *25) + MAX_TFT_X/2;
	*y = (p->y *25) + MAX_TFT_Y/2;
}

// Draw a wireframe
void wire_draw(wire_p *wire, point *view, double scale, uint16_t color)
{
	int i;
	int last = WIRE_SEP;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	wire_p W;
	point P,R;

	W.x = 0;
	W.y = 0;
	W.z = 0;

	for (i = 0; ; i++)
	{
		// W = wire[i];
		cpy_flash((uint8_t *) &wire[i], (uint8_t *) &W, sizeof(W));

		if(W.x == WIRE_END)
		{
			break;
		}
		if(W.x == WIRE_SEP)
		{
			last = WIRE_SEP;
			continue;
		}

		// Get next point
		wire2fp(&W, &P);

		// CORDIC Rotate
		rotate(&P,view);

		// CORDIC Scale
		scale_point(&P, scale);

		// CORDIC Project
		PerspectiveProjection(&P);

		if(last == WIRE_SEP)	
		{
			// first point in list ??
			scale2display(&P, &x0,&y0);
			last = 0;
			continue;
		}

		scale2display(&P, &x1,&y1);
		last = 0;

		// tft_printf(0,300,1,"i:%d", i);
//ets_uart_printf("i:%d,x:%d,y:%d-x1:%d,y1:%d,color:%04x", i,(int)x0, (int)y0, (int)x1, (int)y1, (int)color);

		// Draw line
		tft_drawLine(x0, y0, x1, y1, color);
		// First is Next
		x0 = x1;
		y0 = y1;

		ets_wdt_disable();
	}
//ets_uart_printf("i:%d,done\n",(int) i);
}
