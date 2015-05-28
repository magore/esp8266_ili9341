/**
 @file earth.c

 @par earth  wireframe viewer
 @par Copyright &copy; 2015 Mike Gore, GPL License

 @brief DIsplay earth costline wireframe
 The code handles fixed, proportional and bounding box format fonts
 @see http://en.wikipedia.org/wiki/Glyph_Bitmap_Distribution_Format
 @par Edit History
 - [1.0]   [Mike Gore]  Initial revision of file.

 This is free software: you can redistribute it and/or modify it under the
 terms of the GNU General Public License as published by the Free Software
 Foundation, either version 3 of the License, or (at your option)
 any later version.

 earth.c is distributed in the hope that it will be useful,
 but WITHOUT ANY WARRANTY; without even the implied warranty of
 MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 GNU General Public License for more details.

 You should have received a copy of the GNU General Public License
 along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#define MAP_DATA
#include "user_config.h"

#include "ili9341.h"
#include <math.h>
#include "cordic.h"
#include "earth.h"

void map2fp(mapxyz *in, point *out)
{
	// Point
	out->x = MAP2FP(in->x);
	out->y = MAP2FP(in->y);
	out->z = MAP2FP(in->z);
}

void Point2Display(point *p, int *x, int *y)
{
	*x = (p->x *100) + MAX_TFT_X/2;
	*y = (p->y *100) + MAX_TFT_Y/2;
}

// Draw a wireframe
void earth_draw(point *view, double scale, uint16_t color)
{
	int i;
	int last = MAPSEP;
	int x0 = 0;
	int y0 = 0;
	int x1 = 0;
	int y1 = 0;
	mapxyz M;
	point P,R;

	M.x = 0;
	M.y = 0;
	M.z = 0;

	for (i = 0; ; i++)
	{
		// M = map[i];
		cpy_flash((uint8_t *) &(map[i]), (uint8_t *) &M, sizeof(M));

		if(M.x == MAPEND)
		{
			break;
		}
		if(M.x == MAPSEP)
		{
			last = MAPSEP;
			continue;
		}

		// Grab first point
		if(last == MAPSEP)	
		{
			map2fp(&M, &P);
			rotate(&P,view);
			scale_point(&P, scale);
			PerspectiveProjection(&P);
			Point2Display(&P, &x0,&y0);
			last = 0;
			continue;
		}

		// Get next point
		map2fp(&M, &P);
		rotate(&P,view);
		scale_point(&P, scale);
		PerspectiveProjection(&P);
		Point2Display(&P, &x1,&y1);
		last = 0;

		// tft_printf(0,300,1,"i:%d", i);
		//tft_printf(0,300,1,"i:%d,x:%d,y:%d-x1:%d,y1:%d", i,(int)x0, (int)y0, (int)x1, (int)y1);

		// Draw line
		tft_drawLine(x0, y0, x1, y1, color);
		// First is Next
		x0 = x1;
		y0 = y1;
 ets_wdt_disable();
	}
	// tft_printf(0,300,1,"DONE");
}
