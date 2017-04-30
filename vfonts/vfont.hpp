// font_to_svg.hpp - Read Font in TrueType (R) format, write SVG
// Copyright Don Bright 2013 <hugh.m.bright@gmail.com>
/*

  This software is provided 'as-is', without any express or implied
  warranty.  In no event will the authors be held liable for any damages
  arising from the use of this software.

  Permission is granted to anyone to use this software for any purpose,
  including commercial applications, and to alter it and redistribute it
  freely, subject to the following restrictions:

  1. The origin of this software must not be misrepresented; you must not
     claim that you wrote the original software. If you use this software
     in a product, an acknowledgment in the product documentation would be
     appreciated but is not required.
  2. Altered source versions must be plainly marked as such, and must not be
     misrepresented as being the original software.
  3. This notice may not be removed or altered from any source distribution.

  License based on zlib license, by Jean-loup Gailly and Mark Adler
*/

/*

This program reads a TTF (TrueType (R)) file and outputs an SVG path.

See these sites for more info.
Basic Terms: http://www.freetype.org/freetype2/docs/glyphs/glyphs-3.html
FType + outlines: http://www.freetype.org/freetype2/docs/reference/ft2-outline_processinhtml
FType + contours: http://www.freetype.org/freetype2/docs/glyphs/glyphs-6.html
TType contours: https://developer.apple.com/fonts/TTRefMan/RM01/Chap1.html
TType contours2: http://www.truetype-typography.com/ttoutln.htm
Non-zero winding rule: http://en.wikipedia.org/wiki/Nonzero-rule
SVG paths: http://www.w3schools.com/svg/svg_path.asp
SVG paths + nonzero: http://www.w3.org/TR/SVG/paintinhtml#FillProperties

TrueType is a trademark of Apple Inc. Use of this mark does not imply
endorsement.

*/

#ifndef _VFONT_H_
#define _VFONT_H_

#include <ft2build.h>
#include FT_FREETYPE_H
#include <iostream>
#include <sstream>
#include <vector>
#include <string>

namespace font2svg {

std::stringstream debug;

class ttf_file
{
public:
	std::string filename;
	FT_Library library;
	FT_Face face;
	FT_Error error;

	ttf_file()
	{
		filename = std::string("");
	}

	ttf_file( std::string fname )
	{
		filename = fname;
		error = FT_Init_FreeType( &library );
		if(error)
		{
			printf("Init error code: %d\n", error);
			exit(1);
		}

		// Load a typeface
		error = FT_New_Face( library, filename.c_str(), 0, &face );
		if(error)
		{
			printf("Face load error code: %d\n", error);
			exit(1);
		}

		if (error) {
			printf("problem loading file %s\n", (char *) filename.c_str() );
			exit(1);
		}
#if 0
		printf("//File name: %s\n", (char *) filename.c_str() );
		printf("//Number of faces: %d\n", (int)face->num_faces);
		printf("//Number of glyphs: %d\n", (int)face->num_glyphs);
#endif
	}

