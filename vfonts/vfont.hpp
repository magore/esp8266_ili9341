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
		debug << "Init error code: " << error;

		// Load a typeface
		error = FT_New_Face( library, filename.c_str(), 0, &face );
		debug << "\nFace load error code: " << error;
		debug << "\nfont filename: " << filename;
		if (error) {
			std::cerr << "problem loading file " << filename << "\n";
			exit(1);
		}
		debug << "\nFamily Name: " << face->family_name;
		debug << "\nStyle Name: " << face->style_name;
		debug << "\nNumber of faces: " << face->num_faces;
		debug << "\nNumber of glyphs: " << face->num_glyphs;
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
	std::stringstream debug, svg;
	if (points.size()==0) return "";
	if (contours.size()==0) return "";
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
		debug << "new contour starting. startpt index, endpt index:";
		debug << contour_starti << "," << contour_endi << "\n";
		int offset = contour_starti;
		int npts = contour_endi - contour_starti + 1;
		debug << "number of points in this contour: " << npts << "\n";
		debug << "moving to first pt " << points[offset].x << "," << points[offset].y << "\n";
		svg << "\t\t'M', " << points[contour_starti].x << "," << points[contour_starti].y << ",\n";
		debug << "listing pts: [this pt index][isctrl] <next pt index><isctrl> [x,y] <nx,ny>\n";
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
			debug << " [" << thisi << "]";
			debug << "[" << !this_tagbit1 << "]";
			debug << " <" << nexti << ">";
			debug << "<" << !next_tagbit1 << ">";
			debug << " <<" << nextnexti << ">>";
			debug << "<<" << !nextnext_tagbit1 << ">>";
			debug << " [" << x << "," << y << "]";
			debug << " <" << nx << "," << ny << ">";
			debug << " <<" << nnx << "," << nny << ">>";
			debug << "\n";

			if (this_isctl && next_isctl) {
				debug << " two adjacent ctl pts. adding point halfway between " << thisi << " and " << nexti << ":";
				debug << " reseting x and y to ";
				x = (x + nx) / 2;
				y = (y + ny) / 2;
				this_isctl = false;
				debug << " [" << x << "," << y <<"]\n";
				if (j==0) {
					debug << "first pt in contour was ctrl pt. moving to non-ctrl pt\n";
					svg << "\t\t'M', " << x << "," << y << ",\n";
				}
			}

			if (!this_isctl && next_isctl && !nextnext_isctl) {
				svg << "\t\t'Q', " << nx << "," << ny << "," << nnx << "," << nny << ",\n";
				debug << " bezier to " << nnx << "," << nny << " ctlx, ctly: " << nx << "," << ny << "\n";
			} else if (!this_isctl && next_isctl && nextnext_isctl) {
				debug << " two ctl pts coming. adding point halfway between " << nexti << " and " << nextnexti << ":";
				debug << " reseting nnx and nny to halfway pt";
				nnx = (nx + nnx) / 2;
				nny = (ny + nny) / 2;
				svg << "\t\t'Q', " << nx << "," << ny << "," << nnx << "," << nny << ",\n";
				debug << " bezier to " << nnx << "," << nny << " ctlx, ctly: " << nx << "," << ny << "\n";
			} else if (!this_isctl && !next_isctl) {
				svg << "\t\t'L', " << nx << "," << ny << ",\n";
				debug << " line to " << nx << "," << ny << "\n";			
			} else if (this_isctl && !next_isctl) {
				debug << " this is ctrl pt. skipping to " << nx << "," << ny << "\n";
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

#if 1
		for ( int i = 0 ; i < ftoutline.n_points ; i++ )
			ftpoints[i].y *= -1;
#endif
// MG
///@brief Adjust Y for this character
#if 1
		for ( int i = 0 ; i < ftoutline.n_points ; i++ )
		{
			ftpoints[i].y += (gm.horiBearingY);

		}
#endif
		bbheight = face->bbox.yMax - face->bbox.yMin;
		bbwidth = face->bbox.xMax - face->bbox.xMin;
		tags = ftoutline.tags;
		contours = ftoutline.contours;
		// std::cout << debug.str();

		// MG
		///@brief Dump font glyph header for this character
		tmp.str("");
        tmp << "// Char: " << codepoint << "\n";
		tmp << "// points: " << ftoutline.n_points << "\n";
		tmp << "// bbheight: " << bbheight << "\n";
		tmp << "// bbwidth: " << bbwidth << "\n";
        tmp << "path_t _vec" << codepoint << " = {\n";
		tmp << "\t/* X offset        */ " << 0 << ",\n";
		tmp << "\t/* Y offset        */ " << 0 << ",\n";
		tmp << "\t/* Width           */ " << gm.width << ",\n";
		tmp << "\t/* Height          */ " << gm.height << ",\n";
		tmp << "\t/* X INC           */ " << gm.horiAdvance << ",\n";
		tmp << "\t/* Y INC           */ " << gm.vertAdvance << ",\n";
		tmp << "\t/* gm.horiBearingY */ " << gm.horiBearingY << ",\n";
		tmp << "\t/* gm.vertBearingY */ " << gm.vertBearingY << ",\n";
		tmp << "\t /* Outline */\n";
		std::cout << tmp.str();
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

