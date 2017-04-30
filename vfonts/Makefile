#WARN = -pedantic -Wall
#FREETYPE_FLAGS=$`freetype-config --cflags --libs`

all:	vfont make_vfonts

.PHONY:	vfont
vfont: vfont.cpp
	@echo "g++ $(WARN) vfont.cpp -o vfont `freetype-config --cflags --libs`"
	g++ $(WARN) vfont.cpp -o vfont `freetype-config --cflags --libs`

.PHONY:	make-vfonts
make_vfonts:	vfont
	./vfont ../fonts/freefont/FreeSans.ttf >../display/vfonts.h

clean:
	rm -f vfont