	void free()
	{
		debug << "\n<!--";
		error = FT_Done_Face( face );
		debug << "\nFree face. error code: " << error;
		error = FT_Done_FreeType( library );
		debug << "\nFree library. error code: " << error;
		debug << "\n-->\n";
	}

};


/* Draw the outline of the font as svg.
There are three main components.
1. the points
2. the 'tags' for the points
3. the contour indexes (that define which points belong to which contour)
*/
std::string do_outline(std::vector<FT_Vector> points, std::vector<char> tags, std::vector<short> contours)
{
	std::stringstream svg;

	svg.str("");

	svg << "\t{\n";
	/* tag bit 1 indicates whether its a control point on a bez curve
	or not. two consecutive control points imply another point halfway
	between them */

	// Step 1. move to starting point (M x-coord y-coord )
	// Step 2. decide whether to draw a line or a bezier curve or to move
	// Step 3. for bezier: Q control-point-x control-point-y,
	//		         destination-x, destination-y
	//         for line:   L x-coord, y-coord
	//         for move:   M x-coord, y-coord

	int contour_starti = 0;
	int contour_endi = 0;

	for ( int i = 0 ; i < contours.size() ; i++ ) {
		contour_endi = contours.at(i);
		int offset = contour_starti;
		int npts = contour_endi - contour_starti + 1;

		// Start point index: contour_starti
		// End point index: contour_endi
		// Number of points in this contour: npts 
		// First point points[offset].x, points[offset].y 

		svg << "\t\t'M', " << points[contour_starti].x << "," << points[contour_starti].y << ",\n";

		// points for this contour
		for ( int j = 0; j < npts; j++ ) {
			int thisi = j%npts + offset;
			int nexti = (j+1)%npts + offset;
			int nextnexti = (j+2)%npts + offset;
			int x = points[thisi].x;
			int y = points[thisi].y;
			int nx = points[nexti].x;
			int ny = points[nexti].y;
			int nnx = points[nextnexti].x;
			int nny = points[nextnexti].y;
			bool this_tagbit1 = (tags[ thisi ] & 1);
			bool next_tagbit1 = (tags[ nexti ] & 1);
			bool nextnext_tagbit1 = (tags[ nextnexti ] & 1);
			bool this_isctl = !this_tagbit1;
			bool next_isctl = !next_tagbit1;
			bool nextnext_isctl = !nextnext_tagbit1;

			// thisi is this point index
			// nexti is thisi+1
			// nextnexti is thisi+2
			// this_tagbit1 is this tagbit
			// next_tagbit1 is tagbit+1
			// nextnext_tagbit1 is tagbit+2

			if (this_isctl && next_isctl) {
				// two adjacent ctl pts. adding point halfway between 
				x = (x + nx) / 2;
				y = (y + ny) / 2;
				this_isctl = false;

				if (j==0) {
					// first pt in contour was ctrl pt. moving to non-ctrl pt
					svg << "\t\t'M', " << x << "," << y << ",\n";
				}
			}

			if (!this_isctl && next_isctl && !nextnext_isctl) {
				// bezier 
				svg << "\t\t'Q', " << nx << "," << ny << "," << nnx << "," << nny << ",\n";
			} else if (!this_isctl && next_isctl && nextnext_isctl) {
				// two ctl pts coming. split bezier by adding point halfway between 
				nnx = (nx + nnx) / 2;
				nny = (ny + nny) / 2;
				svg << "\t\t'Q', " << nx << "," << ny << "," << nnx << "," << nny << ",\n";
			} else if (!this_isctl && !next_isctl) {
				// Line
				svg << "\t\t'L', " << nx << "," << ny << ",\n";
			} else if (this_isctl && !next_isctl) {

				// this is ctrl pt. skipping to nx,ny
			}
		}
		contour_starti = contour_endi+1;
		svg << "\t\t'Z',\n";
	}
	svg << "\t\t'.'\n";
	svg << "\t}\n";
	svg << "};\n";
	return svg.str();
}

class glyph
{
public:
	int codepoint;
	FT_GlyphSlot slot;
	FT_Error error;
	FT_Outline ftoutline;
	FT_Glyph_Metrics gm;
	FT_Face face;
	ttf_file file;

	FT_Vector* ftpoints;
	char* tags;
	short* contours;

	std::stringstream debug, tmp;
	int bbwidth, bbheight;

	glyph( ttf_file &f, std::string unicode_str )
	{
		file = f;
		init( unicode_str );
	}

	glyph( const char * filename, std::string unicode_str )
	{
		this->file = ttf_file( std::string(filename) );
		init( unicode_str );
	}

	glyph( const char * filename, const char * unicode_c_str )
	{
		this->file = ttf_file( std::string(filename) );
		init( std::string(unicode_c_str) );
	}

	void free()
	{
		file.free();
	}


