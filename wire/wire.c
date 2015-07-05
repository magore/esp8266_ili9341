/**
 @file wire.c

 @brief wireframe view code

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

#include <user_config.h>

/*
 @brief convert fixed point coordinate to floating point
 @param [in] *in: fixed point coordinate
 @param [out] *out: floating point coordinate
 @return void
*/
void wire2fp(wire_p *in, point *out)
{
	// Point
	out->x = WIRE_2FP(in->x);
	out->y = WIRE_2FP(in->y);
	out->z = WIRE_2FP(in->z);
}

/*
 @brief Draw a wireframe
 @param [in] *wire: fixed point  points
 @param [in] *edge: fixed point  edges - optional or NULL
 @param [in] *view: view point
 @param [in] x: X offset
 @param [in] y: Y offsetfactor
 @param [in] scale: scale factor
 @param [in] color: color
 @return void
*/
void wire_draw(window *win, const wire_p *wire, const wire_e *edge, point *view, int x, int y, double scale, uint16_t color)
{
	int i;
	int last = WIRE_SEP;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	wire_p W;
	wire_e E;
	point P,R;

	W.x = 0;
	W.y = 0;
	W.z = 0;

	if(edge != NULL )
	{
		for (i = 0;; i++)
		{
			// E = edge[i];
			cpy_flash((uint8_t *) &edge[i], (uint8_t *) &E, sizeof(E));
			if(E.p1 == -1)
				break;

			// P1 
			cpy_flash((uint8_t *) &wire[E.p1], (uint8_t *) &W, sizeof(W));
			wire2fp(&W, &P);
			// CORDIC Rotate
			rotate(&P,view);
			// CORDIC Perspective Projection Offset and Scale
			PerspectiveProjection(&P, scale, x,y);
			x0 = P.x;
			y0 = P.y;

			// P2 
			cpy_flash((uint8_t *) &wire[E.p2], (uint8_t *) &W, sizeof(W));
			wire2fp(&W, &P);
			// CORDIC Rotate
			rotate(&P,view);
			// CORDIC Perspective Projection Offset and Scale
			PerspectiveProjection(&P, scale, x,y);
			x1 = P.x;
			y1 = P.y;

			// Draw line
			tft_drawLine(win, x0, y0, x1, y1, color);
//DEBUG_PRINTF("i:%d (x:%d,y:%d),(x1:%d,y1:%d),color:%04x\n", i,(int)x0, (int)y0, (int)x1, (int)y1, (int)color);

			ets_wdt_disable();
		}
		return;
	}

	/* edge == NULL */
	/* We have a list of connected points and no edge data */
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

		// CORDIC Perspective Projection Offset and Scale
		PerspectiveProjection(&P, scale, x,y);

		if(last == WIRE_SEP)	
		{
			// first point in list ??
			x0 = P.x;
			y0 = P.y;
			last = 0;
			continue;
		}
		x1 = P.x;
		y1 = P.y;
		last = 0;

		// tft_printf(0,300,1,"i:%d", i);
//DEBUG_PRINTF("i:%d,x:%d,y:%d-x1:%d,y1:%d,color:%04x", i,(int)x0, (int)y0, (int)x1, (int)y1, (int)color);

		// Draw line
		tft_drawLine(win, x0, y0, x1, y1, color);
		// First is Next
		x0 = x1;
		y0 = y1;

		ets_wdt_disable();
	}
//DEBUG_PRINTF("i:%d,done\n",(int) i);
}
