CFLAGS=-D_GNU_SOURCE

# Various fixed mono size font sets
FONTS_C = fixed/7x13.bdf \
	fixed/7x13B.bdf \
	fonts/C/24-Terminus.bdf \
	fonts/C/24-Terminus-Bold.bdf \
	fonts/C/32-Terminus.bdf \
	fonts/C/32-Terminus-Bold.bdf 

# Various fixed bold mono size font sets
FONTS_B = fixed/7x13B.bdf \
	fonts/C/24-Terminus-Bold.bdf \
	fonts/C/32-Terminus-Bold.bdf 

# Various fixed bold mono size font sets - with extra large Freefont set
FONTS_BE = $(FONTS_B) freefont/FreeSansBold.bdf

# Various proportional size font sets
FONTS_P = fonts/P/12-Adobe-Helvetica.bdf \
	fonts/P/12-Adobe-Helvetica-Bold.bdf \
	fonts/P/21-Adobe-Helvetica.bdf \
	fonts/P/21-Adobe-Helvetica-Bold.bdf \
	fonts/P/35-Adobe-Helvetica.bdf \
	fonts/P/35-Adobe-Helvetica-Bold.bdf

#all:	bdffont2c pfont copyfonts cleanfont
all:	bdffont2c pfont_sbb copyfonts cleanfont

install:	all 
	cp -p bdffont2c /usr/local/bin

.PHONY:	copyfonts
copyfonts: 
	cp -p font.h fonts.h ../display

bdffont2c.o:	bdffont2c.c
	$(CC) $(CFLAGS) -g -c bdffont2c.c 

bdffontutil.o:	bdffontutil.c
	$(CC) $(CFLAGS) -g -c bdffontutil.c 

bdffont2c: bdffont2c.o bdffontutil.o
	$(CC) $(CFLAGS) -g -o bdffont2c bdffont2c.o bdffontutil.o

# Give help page
help:	install
	./bdffont2c

# ============================================================
# bdffont2c: [-o font_output_file ] [ -l lower_bound ] [ -u upper_bound ] [ -preview ] [ -Preview ] bdf files...
#  -o: output file containg C STructures of converted font data
#  -u: limit Glyph processing above limit
#  -l: limit Glyph processing below limit
#  -p 1 Preview font bitmap data - only parts with 1 bits set
#  -p 2 Preview Full font bitmap data - everything inside font bounding box
#  -p 3 Preview Proportional font bitmap data and fixed - everything inside font bounding box
#  -f convert any font to fixed size bitmap - everything inside font bounding box
#  -s Compact font to smallest bounding box
#     Notes: modifies or creates font specs
#  bdf files...: One of more BDF format font files
# ============================================================

# Always compile
.PHONY:	bdfview
bdfview:
	$(CC) $(CFLAGS) -g -o bdfview bdfview.c bdffontutil.o
# ============================================================

# Examples
# Fixed font set 
fixed:	cleanfont bdffont2c 
	./bdffont2c -p 2 -l 32 -u 127 -o fonts.h $(FONTS_C)

# Fixed font set force to fixed mode
ffont:	cleanfont bdffont2c 
	./bdffont2c -f -p 2 -l 32 -u 127 -o fonts.h $(FONTS_C)

# Smallest bounding box
ffont_sbb:	cleanfont bdffont2c 
	./bdffont2c -s -p 2 -l 32 -u 127 -o fonts.h $(FONTS_B)

# Proportional font set
pfont:	cleanfont bdffont2c 
	./bdffont2c -p 3 -l 32 -u 127 -o fonts.h $(FONTS_P)

# Smallest bounding box, convert to proportional
pfont_sbb:	cleanfont bdffont2c
	./bdffont2c -s -p 3 -l 32 -u 127 -o fonts.h $(FONTS_BE)

# ============================================================
# Various viewer tests with different font groups

# view font index 5
# Preview Full font bitmap data - everything inside font bounding box
test_ffont:    ffont bdfview 
	./bdfview -n 5 -p 2 

# view font index 2
# Preview Proportional font bitmap data and fixed - everything inside font bounding box
test_ffont_sbb:    ffont_sbb bdfview 
	./bdfview -n 2 -p 3 

# view font index 5
# Preview Proportional font bitmap data and fixed - everything inside font bounding box
test_pfont:	pfont bdfview 
	./bdfview -n 5 -p 3 

# view font index 3
# Preview Proportional font bitmap data and fixed - everything inside font bounding box
test_pfont_sbb:	pfont_sbb bdfview 
	./bdfview -n 3 -p 3 
# ============================================================

cleanfont:
	rm -f fonts.h 

clean:
	-rm -f bdffont2c *\.o fonts.h bdfview *\.bdf