	char *facename(char *str, int len)
	{
        snprintf(str,len, "%s%s", (char *) face->family_name, face->style_name);
		return(str);
	}

	void init( std::string unicode_s )
	{
		face = file.face;
		codepoint = strtol( unicode_s.c_str() , NULL, 0 );
		// Load the Glyph into the face's Glyph Slot + print details
		FT_UInt glyph_index = FT_Get_Char_Index( face, codepoint );
		debug << "<!--\nUnicode requested: " << unicode_s;
		debug << " (decimal: " << codepoint << " hex: 0x"
			<< std::hex << codepoint << std::dec << ")";
		debug << "\nGlyph index for unicode: " << glyph_index;
		error = FT_Load_Glyph( face, glyph_index, FT_LOAD_NO_SCALE );
		debug << "\nLoad Glyph into Face's glyph slot. error code: " << error;
		slot = face->glyph;
		ftoutline = slot->outline;
		char glyph_name[1024];
		FT_Get_Glyph_Name( face, glyph_index, glyph_name, 1024 );
		debug << "\nGlyph Name: " << glyph_name;
		debug << "\nGlyph Width: " << gm.width
			<< " Height: " << gm.height
			<< " Hor. Advance: " << gm.horiAdvance
			<< " Vert. Advance: " << gm.vertAdvance;
		gm = slot->metrics;

		// Print outline details, taken from the glyph in the slot.
		debug << "\nNum points: " << ftoutline.n_points;
		debug << "\nNum contours: " << ftoutline.n_contours;
		debug << "\nContour endpoint index values:";
		for ( int i = 0 ; i < ftoutline.n_contours ; i++ ) debug << " " << ftoutline.contours[i];
		debug << "\n-->\n";

		// Invert y coordinates (SVG = neg at top, TType = neg at bottom)

		ftpoints = ftoutline.points;

// INVERT TTF to SVG Y axis

		for ( int i = 0 ; i < ftoutline.n_points ; i++ )
		{
			ftpoints[i].y = gm.height - ftpoints[i].y ;
		}

		bbheight = face->bbox.yMax - face->bbox.yMin;
		bbwidth = face->bbox.xMax - face->bbox.xMin;
		tags = ftoutline.tags;
		contours = ftoutline.contours;

		char tmp[256+2];
		// MG
		///@brief Dump font glyph header for this character
        printf("// Char: %02x\n", (int) codepoint);
        printf("// Name: %s\n", glyph_name);
		printf("// points: %d\n", (int) ftoutline.n_points);
		printf("// bbheight: %d\n", (int) bbheight);
		printf("// bbwidth: %d\n", (int) bbwidth);
        printf("path_t %s_glyph_%02x = {\n", (char *) facename(tmp,256), (int)codepoint);
		printf("\t/* X offset        */ %d,\n", 0 );
		printf("\t/* Y offset        */ %d,\n", 0);
		printf("\t/* Width           */ %d,\n", (int) gm.width);
		printf("\t/* Height          */ %d,\n", (int) gm.height);
		printf("\t/* X INC           */ %d,\n", (int) gm.horiAdvance);
		printf("\t/* Y INC           */ %d,\n", (int) gm.vertAdvance);
		printf("\t/* gm.horiBearingY */ %d,\n", (int) gm.horiBearingY);
		printf("\t/* gm.vertBearingY */ %d,\n", (int) gm.vertBearingY);
		printf("\t/* Contours */        %d,\n", (int) ftoutline.n_contours);
		printf("\t/* Outline */\n");
	}


	std::string outline()  {
		std::vector<FT_Vector> pointsv(ftpoints,ftpoints+ftoutline.n_points);
		std::vector<char> tagsv(tags,tags+ftoutline.n_points);
		std::vector<short> contoursv(contours,contours+ftoutline.n_contours);
		return do_outline(pointsv, tagsv, contoursv);
	}

};

} // namespace

#endif

